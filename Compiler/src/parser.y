%{
    #include "ast.hpp"
    #include <cstdio>
    #include <fstream>
    #include "symbol_table.hpp"
    #include "code_generator.hpp"

    extern FILE* yyin;

    ProgramNode* root;
    void yyerror(const char *s);
    int yylex(void);
    extern int yylineno;
%}

%debug

%code requires {
    #include <iostream>
    #include <string>
    #include "ast.hpp"
    
}

%union {
    long long num;
    std::string* str;
    ProgramNode* program_node;
    ProceduresNode* procedures_node;
    MainNode* main_node;
    CommandsNode* commands_node;
    CommandNode* command_node;
    ProcedureHeadNode* proc_head_node;
    ProcedureCallNode* proc_call_node;
    DeclarationsNode* declarations_node;
    ArgumentsDeclarationNode* args_decl_node;
    ProcedureCallArguments* args_node;
    ExpressionNode* expression_node;
    ConditionNode* condition_node;
    ValueNode* value_node;
    IdentifierNode* identifier_node;
}

%token <num> NUM
%token FROM TO DOWNTO
%token READ WRITE T ASSIGN
%token EQ NEQ LT GT LEQ GEQ 
%token <str> PIDENTIFIER_TOKEN
%token PLUS MINUS ASTERISK FWSLASH PERCENT
%token WHILE DO ENDWHILE REPEAT UNTIL FOR ENDFOR
%token COMMA LPAREN RPAREN LBRACK RBRACK COLON SEMICOLON 
%token PROGRAM PROCEDURE IS IF BEGIN_T END_T THEN ELSE ENDIF 

%type <identifier_node> identifier
%type <value_node> value 
%type <expression_node> expression
%type <condition_node> condition
%type <program_node> program_all
%type <procedures_node> procedures
%type <main_node> main
%type <commands_node> commands
%type <command_node> command
%type <proc_head_node> proc_head
%type <proc_call_node> proc_call
%type <declarations_node> declarations
%type <args_decl_node> args_decl
%type <args_node> args

%start program_all

%%

program_all:
    procedures main {
        root = new ProgramNode();
        root->addProcedures($1);
        root->addMain($2);
    }
    ;

procedures:
    procedures PROCEDURE proc_head IS declarations BEGIN_T commands END_T {
        $1->addProcedure(new ProcedureNode($3, $5, $7));
        $$ = $1;
        $$->setLineNumber(yylineno);
    }
    | procedures PROCEDURE proc_head IS BEGIN_T commands END_T {
        $1->addProcedure(new ProcedureNode($3, nullptr, $6));
        $$ = $1;
        $$->setLineNumber(yylineno);
    }
    | {
        $$ = new ProceduresNode();
        $$->setLineNumber(yylineno);
    }
    ;

main:
    PROGRAM IS declarations BEGIN_T commands END_T {
        $$ = new MainNode($3, $5);
        $$->setLineNumber(yylineno);
    }
    | PROGRAM IS BEGIN_T commands END_T {
       $$ = new MainNode($4); 
       $$->setLineNumber(yylineno);
    }
    ;

commands:
    commands command {
        $1->addCommand($2);
        $$ = $1;
    }
    | command {
        $$ = new CommandsNode();
        $$->addCommand($1);
    }
    ;

command:
    identifier ASSIGN expression SEMICOLON {
         $$ = new AssignNode($1, $3);
         $$->setLineNumber(yylineno);
    }
    | IF condition THEN commands ELSE commands ENDIF {
        $$ = new IfNode($2, $4, $6);
        $$->setLineNumber(yylineno);
    }
    | IF condition THEN commands ENDIF {
        $$ = new IfNode($2, $4, nullptr);
        $$->setLineNumber(yylineno);
    }
    | WHILE condition DO commands ENDWHILE {
        $$ = new WhileNode($2, $4);
        $$->setLineNumber(yylineno);
    }
    | REPEAT commands UNTIL condition SEMICOLON {
        $$ = new RepeatUntilNode($4, $2);
        $$->setLineNumber(yylineno);
    }
    | FOR PIDENTIFIER_TOKEN FROM value TO value DO commands ENDFOR {
        $$ = new ForToNode($2, $4, $6, $8);
        $$->setLineNumber(yylineno);
    }
    | FOR PIDENTIFIER_TOKEN FROM value DOWNTO value DO commands ENDFOR {
        $$ = new ForDownToNode($2, $4, $6, $8);
        $$->setLineNumber(yylineno);
    }
    | proc_call SEMICOLON {
        $$ = $1;
        $$->setLineNumber(yylineno);
    }
    | READ identifier SEMICOLON {
        $$ = new ReadNode($2);
        $$->setLineNumber(yylineno);
    }
    | WRITE value SEMICOLON {
        $$ = new WriteNode($2);
        $$->setLineNumber(yylineno);
    }
    ;

proc_head:
    PIDENTIFIER_TOKEN LPAREN args_decl RPAREN {
        $$ = new ProcedureHeadNode($1, $3);
        $$->setLineNumber(yylineno);
    }
    ;

proc_call:
    PIDENTIFIER_TOKEN LPAREN args RPAREN {
        $$ = new ProcedureCallNode($1, $3);
        $$->setLineNumber(yylineno);
    }
    ;

