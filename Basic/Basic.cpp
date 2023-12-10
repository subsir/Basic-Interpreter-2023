/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include "Utils/error.hpp"
#include "Utils/strlib.hpp"
#include "Utils/tokenScanner.hpp"
#include "evalstate.hpp"
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>

/* Function prototypes */
bool runstate = false; // run state
void processLine(std::string line, Program &program, EvalState &state,
                 int run_lineNum);

/* Main program */

int main() {
  EvalState state;
  Program program;
  // cout << "Stub implementation of BASIC" << endl;
  while (true) {
    try {
      std::string input;
      getline(std::cin, input);
      if (input.empty())
        return 0;
      processLine(input, program, state, -1);
    } catch (ErrorException &ex) {
      std::cout << ex.getMessage() << std::endl;
    }
  }
  return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state,
                 int run_lineNum) {

  TokenScanner scanner;
  scanner.ignoreWhitespace();
  scanner.scanNumbers();
  scanner.setInput(line);
  if (scanner.hasMoreTokens()) {
    std::string token = scanner.nextToken();
    int lineNum = -1;
    if (scanner.getTokenType(token) == NUMBER) {
      lineNum = std::stoi(token);
      program.addSourceLine(lineNum, line);
      if (scanner.hasMoreTokens() == false) {
        program.removeSourceLine(lineNum);
      } else {
        token = scanner.nextToken();
        if (token == "GOTO") {
          if (scanner.hasMoreTokens()) {
            token = scanner.nextToken();
            int next_lineNum = std::stoi(token);
            program.addGOTOoperation(lineNum, next_lineNum);
          }
        }
      }
      return;
    }
    if (token == "REM") {
    } else if (token == "GOTO") {
    } else if (token == "LET") {
      if (scanner.hasMoreTokens()) {
        CompoundExp *exp = dynamic_cast<CompoundExp *>(readE(scanner));
        if (exp != nullptr) {
          if (exp->getLHS()->getType() == IDENTIFIER and
              exp->getLHS()->toString() == "LET") {
            delete exp;
            error("SYNTAX ERROR");
          }
          if (exp->getLHS()->getType() != IDENTIFIER) {
            delete exp;
            error("Illegal variable in assignment");
          }
          std::string var = exp->getLHS()->toString();
          int value;
          try {
            value = exp->getRHS()->eval(state);
          } catch (ErrorException &ex) {
            delete exp;
            error(ex.getMessage());
          }
          state.setValue(var, value);
          delete exp;
        } else {
          std::cout << "what?";
          exit(0);
        }
      }
    } else if (token == "IF") {
      if (scanner.hasMoreTokens()) {
        int lhs, rhs;
        int op_index, lhs_index, rhs_index;
        program.IFclear();
        std::string op = "";
        for (int i = 0; i < line.size(); i++) {
          if (line[i] == '<') {
            op = "<";
            op_index = i;
          } else if (line[i] == '>') {
            op = ">";
            op_index = i;
          } else if (line[i] == '=') {
            op = "=";
            op_index = i;
          } else if (line[i] == 'T') {
            rhs_index = i - 1;
          } else if (line[i] == 'F') {
            lhs_index = i + 1;
          }
        }
        scanner.setInput(line.substr(lhs_index, op_index - lhs_index - 1));
        Expression *explhs = readE(scanner);
        try {
          lhs = explhs->eval(state);
        } catch (ErrorException &ex) {
          delete explhs;
          error(ex.getMessage());
        }
        scanner.setInput(line.substr(op_index + 1, rhs_index - op_index - 1));
        Expression *exprhs = readE(scanner);
        try {
          rhs = exprhs->eval(state);
        } catch (ErrorException &ex) {
          delete exprhs;
          error(ex.getMessage());
        }
        bool operate = false;
        if (op == "=" and lhs == rhs) {
          operate = true;
        } else if (op == "<" and lhs < rhs) {
          operate = true;
        } else if (op == ">" and lhs > rhs) {
          operate = true;
        }
        scanner.setInput(line);
        if (operate) {
          while (scanner.hasMoreTokens() and token != "THEN") {
            token = scanner.nextToken();
          }
          if (scanner.hasMoreTokens()) {
            token = scanner.nextToken();
            int next_lineNum = std::stoi(token);
            program.addIFoperation(run_lineNum, next_lineNum);
          }
        }
        delete explhs;
        delete exprhs;
      }
    } else if (token == "PRINT") {
      Expression *exp = readE(scanner);
      if (exp != nullptr) {
        try {
          std::cout << exp->eval(state) << '\n';
        } catch (ErrorException &ex) {
          delete exp;
          error(ex.getMessage());
        }
        delete exp;
      }
    } else if (token == "INPUT") {
      if (scanner.hasMoreTokens()) {
        Expression *exp = readE(scanner);
        Expression *exp2;
        bool valid = false;
        int value;
        while (valid == false) {
          std::cout << " ? ";
          getline(std::cin, line);
          scanner.setInput(line);
          scanner.scanNumbers();
          valid = true;
          if (line.find(".") != -1) {
            valid = false;
            std::cout << "INVALID NUMBER\n";
            continue;
          }
          try {
            exp2 = parseExp(scanner);
          } catch (...) {
            valid = false;
            std::cout << "INVALID NUMBER\n";
            continue;
          }
          if (exp2->getType() == COMPOUND) {
            CompoundExp *exp3 = dynamic_cast<CompoundExp *>(exp2);
            if (line.find(exp3->getOp()) != 0) {
              valid = false;
              delete exp2;
              std::cout << "INVALID NUMBER\n";
              continue;
            }
          }
          try {
            value = exp2->eval(state);
          } catch (...) {
            valid = false;
            delete exp2;
            std::cout << "INVALID NUMBER\n";
          }
        }
        state.setValue(exp->toString(), value);
        delete exp;
        delete exp2;
      }
    } else if (token == "END") {
      runstate = false;
    } else if (token == "RUN") {
      runstate = true;
      int i = program.getLowestNumber();
      while (runstate and i != -1) {
        if (i == -2) {
          std::cout << "LINE NUMBER ERROR\n";
          return;
        }
        std::string run_line = program.getSourceLine(i);
        int begin = run_line.find(" ");
        run_lineNum = i;
        std::string line2 = run_line.substr(begin);
        processLine(line2, program, state, run_lineNum);
        i = program.getNextLineNumber(i);
      }
      runstate = false;
    } else if (token == "LIST") {
      std::vector<int> list = program.getLISTLineNumber();
      for (int i : list) {
        std::cout << program.getSourceLine(i) << '\n';
      }
    } else if (token == "QUIT") {
      exit(0);
    } else if (token == "CLEAR") {
      state.Clear();
      program.clear();
    }
    if (lineNum != -1) {
    }
  }
}
