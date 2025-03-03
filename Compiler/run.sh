#!/bin/bash

# Check if an argument is provided
if [ -z "$1" ]; then
  echo "No argument provided. Use 'cln' or 'long'. 'c' to clean"
  exit 1
fi

# Handle the argument
if [ "$1" == "long" ]; then
  bison -d -Wcounterexamples -o src/parser.tab.c src/parser.y
  flex -o src/lex.yy.c src/lexer.l
  g++ -DLARGE_NUMBER=2147483648 -o compiler src/parser.tab.c src/lex.yy.c -lfl -std=c++11 
  echo "Compiler for 'long long'."

elif [ "$1" == "cln" ]; then
  bison -d -Wcounterexamples -o src/parser.tab.c src/parser.y
  flex -o src/lex.yy.c src/lexer.l
  g++ -DLARGE_NUMBER=4611686018427387904 -o compiler src/parser.tab.c src/lex.yy.c -lfl -std=c++11 
  echo "Compiler for 'cln'."

elif [ "$1" == "c" ]; then
  rm -f src/lex.yy.c src/parser.tab.c src/parser.tab.h compiler
  echo "Clean up completed."

else
  echo "No argument provided. Use 'cln' or 'long'! 'c' to clean"
  exit 1
fi
