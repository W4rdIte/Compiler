#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <vector>

#include "ast.hpp"

class SymbolTable
{
public:
    long long pid = 3;                                                                        // RAX, RBX, RCX
    std::unordered_map<std::string, long long> zmienna_pid;                                   // main -> a, b, c, d, x, y
    std::unordered_map<std::string, long long> tablica_indeks_pid;                            // main T
    std::unordered_map<std::string, long long> parametr_pid;                                  // gcd -> gcd::a, gcd::b, gcd::c
    std::unordered_map<std::string, std::vector<std::pair<std::string, bool>>> funkcja_param; // gcd(a, b, c);
    std::unordered_map<std::string, long long> tablica_param_pid;                             // gcd::x, gcd::y
    std::unordered_map<std::string, long long> funkcja_RBX;                                   // adres powrotu dla funkcji
    std::unordered_set<long long> iterator_pid;

    std::string getName(std::string func, std::string var)
    {
        return func + "::" + var;
    }

    class SymbolTableError : public std::runtime_error
    {
    public:
        SymbolTableError(const std::string &message, int line)
            : std::runtime_error("\e[0;31mError:\e[0m " + message + " at line: " + std::to_string(line)) {}
    };

    SymbolTable(ProgramNode *root)
    {
        if (root->procedures)
        {
            for (const auto &proc : root->procedures->procedures)
            {
                std::string procName = *proc->arguments->procedureName;
                funkcja_RBX[procName] = pid++;
                if (proc->arguments)
                {
                    funkcja_param[procName] = {};
                    for (const auto &arg : proc->arguments->arguments->arguments)
                    {
                        std::string argName = *arg->argumentName;
                        std::string name = getName(procName, argName);
                        try {
                            ensureUnique(name);
                        } catch (const std::runtime_error &e)
                        {
                            throw SymbolTableError(e.what(), proc->arguments->getLineNumber());
                        }
                        if (!arg->isArray)
                        {
                            parametr_pid[name] = pid++;
                            funkcja_param[procName].push_back({argName, false});
                        }
                        else
                        {
                            tablica_param_pid[name] = pid++;
                            funkcja_param[procName].push_back({argName, true});
                        }
                    }
                }

                if (proc->declarations)
                {
                    for (const auto &decl : proc->declarations->declarations)
                    {
                        std::string name = getName(procName, *decl->name);
                        try {
                            ensureUnique(name);
                        } catch (const std::runtime_error &e)
                        {
                            throw SymbolTableError(e.what(), proc->declarations->getLineNumber());
                        }
                        if (!decl->isArray)
                        {
                            zmienna_pid[name] = pid++;
                        }
                        else
                        {
                            tablica_indeks_pid[name] = pid - decl->start;
                            for (int i = decl->start; i <= decl->end; i++)
                            {
                                pid++;
                            }
                        }
                    }
                }
            }
        }

        if (root->main && root->main->declarations)
        {
            for (const auto &decl : root->main->declarations->declarations)
            {
                std::string name = getName("", *decl->name);
                try {
                    ensureUnique(name);
                } catch (const std::runtime_error &e)
                {
                    throw SymbolTableError(e.what(), root->main->getLineNumber());
                }
                if (!decl->isArray)
                {
                    zmienna_pid[name] = pid++;
                }
                else
                {
                    tablica_indeks_pid[name] = pid - decl->start;
                    for (int i = decl->start; i <= decl->end; i++)
                    {
                        pid++;
                    }
                }
            }
        }
    }

    std::pair<long long, bool> getPid(std::string name)
    {
        if (zmienna_pid.find(name) != zmienna_pid.end())
        {
            return {zmienna_pid[name], false};
        }
        else if (parametr_pid.find(name) != parametr_pid.end())
        {
            return {parametr_pid[name], true};
        }
        else if (tablica_indeks_pid.find(name) != tablica_indeks_pid.end())
        {
            throw std::runtime_error("Array wrongly used: " + name);
        }
        else if (tablica_param_pid.find(name) != tablica_param_pid.end())
        {
            throw std::runtime_error("Array wrongly used: " + name);
        } 
        else {
            throw std::runtime_error("Variable not declared: " + name);
        }
    }

    void ensureUnique(std::string name) {
        if (zmienna_pid.find(name) != zmienna_pid.end())
        {
            throw std::runtime_error("Identifier already used: " + name);
        }
        else if (parametr_pid.find(name) != parametr_pid.end())
        {
            throw std::runtime_error("Identifier already used: " + name);
        }
        else if (tablica_indeks_pid.find(name) != tablica_indeks_pid.end())
        {
            throw std::runtime_error("Identifier already used: " + name);
        }
        else if (tablica_param_pid.find(name) != tablica_param_pid.end())
        {
            throw std::runtime_error("Identifier already used: " + name);
        }
    }

    std::pair<long long, bool> getArrPid(std::string name)
    {
        if (tablica_indeks_pid.find(name) != tablica_indeks_pid.end())
        {
            return {tablica_indeks_pid[name], false};
        }
        else if (tablica_param_pid.find(name) != tablica_param_pid.end())
        {
            return {tablica_param_pid[name], true};
        }
        else if (zmienna_pid.find(name) != zmienna_pid.end())
        {
            throw std::runtime_error("Variable wrongly used: " + name);
        }
        else if (parametr_pid.find(name) != parametr_pid.end())
        {
            throw std::runtime_error("Variable wrongly used: " + name);
        } else {
            throw std::runtime_error("Array not declared: " + name);
        }
    }

    long long getNewPid()
    {
        return pid++;
    }
};

#endif // SYMBOL_TABLE_HPP
