#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>
#include <string>

class AstNode
{
public:
    AstNode() : lineNumber(0) {}
    virtual ~AstNode() = default;
    
    void setLineNumber(int line) { lineNumber = line; }
    int getLineNumber() const { return lineNumber; }
    int lineNumber = 0;
};

class ExpressionNode : public AstNode
{
public:
    // ~ExpressionNode() override = default;
};

class IdentifierNode : public ExpressionNode
{
public:
    explicit IdentifierNode(std::string *varName) // zmienna
        : name(varName)
    {
    }
    IdentifierNode(std::string *varName, IdentifierNode *idx) // zmienna[zmienna]
        : name(varName), index_var(idx), isElement(true)
    {
    }
    IdentifierNode(std::string *varName, long long idx) // zmienna[liczba]
        : name(varName), index_const(idx), isElement(true)
    {
    }
    IdentifierNode(std::string *varName, long long startIdx, long long endIdx) // zmienna[st:kon]
        : name(varName), start(startIdx), end(endIdx), isArray(true)
    {
    }

    ~IdentifierNode()
    {
        delete name;
        delete index_var;
    }

    std::string getName() const
    {
        return *name;
    }

    bool getIsArray() const
    {
        return isArray;
    }

    long long getStart() const
    {
        return start;
    }

    long long getEnd() const
    {
        return end;
    }

     
    std::string *name;
    IdentifierNode *index_var = nullptr;
    long long index_const = 0;
    long long start = 0, end = 0;
    bool isArray = false;
    bool isElement = false;
};

class ValueNode : public ExpressionNode
{
public:
    ValueNode(long long val, int minus = 1) : value(val * minus), identifier(nullptr) {}
    ValueNode(IdentifierNode *id) : value(0), identifier(id) {}

    long long value;
    IdentifierNode *identifier;
};

class ArgumentNode : public AstNode
{
public:
    explicit ArgumentNode(std::string *varName)
        : argumentName(varName) {}
    ArgumentNode(std::string *varName, bool isArr)
        : argumentName(varName), isArray(isArr) {}

    ~ArgumentNode()
    {
        delete argumentName;
    }

    const std::string *getArgumentName() const
    {
        return argumentName;
    }

    bool getIsArray() const
    {
        return isArray;
    }

    std::string *argumentName;
    bool isArray = false;
};

class ArgumentsDeclarationNode : public AstNode
{
public:
    void addVariableArgument(std::string *varName)
    {
        arguments.push_back(new ArgumentNode(varName));
    }
    void addArrayArgument(std::string *varName)
    {
        arguments.push_back(new ArgumentNode(varName, true));
    }

    const std::vector<ArgumentNode *> &getArguments() const
    {
        return arguments;
    }

    ~ArgumentsDeclarationNode()
    {
        for (auto arg : arguments)
        {
            delete arg;
        }
    }

    std::vector<ArgumentNode *> arguments;
};

class ProcedureHeadNode : public AstNode
{
public:
    explicit ProcedureHeadNode(std::string *procName, ArgumentsDeclarationNode *args) : procedureName(procName), arguments(args) {}

    ~ProcedureHeadNode()
    {
        delete procedureName;
        delete arguments;
    }

    const std::string *getProcedureName() const
    {
        return procedureName;
    }

    const ArgumentsDeclarationNode *getArguments() const
    {
        return arguments;
    }

    std::string *procedureName;
    ArgumentsDeclarationNode *arguments;
};

class ProcedureCallArguments : public AstNode
{
public:
    ~ProcedureCallArguments()
    {
        for (auto arg : arguments)
        {
            delete arg;
        }
    }

    void addArgument(std::string *argName)
    {
        arguments.push_back(new IdentifierNode(argName));
    }

    const std::vector<IdentifierNode *> &getArguments() const
    {
        return arguments;
    }

    std::vector<IdentifierNode *> arguments;
};

class BinaryExpressionNode : public ExpressionNode
{
public:
    BinaryExpressionNode(ValueNode *lhs, const std::string &operator_, ValueNode *rhs) : left(lhs), op(operator_), right(rhs) {}

    ~BinaryExpressionNode()
    {
        delete left;
        delete right;
    }

