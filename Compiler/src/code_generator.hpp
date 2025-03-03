#ifndef CDG_HPP
#define CDG_HPP

#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>

#include "ast.hpp"
#include "symbol_table.hpp"


class CodeGeneratorError : public std::runtime_error
{
public:
    CodeGeneratorError(const std::string &message, int line)
        : std::runtime_error("\e[0;31mError:\e[0m " + message + " at line: " + std::to_string(line)) {}
};


class CodeGenerator
{
public:
    std::vector<std::string> instructions;
    std::unordered_map<std::string, long long> function_start;
    std::unordered_set<std::string> declared_functions;
    SymbolTable *symbolTable;

    std::string
    getName(std::string func, std::string var)
    {
        return func + "::" + var;
    }

    bool generate_load_to_RAX(ValueNode *node, std::string procName)
    {
        if (node->identifier)
        {
            std::string name = getName(procName, node->identifier->getName());
            if (node->identifier->isElement)
            {
                if (node->identifier->index_var)
                {
                    ValueNode *vn = new ValueNode(node->identifier->index_var);
                    generate_load_to_RAX(vn, procName);
                }
                else
                {
                    instructions.push_back("SET " + std::to_string(node->identifier->index_const));
                }
                instructions.push_back("STORE 1");
                std::pair<long long, bool> pid = symbolTable->getArrPid(name);
                if (pid.second)
                {
                    instructions.push_back("LOAD " + std::to_string(pid.first));
                }
                else
                {
                    instructions.push_back("SET " + std::to_string(pid.first));
                }
                instructions.push_back("ADD 1");
                instructions.push_back("LOADI 0");
            }
            else
            {
                std::pair<long long, bool> pid = symbolTable->getPid(name);
                if (pid.second)
                {
                    instructions.push_back("LOADI " + std::to_string(pid.first));
                }
                else
                {
                    instructions.push_back("LOAD " + std::to_string(pid.first));
                }
            }
        }
        else
        {
            instructions.push_back("SET " + std::to_string(node->value));
        }
        return true;
    }

    bool generate_save_from_RAX(ValueNode *node, std::string procName, bool ignore=false)
    {
        std::string name = getName(procName, node->identifier->getName());
        if (node->identifier->isElement)
        {
            instructions.push_back("STORE 2");
            if (node->identifier->index_var)
            {
                ValueNode *vn = new ValueNode(node->identifier->index_var);
                generate_load_to_RAX(vn, procName);
            }
            else
            {
                instructions.push_back("SET " + std::to_string(node->identifier->index_const));
            }
            instructions.push_back("STORE 1");
            std::pair<long long, bool> pid = symbolTable->getArrPid(name);
            if (pid.second)
            {
                instructions.push_back("LOAD " + std::to_string(pid.first));
            }
            else
            {
                instructions.push_back("SET " + std::to_string(pid.first));
            }
            instructions.push_back("ADD 1");
            instructions.push_back("STORE 1");
            instructions.push_back("LOAD 2");
            instructions.push_back("STOREI 1");
        }
        else
        {
            std::pair<long long, bool> pid = symbolTable->getPid(name);

            if (!ignore && (symbolTable->iterator_pid.find(pid.first) != symbolTable->iterator_pid.end()))
            {
                throw std::runtime_error("Cannot modify iterator: " + name);
            }

            if (pid.second)
            {
                instructions.push_back("STOREI " + std::to_string(pid.first));
            }
            else
            {
                instructions.push_back("STORE " + std::to_string(pid.first));
            }
        }
        return true;
    }

    bool generate_binary_expression(BinaryExpressionNode *expr, std::string procName)
    {
        if (expr->op == "+")
        {
            generate_addition(expr->left, expr->right, procName);
        }
        else if (expr->op == "-")
        {
            generate_substract(expr->left, expr->right, procName);
        }
        else if (expr->op == "*")
        {
            generate_multiplication(expr->left, expr->right, procName);
        }
        else if (expr->op == "/")
        {
            generate_division(expr->left, expr->right, procName);
        }
        else if (expr->op == "%")
        {
            generate_modulo(expr->left, expr->right, procName);
        }

        return true;
    }

    bool generate_substract(ValueNode *left, ValueNode *right, std::string procName)
    {
        generate_load_to_RAX(right, procName);
        // instructions.push_back("PUT 0");
        long long tmpPid = symbolTable->getNewPid();
        instructions.push_back("STORE " + std::to_string(tmpPid));
        generate_load_to_RAX(left, procName);
        // instructions.push_back("PUT 0");
        instructions.push_back("SUB " + std::to_string(tmpPid));
        // po wykonaniu operacji wynik jest w RAX
        return true;
    }