declarations:
    declarations COMMA PIDENTIFIER_TOKEN {
        $1->addVariableDeclaration($3);
        $$ = $1;
        $$->setLineNumber(yylineno - 1);
    }
    | declarations COMMA PIDENTIFIER_TOKEN LBRACK value COLON value RBRACK {
        $1->addArrayDeclaration($3, $5, $7);
        $$ = $1;
        $$->setLineNumber(yylineno - 1);    
    }
    | PIDENTIFIER_TOKEN {
        $$ = new DeclarationsNode();
        $$->addVariableDeclaration($1);
        $$->setLineNumber(yylineno - 1);
    }
    | PIDENTIFIER_TOKEN LBRACK value COLON value RBRACK {
        $$ = new DeclarationsNode();
        $$->addArrayDeclaration($1, $3, $5);
        $$->setLineNumber(yylineno - 1);
    }
    ;

args_decl:
    args_decl COMMA PIDENTIFIER_TOKEN {
        $1->addVariableArgument($3);
        $$ = $1;
        $$->setLineNumber(yylineno);
    }
    | args_decl COMMA T PIDENTIFIER_TOKEN {
        $1->addArrayArgument($4);
        $$ = $1;
        $$->setLineNumber(yylineno);
    }
    | PIDENTIFIER_TOKEN {
        $$ = new ArgumentsDeclarationNode();
        $$->addVariableArgument($1);
        $$->setLineNumber(yylineno);
    }
    | T PIDENTIFIER_TOKEN {
        $$ = new ArgumentsDeclarationNode();
        $$->addArrayArgument($2);
        $$->setLineNumber(yylineno);
    }
    ;

args:
    args COMMA PIDENTIFIER_TOKEN {
        $1->addArgument($3);
        $$ = $1;
        $$->setLineNumber(yylineno);
    }
    | PIDENTIFIER_TOKEN {
        $$ = new ProcedureCallArguments();
        $$->addArgument($1);
        $$->setLineNumber(yylineno);
    }
    ;

expression:
    value {
        $$ = $1;
        $$->setLineNumber(yylineno);
    }
    | value PLUS value {
        $$ = new BinaryExpressionNode($1, "+", $3);
        $$->setLineNumber(yylineno);
    }
    | value MINUS value {
         $$ = new BinaryExpressionNode($1, "-", $3);
         $$->setLineNumber(yylineno);
    }
    | value ASTERISK value {
         $$ = new BinaryExpressionNode($1, "*", $3);
         $$->setLineNumber(yylineno);
    }
    | value FWSLASH value {
         $$ = new BinaryExpressionNode($1, "/", $3);
         $$->setLineNumber(yylineno);
    }
    | value PERCENT value {
         $$ = new BinaryExpressionNode($1, "%", $3);
         $$->setLineNumber(yylineno);
    }
    ;

condition:
    value EQ value {
        $$ = new ConditionNode($1, "=", $3);
        $$->setLineNumber(yylineno);
    }
    | value NEQ value {
        $$ = new ConditionNode($1, "!=", $3);
        $$->setLineNumber(yylineno);
    }
    | value GT value {
        $$ = new ConditionNode($1, ">", $3);
        $$->setLineNumber(yylineno);
    }
    | value LT value {
        $$ = new ConditionNode($1, "<", $3);
        $$->setLineNumber(yylineno);
    }
    | value GEQ value {
        $$ = new ConditionNode($1, ">=", $3);
        $$->setLineNumber(yylineno);
    }
    | value LEQ value {
        $$ = new ConditionNode($1, "<=", $3);
        $$->setLineNumber(yylineno);
    }
    ;

value:
    NUM {
        $$ = new ValueNode($1);
    }
    | identifier {
        $$ = new ValueNode($1);
    }
    | MINUS NUM {
        $$ = new ValueNode($2, -1);
    }
    ;

identifier:
    PIDENTIFIER_TOKEN {
        $$ = new IdentifierNode($1);
    }
    | PIDENTIFIER_TOKEN LBRACK PIDENTIFIER_TOKEN RBRACK {
        $$ = new IdentifierNode($1, new IdentifierNode($3));
    }
    | PIDENTIFIER_TOKEN LBRACK NUM RBRACK {
        $$ = new IdentifierNode($1, $3);
    }
    ;

%%

void yyerror(const char *s) {
     // Suppress error messages
     fprintf(stderr, "Error: %s at line %d\n", s, yylineno);
}

int main(int argc, char** argv) {
    std::string inputFileName = "input.imp";
    std::string outputFileName = "output.mr";

    FILE* inputFile = fopen(inputFileName.c_str(), "r");
    if (!inputFile) {
        std::cerr << "Error: Could not open input file " << inputFileName << "\n";
        return 1;
    }
    yyin = inputFile;

    if (yyparse() == 0 && root != nullptr) {
        SymbolTable* tb;  

        try {
            tb = new SymbolTable(root); 
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            fclose(inputFile);
            return 1;
        }

        std::ofstream outputFile(outputFileName);
        if (!outputFile) {
            std::cerr << "Error: Could not open output file " << outputFileName << "\n";
            return 1;
        }
        
        std::streambuf* coutbuf = std::cout.rdbuf(); 
        std::cout.rdbuf(outputFile.rdbuf()); 
        
        CodeGenerator generate; 
        try {
            generate.generate_code(root, tb); 
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            fclose(inputFile);
            std::cout.rdbuf(coutbuf);
            return 1;
        }
        std::cout.rdbuf(coutbuf);
    } else {
        std::cerr << "There is no PROGRAM created!\n";
    }

    fclose(inputFile);
    return 0;
}