    ValueNode *left;
    std::string op;
    ValueNode *right;
};

class ConditionNode : public AstNode
{
public:
    ConditionNode(ValueNode *lhs, const std::string &operator_, ValueNode *rhs) : left(lhs), op(operator_), right(rhs) {}

    ~ConditionNode()
    {
        delete left;
        delete right;
    }

    ValueNode *left;
    std::string op;
    ValueNode *right;
};

class DeclarationsNode : public AstNode
{
public:
    ~DeclarationsNode()
    {
        for (auto decl : declarations)
        {
            delete decl;
        }
    }

    void addVariableDeclaration(std::string *varName)
    {
        declarations.push_back(new IdentifierNode(varName));
    }

    void addArrayDeclaration(std::string *varName, long long startIdx, long long endIdx)
    {
        declarations.push_back(new IdentifierNode(varName, startIdx, endIdx));
    }

    void addArrayDeclaration(std::string *varName, ValueNode *startIdx, ValueNode *endIdx)
    {
        declarations.push_back(new IdentifierNode(varName, startIdx->value, endIdx->value));
    }

    const std::vector<IdentifierNode *> &getDeclarations() const
    {
        return declarations;
    }

     
    std::vector<IdentifierNode *> declarations;
};

class CommandNode : public AstNode
{
    //~CommandNode() override = default;
};

class CommandsNode : public AstNode
{
public:
    ~CommandsNode()
    {
        for (auto cmd : commands)
        {
            delete cmd;
        }
    }
    void addCommand(CommandNode *command)
    {
        commands.push_back(command);
    }

    const std::vector<CommandNode *> &getCommands() const
    {
        return commands;
    }



     
    std::vector<CommandNode *> commands;
};

class AssignNode : public CommandNode
{
public:
    explicit AssignNode(IdentifierNode *id, ExpressionNode *exp, bool ign=false) : identifier(id), expression(exp), ignore(ign) {}

    ~AssignNode()
    {
        delete identifier;
        delete expression;
    }

    IdentifierNode *getIdentifier() const
    {
        return identifier;
    }

    ExpressionNode *getExpression() const
    {
        return expression;
    }

    

     
    bool ignore;
    IdentifierNode *identifier;
    ExpressionNode *expression;
};

class IfNode : public CommandNode
{
public:
    IfNode(ConditionNode *cond, CommandsNode *thenCmds, CommandsNode *elseCmds = nullptr)
        : condition(cond), thenCommands(thenCmds), elseCommands(elseCmds) {}

    ~IfNode()
    {
        delete condition;
        delete thenCommands;
        delete elseCommands;
    }

    const CommandsNode *getThenCommands() const
    {
        return thenCommands;
    }

    const CommandsNode *getElseCommands() const
    {
        return elseCommands;
    }


     
    ConditionNode *condition;
    CommandsNode *thenCommands;
    CommandsNode *elseCommands = nullptr;
};

class WhileNode : public CommandNode
{
public:
    explicit WhileNode(ConditionNode *cond, CommandsNode *cmds)
        : condition(cond), commands(cmds) {}

    ~WhileNode()
    {
        delete condition;
        delete commands;
    }

    const CommandsNode *getCommands() const
    {
        return commands;
    }

     
    ConditionNode *condition;
    CommandsNode *commands;
};

class RepeatUntilNode : public CommandNode
{
public:
    explicit RepeatUntilNode(ConditionNode *cond, CommandsNode *comms)
        : condition(cond), commands(comms) {}

    ~RepeatUntilNode()
    {
        delete condition;
        delete commands;
    }

    const CommandsNode *getCommands() const
    {
        return commands;
    }


     
    ConditionNode *condition;
    CommandsNode *commands;
};

class ForToNode : public CommandNode
{
public:
    explicit ForToNode(std::string *pid, ValueNode *fromVal, ValueNode *toVal, CommandsNode *comms) : pidentifier(new IdentifierNode(pid)), fromValue(fromVal), toValue(toVal), commands(comms) {}

    ~ForToNode()
    {
        delete pidentifier;
        delete fromValue;
        delete toValue;
        delete commands;
    }