    bool generate_addition(ValueNode *left, ValueNode *right, std::string procName)
    {
        generate_load_to_RAX(right, procName);
        long long tmpPid = symbolTable->getNewPid();
        instructions.push_back("STORE " + std::to_string(tmpPid));
        generate_load_to_RAX(left, procName);
        instructions.push_back("ADD " + std::to_string(tmpPid));
        // po wykonaniu operacji wynik jest w RAX
        return true;
    }

    bool generate_sign(long long currPid, long long signPid)
    {
        instructions.push_back("JNEG 2");
        instructions.push_back("JUMP 6");
        instructions.push_back("SET 1");
        instructions.push_back("ADD " + std::to_string(signPid));
        instructions.push_back("STORE " + std::to_string(signPid));
        instructions.push_back("SET 0");
        instructions.push_back("SUB " + std::to_string(currPid));
        instructions.push_back("STORE " + std::to_string(currPid));
        return true;
    }

    bool generate_multiplication(ValueNode *left, ValueNode *right, std::string procName)
    {
        long long signPid = symbolTable->getNewPid();
        instructions.push_back("SET 0");
        instructions.push_back("STORE " + std::to_string(signPid));

        generate_load_to_RAX(right, procName);
        long long bPid = symbolTable->getNewPid();
        instructions.push_back("STORE " + std::to_string(bPid));
        generate_sign(bPid, signPid);

        generate_load_to_RAX(left, procName);
        long long aPid = symbolTable->getNewPid();
        instructions.push_back("STORE " + std::to_string(aPid));
        generate_sign(aPid, signPid);

        instructions.push_back("LOAD " + std::to_string(bPid));
        instructions.push_back("JZERO 2");
        instructions.push_back("JUMP 5");
        instructions.push_back("SET 1");
        instructions.push_back("STORE " + std::to_string(bPid));
        instructions.push_back("SET 0");
        instructions.push_back("STORE " + std::to_string(aPid));

        long long resultPid = symbolTable->getNewPid();
        instructions.push_back("SET 0");
        instructions.push_back("STORE " + std::to_string(resultPid));

        instructions.push_back("LOAD " + std::to_string(bPid));
        instructions.push_back("JPOS 2");
        instructions.push_back("JUMP 19");

        instructions.push_back("HALF");
        instructions.push_back("ADD 0");
        instructions.push_back("SUB " + std::to_string(bPid));
        instructions.push_back("JNEG 8");

        instructions.push_back("LOAD " + std::to_string(aPid));
        instructions.push_back("ADD 0");
        instructions.push_back("STORE " + std::to_string(aPid));
        instructions.push_back("LOAD " + std::to_string(bPid));
        instructions.push_back("HALF");
        instructions.push_back("STORE " + std::to_string(bPid));
        instructions.push_back("JUMP -12");

        instructions.push_back("LOAD " + std::to_string(aPid));
        instructions.push_back("ADD " + std::to_string(resultPid));
        instructions.push_back("STORE " + std::to_string(resultPid));
        instructions.push_back("SET -1");
        instructions.push_back("ADD " + std::to_string(bPid));
        instructions.push_back("STORE " + std::to_string(bPid));
        instructions.push_back("JUMP -19");

        instructions.push_back("SET -1");
        instructions.push_back("ADD " + std::to_string(signPid));
        instructions.push_back("JZERO 2");
        instructions.push_back("JUMP 4");

        instructions.push_back("SET 0");
        instructions.push_back("SUB " + std::to_string(resultPid));
        instructions.push_back("JUMP 2");

        instructions.push_back("LOAD " + std::to_string(resultPid));
        return true;
    }

    bool generate_division(ValueNode *left, ValueNode *right, std::string procName)
    {
        long long signPid = symbolTable->getNewPid();
        instructions.push_back("SET 0");
        instructions.push_back("STORE " + std::to_string(signPid));

        generate_load_to_RAX(right, procName);
        long long bPid = symbolTable->getNewPid();
        instructions.push_back("STORE " + std::to_string(bPid));
        generate_sign(bPid, signPid);

        std::string baseName = "#DIVISION_HELPER#";
        std::string name = getName(procName, baseName);
        symbolTable->zmienna_pid[name] = bPid;
        ValueNode *rightCopy = new ValueNode(new IdentifierNode(new std::string(baseName)));

        generate_load_to_RAX(left, procName);
        long long aPid = symbolTable->getNewPid();
        instructions.push_back("STORE " + std::to_string(aPid));
        generate_sign(aPid, signPid);

        // cheat division by 0
        instructions.push_back("LOAD " + std::to_string(bPid));
        instructions.push_back("JZERO 2");
        instructions.push_back("JUMP 5");
        instructions.push_back("SET 1");
        instructions.push_back("STORE " + std::to_string(bPid));
        instructions.push_back("SET 0");
        instructions.push_back("STORE " + std::to_string(aPid));

        long long resultPid = symbolTable->getNewPid();
        instructions.push_back("SET 0");
        instructions.push_back("STORE " + std::to_string(resultPid));

        long long iPid = symbolTable->getNewPid();
        instructions.push_back("SET " + std::to_string(LARGE_NUMBER));
        instructions.push_back("STORE " + std::to_string(iPid));

        long long bxdPid = symbolTable->getNewPid();
        generate_multiplication(rightCopy, new ValueNode(LARGE_NUMBER), procName);
        instructions.push_back("STORE " + std::to_string(bxdPid));
        instructions.push_back("JNEG 2");
        instructions.push_back("JUMP 4");
        instructions.push_back("SET 0");
        instructions.push_back("SUB " + std::to_string(bxdPid));
        instructions.push_back("STORE " + std::to_string(bxdPid));

        for (int i = 0; i < 64; i++)
        {
            instructions.push_back("LOAD " + std::to_string(aPid));
            instructions.push_back("SUB " + std::to_string(bxdPid));
            instructions.push_back("JNEG 5");
            instructions.push_back("STORE " + std::to_string(aPid));

            instructions.push_back("LOAD " + std::to_string(resultPid));
            instructions.push_back("ADD " + std::to_string(iPid));
            instructions.push_back("STORE " + std::to_string(resultPid));

            instructions.push_back("LOAD " + std::to_string(iPid));
            instructions.push_back("HALF");
            instructions.push_back("STORE " + std::to_string(iPid));
            instructions.push_back("LOAD " + std::to_string(bxdPid));
            instructions.push_back("HALF");
            instructions.push_back("STORE " + std::to_string(bxdPid));
        }

        instructions.push_back("SET -1");
        instructions.push_back("ADD " + std::to_string(signPid));
        instructions.push_back("JZERO 2");
        // if sign is positive
        long long jumpIdx = instructions.size();
        instructions.push_back("JUMP ");

        // if sign is negative
        instructions.push_back("LOAD " + std::to_string(resultPid));
        instructions.push_back("STORE " + std::to_string(bPid));
        generate_multiplication(rightCopy, right, procName);
        instructions.push_back("STORE " + std::to_string(bPid));
        generate_load_to_RAX(left, procName);
        instructions.push_back("SUB " + std::to_string(bPid));
        instructions.push_back("JZERO 4");
        instructions.push_back("SET 1");
        instructions.push_back("ADD " + std::to_string(resultPid));
        instructions.push_back("STORE " + std::to_string(resultPid));

        instructions.push_back("SET 0");
        instructions.push_back("SUB " + std::to_string(resultPid));
        instructions.push_back("JUMP 2");

        instructions[jumpIdx] += std::to_string(instructions.size() - jumpIdx);
        instructions.push_back("LOAD " + std::to_string(resultPid));
        return true;
    }

    bool generate_modulo(ValueNode *left, ValueNode *right, std::string procName)
    {
        generate_load_to_RAX(left, procName);
        long long aPid = symbolTable->getNewPid();
        instructions.push_back("STORE " + std::to_string(aPid));
        std::string baseName1 = "#MOD_1_HELPER#";
        std::string name1 = getName(procName, baseName1);
        symbolTable->zmienna_pid[name1] = aPid;
        ValueNode *leftCopy = new ValueNode(new IdentifierNode(new std::string(baseName1)));

        long long tmpPid = symbolTable->getNewPid();
        instructions.push_back("STORE " + std::to_string(tmpPid));

        generate_load_to_RAX(right, procName);
        long long bPid = symbolTable->getNewPid();
        instructions.push_back("STORE " + std::to_string(bPid));
        std::string baseName2 = "#MOD_2_HELPER#";
        std::string name2 = getName(procName, baseName2);
        symbolTable->zmienna_pid[name2] = bPid;
        ValueNode *rightCopy = new ValueNode(new IdentifierNode(new std::string(baseName2)));

        long long jumpIdx = instructions.size();
        instructions.push_back("JZERO "); // ??

        // if not zero
        generate_division(leftCopy, rightCopy, procName);
        instructions.push_back("STORE " + std::to_string(aPid));
        generate_multiplication(leftCopy, rightCopy, procName);
        instructions.push_back("STORE " + std::to_string(aPid));
        instructions.push_back("LOAD " + std::to_string(tmpPid));
        instructions.push_back("SUB " + std::to_string(aPid));

        instructions.push_back("JUMP 2");
        instructions[jumpIdx] += std::to_string(instructions.size() - jumpIdx);
        // if zero
        instructions.push_back("SET 0");

        return true;
    }