    const CommandsNode *getCommands() const
    {
        return commands;
    }



     
    IdentifierNode *pidentifier;
    ValueNode *fromValue;
    ValueNode *toValue;
    CommandsNode *commands;
};

class ForDownToNode : public CommandNode
{
public:
    explicit ForDownToNode(std::string *pid, ValueNode *fromVal, ValueNode *toVal, CommandsNode *comms) : pidentifier(new IdentifierNode(pid)), fromValue(fromVal), toValue(toVal), commands(comms) {}
    ~ForDownToNode()
    {
        delete pidentifier;
        delete fromValue;
        delete toValue;
        delete commands;
    }

    const CommandsNode *getCommands() const
    {
        return commands;
    }

    IdentifierNode *pidentifier;
    ValueNode *fromValue;
    ValueNode *toValue;
    CommandsNode *commands;
};

class ProcedureCallNode : public CommandNode
{
public:
    explicit ProcedureCallNode(std::string *pidentifier, ProcedureCallArguments *args) : procedureName(pidentifier), arguments(args) {}
    ~ProcedureCallNode()
    {
        delete procedureName;
        delete arguments;
    }

    const std::string *getProcedureName() const
    {
        return procedureName;
    }

    const ProcedureCallArguments *getArguments() const
    {
        return arguments;
    }



     
    std::string *procedureName;
    ProcedureCallArguments *arguments;
};

class WriteNode : public CommandNode
{
public:
    WriteNode(ValueNode *nd) : node(nd) {}
    ~WriteNode()
    {
        delete node;
    }


     
    ValueNode *node;
};

class ReadNode : public CommandNode
{
public:
    explicit ReadNode(IdentifierNode *val) : identifier(val) {}

    ~ReadNode()
    {
        delete identifier;
    }

    IdentifierNode *getIdentifier() const
    {
        return identifier;
    }


     
    IdentifierNode *identifier;
};

class MainNode : public AstNode
{
public:
    explicit MainNode(DeclarationsNode *decl, CommandsNode *cmds) : declarations(decl), commands(cmds) {}
    explicit MainNode(CommandsNode *cmds) : declarations(nullptr), commands(cmds) {}
    ~MainNode()
    {
        delete declarations;
        delete commands;
    }

    const DeclarationsNode *getDeclarations() const
    {
        return declarations;
    }

    const CommandsNode *getCommands() const
    {
        return commands;
    }



     
    DeclarationsNode *declarations;
    CommandsNode *commands;
};

class ProcedureNode : public AstNode
{
public:
    explicit ProcedureNode(ProcedureHeadNode *args, DeclarationsNode *decls, CommandsNode *comms = nullptr) : arguments(args), declarations(decls), commands(comms) {}
    ~ProcedureNode()
    {
        delete arguments;
        delete declarations;
        delete commands;
    }

    const ProcedureHeadNode *getArguments() const
    {
        return arguments;
    }
    const DeclarationsNode *getDeclarations() const
    {
        return declarations;
    }
    const CommandsNode *getCommands() const
    {
        return commands;
    }


     
    ProcedureHeadNode *arguments;
    DeclarationsNode *declarations;
    CommandsNode *commands;
};

class ProceduresNode : public AstNode
{
public:
    ~ProceduresNode()
    {
        for (auto proc : procedures)
        {
            delete proc;
        }
    }
    void addProcedure(ProcedureNode *procedure)
    {
        procedures.push_back(procedure);
    }
    const std::vector<ProcedureNode *> &getProcedures() const
    {
        return procedures;
    }


     
    std::vector<ProcedureNode *> procedures;
};

class ProgramNode : public AstNode
{
public:
    ProgramNode() : main(nullptr), procedures(nullptr) {}
    void addProcedures(ProceduresNode *procs)
    {
        procedures = procs;
    }

    void addMain(MainNode *mainNode)
    {
        main = mainNode;
    }

    const ProceduresNode *getProcedures() const
    {
        return procedures;
    }

    const MainNode *getMain() const
    {
        return main;
    }

    ~ProgramNode()
    {
        delete main;
        delete procedures;
    }

     
    MainNode *main;
    ProceduresNode *procedures;
};

#endif // AST_HPP