    bool generate_condition(ConditionNode *condition, std::string procName) // overload
    {
        generate_substract(condition->left, condition->right, procName);

        std::string op = condition->op;

        if (op == "=")
        {
            instructions.push_back("JZERO ");
            return true;
        }
        else if (op == "<")
        {
            instructions.push_back("JNEG ");
            return true;
        }
        else if (op == ">")
        {
            instructions.push_back("JPOS ");
            return true;
        }
        else if (op == "<=")
        {
            instructions.push_back("JPOS ");
            return false;
        }
        else if (op == ">=")
        {
            instructions.push_back("JNEG ");
            return false;
        }
        else if (op == "!=")
        {
            instructions.push_back("JZERO ");
            return false;
        }

        return true;
    }

    bool generate_if(IfNode *ifNode, std::string procName)
    {
        int beginIf = instructions.size();
        int endFirstPart = 0;
        bool elseFirst = generate_condition(ifNode->condition, procName);
        int endIf = instructions.size() - 1;

        if (elseFirst)
        {
            if (ifNode->elseCommands)
            {
                for (const auto &cmd : ifNode->elseCommands->commands)
                {
                    generate_command(cmd, procName);
                }
            }
        }
        else
        {
            for (const auto &cmd : ifNode->thenCommands->commands)
            {
                generate_command(cmd, procName);
            }
        }
        instructions.push_back("JUMP ");

        endFirstPart = instructions.size();
        instructions[endIf] += std::to_string(endFirstPart - endIf);

        if (!elseFirst)
        {
            if (ifNode->elseCommands)
            {
                for (const auto &cmd : ifNode->elseCommands->commands)
                {
                    generate_command(cmd, procName);
                }
            }
        }
        else
        {
            for (const auto &cmd : ifNode->thenCommands->commands)
            {
                generate_command(cmd, procName);
            }
        }

        instructions[endFirstPart - 1] += std::to_string(instructions.size() - endFirstPart + 1);

        return true;
    }

    bool generate_while(WhileNode *whileNode, std::string procName)
    {
        int beginWhile = instructions.size();
        int breakLabel = 0;
        bool elseFirst = generate_condition(whileNode->condition, procName);
        int endWhile = instructions.size() - 1;

        if (elseFirst)
        {
            breakLabel = instructions.size();
            instructions.push_back("JUMP ");
        }

        for (const auto &cmd : whileNode->commands->commands)
        {
            generate_command(cmd, procName);
        }
        instructions.push_back("JUMP " + std::to_string(beginWhile - (int)instructions.size()));

        if (elseFirst)
        {
            instructions[breakLabel] += std::to_string(instructions.size() - breakLabel);
            instructions[endWhile] += std::to_string(2);
        }
        else
        {
            instructions[endWhile] += std::to_string(instructions.size() - endWhile);
        }

        return true;
    }

    bool generate_assignment(AssignNode *assignCmd, std::string procName)
    {
        try {
            if (auto *binaryExpr = dynamic_cast<BinaryExpressionNode *>(assignCmd->expression))
            {
                generate_binary_expression(binaryExpr, procName);
            }
            else if (auto *valueExpr = dynamic_cast<ValueNode *>(assignCmd->expression))
            {
                generate_load_to_RAX(valueExpr, procName);
            }
            
            generate_save_from_RAX(new ValueNode(assignCmd->identifier), procName, assignCmd->ignore);
            return true;
        } catch (const std::runtime_error &e)
        {
            throw CodeGeneratorError(e.what(), assignCmd->getLineNumber());
            return false;
        }
    }

    bool generate_write(WriteNode *writeCmd, std::string procName)
    {
        generate_load_to_RAX(writeCmd->node, procName);
        instructions.push_back("PUT 0");
        return true;
    }

    bool generate_read(ReadNode *readCmd, std::string procName)
    {
        instructions.push_back("GET 0");
        generate_save_from_RAX(new ValueNode(readCmd->identifier), procName);
        return true;
    }

    bool generate_procedure_call(ProcedureCallNode *procCall, std::string procName)
    {
        std::string name = *procCall->procedureName;
        if (declared_functions.find(name) == declared_functions.end())
        {
            throw CodeGeneratorError("Procedure " + name + " not declared", procCall->getLineNumber());
            return false;
        }

        std::pair<long long, bool> pidOrg, pidFun;
        if (procCall->arguments)
        {
            long long i = 0;
            for (const auto &arg : procCall->arguments->getArguments())
            {
                if (symbolTable->funkcja_param[name][i].second) // brak **
                {
                    try {
                        pidOrg = symbolTable->getArrPid(getName(procName, arg->getName()));
                        pidFun = symbolTable->getArrPid(getName(name, symbolTable->funkcja_param[name][i++].first));
                    } catch (const std::runtime_error &e)
                    {
                        throw CodeGeneratorError("Wrong param in procedure " + name, procCall->getLineNumber());
                        return false;
                    }
                    if (pidOrg.second)
                    {
                        instructions.push_back("LOAD " + std::to_string(pidOrg.first));
                    }
                    else
                    {
                        instructions.push_back("SET " + std::to_string(pidOrg.first));
                    }
                    instructions.push_back("STORE " + std::to_string(pidFun.first));
                }
                else
                {
                    try {
                        pidOrg = symbolTable->getPid(getName(procName, arg->getName()));
                        pidFun = symbolTable->getPid(getName(name, symbolTable->funkcja_param[name][i++].first));
                    } catch (const std::runtime_error &e)
                    {
                        throw CodeGeneratorError("Wrong param in procedure " + name, procCall->getLineNumber());
                        return false;
                    }
                    // instructions.push_back("[ARG] " + std::to_string(pidOrg.first) + " -> " + std::to_string(pidFun.first));
                    if (pidOrg.second)
                    {
                        instructions.push_back("LOAD " + std::to_string(pidOrg.first));
                    }
                    else
                    {
                        instructions.push_back("SET " + std::to_string(pidOrg.first));
                    }
                    instructions.push_back("STORE " + std::to_string(pidFun.first));
                }
            }
        }
        instructions.push_back("SET " + std::to_string(instructions.size() + 3));
        instructions.push_back("STORE " + std::to_string(symbolTable->funkcja_RBX[name]));
        long long diff = function_start[name] - instructions.size();
        instructions.push_back("JUMP " + std::to_string(diff));
        return true;
    }

    bool generate_code(ProgramNode *root, SymbolTable *symbolTable)
    {
        this->symbolTable = symbolTable;
        long long main_pos = 0;
        instructions.push_back("JUMP ");
        if (root->procedures)
        {
            for (const auto &proc : root->procedures->procedures)
            {
                std::string procName = *proc->arguments->procedureName;
                function_start[procName] = instructions.size();
                for (const auto &cmd : proc->commands->commands)
                {
                    generate_command(cmd, procName);
                }
                instructions.push_back("RTRN " + std::to_string(symbolTable->funkcja_RBX[procName]));
                declared_functions.insert(procName);
            }
        }
        instructions[main_pos] += std::to_string(instructions.size());

        if (root->main)
        {
            std::string procName = "";
            for (const auto &cmd : root->main->commands->commands)
            {
                generate_command(cmd, procName);
            }
            instructions.push_back("HALT");
        }

        for (const auto &inst : instructions)
        {
            std::cout << inst << std::endl;
        }
        return true;
    }

    bool generate_command(CommandNode *cmd, std::string procName)
    {
        if (auto assignCmd = dynamic_cast<AssignNode *>(cmd))
        {
            return generate_assignment(assignCmd, procName);
        }
        else if (auto *procCall = dynamic_cast<ProcedureCallNode *>(cmd))
        {
            return generate_procedure_call(procCall, procName);
        }
        else if (auto *readCmd = dynamic_cast<ReadNode *>(cmd))
        {
            return generate_read(readCmd, procName);
        }
        else if (auto *writeCmd = dynamic_cast<WriteNode *>(cmd))
        {
            return generate_write(writeCmd, procName);
        }
        else if (auto *ifNode = dynamic_cast<IfNode *>(cmd))
        {
            return generate_if(ifNode, procName);
        }
        else if (auto *whileNode = dynamic_cast<WhileNode *>(cmd))
        {
            return generate_while(whileNode, procName);
        }
        else if (auto *forToNode = dynamic_cast<ForToNode *>(cmd))
        {
            return generate_for_to(forToNode, procName);
        }
        else if (auto *forDownToNode = dynamic_cast<ForDownToNode *>(cmd))
        {
            return generate_for_downto(forDownToNode, procName);
        }
        else if (auto *repeatUntilNode = dynamic_cast<RepeatUntilNode *>(cmd))
        {
            return generate_repeat_until(repeatUntilNode, procName);
        }

        return false;
    }

    bool generate_repeat_until(RepeatUntilNode *repeatUntilNode, std::string procName)
    {
        long long beginRepeat = instructions.size();
        for (const auto &cmd : repeatUntilNode->commands->commands)
        {
            generate_command(cmd, procName);
        }
        bool elseFirst = generate_condition(repeatUntilNode->condition, procName);
        long long endIf = instructions.size() - 1;

        if (!elseFirst)
        {
            instructions[endIf] += std::to_string(beginRepeat - endIf);
        }
        else
        {
            instructions[endIf] += std::to_string(2);
            instructions.push_back("JUMP " + std::to_string(beginRepeat - (long long)instructions.size()));
        }
        return true;
    }

    bool generate_for_to(ForToNode *forToNode, std::string procName)
    {
        std::string baseName = forToNode->pidentifier->getName();
        std::string name = getName(procName, forToNode->pidentifier->getName());
        symbolTable->zmienna_pid[name] = symbolTable->getNewPid();
        symbolTable->iterator_pid.insert(symbolTable->zmienna_pid[name]);

        // i = start
        generate_load_to_RAX(forToNode->fromValue, procName);
        instructions.push_back("STORE " + std::to_string(symbolTable->zmienna_pid[name]));

        std::string baseEndName = baseName + "::END";
        std::string endName = name + "::END";
        symbolTable->zmienna_pid[endName] = symbolTable->getNewPid();

        // i_end = koniec
        generate_load_to_RAX(forToNode->toValue, procName);
        instructions.push_back("STORE " + std::to_string(symbolTable->zmienna_pid[endName]));

        AssignNode *assgn = new AssignNode(new IdentifierNode(new std::string(baseName)), new BinaryExpressionNode(new ValueNode(new IdentifierNode(new std::string(baseName))), "+", new ValueNode(1)), true);
        forToNode->commands->commands.push_back(assgn);
        ConditionNode *cond = new ConditionNode(new ValueNode(new IdentifierNode(new std::string(baseName))), "<=", new ValueNode(new IdentifierNode(new std::string(baseEndName))));
        WhileNode *whileNode = new WhileNode(cond, forToNode->commands);
        generate_while(whileNode, procName);

        return true;
    }

    bool generate_for_downto(ForDownToNode *forToNode, std::string procName)
    {
        std::string baseName = forToNode->pidentifier->getName();
        std::string name = getName(procName, forToNode->pidentifier->getName());
        symbolTable->zmienna_pid[name] = symbolTable->getNewPid();
        symbolTable->iterator_pid.insert(symbolTable->zmienna_pid[name]);

        // i = start
        generate_load_to_RAX(forToNode->fromValue, procName);
        instructions.push_back("STORE " + std::to_string(symbolTable->zmienna_pid[name]));

        std::string baseEndName = baseName + "::END";
        std::string endName = name + "::END";
        symbolTable->zmienna_pid[endName] = symbolTable->getNewPid();

        // i_end = koniec
        generate_load_to_RAX(forToNode->toValue, procName);
        instructions.push_back("STORE " + std::to_string(symbolTable->zmienna_pid[endName]));

        AssignNode *assgn = new AssignNode(new IdentifierNode(new std::string(baseName)), new BinaryExpressionNode(new ValueNode(new IdentifierNode(new std::string(baseName))), "-", new ValueNode(1)), true);
        forToNode->commands->commands.push_back(assgn);
        ConditionNode *cond = new ConditionNode(new ValueNode(new IdentifierNode(new std::string(baseName))), ">=", new ValueNode(new IdentifierNode(new std::string(baseEndName))));
        WhileNode *whileNode = new WhileNode(cond, forToNode->commands);
        generate_while(whileNode, procName);

        return true;
    }
};

#endif // CDG_HPP