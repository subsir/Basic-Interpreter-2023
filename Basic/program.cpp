/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

Program::Program() = default;

Program::~Program() = default;

void Program::clear() {
  int min_index = -1;
  index.clear();
  pg.clear();
  sm.clear();
  go.clear();
  opif.clear();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
  pg[lineNumber] = line;
  auto it = std::find(index.begin(), index.end(), lineNumber);
  if (it != index.end()) {
    sm.erase(lineNumber);
    go.erase(lineNumber);
    opif.erase(lineNumber);
  } else {
    index.push_back(lineNumber);
  }
  if (min_index == -1 or index[min_index] > lineNumber) {
    min_index = index.size() - 1;
  }
}

void Program::removeSourceLine(int lineNumber) {
  auto it = pg.find(lineNumber);
  if (it != pg.end()) {
    pg.erase(it);
    sm.erase(lineNumber);
    go.erase(lineNumber);
    opif.erase(lineNumber);
    auto index_of_lineNum = std::find(index.begin(), index.end(), lineNumber);
    index.erase(index_of_lineNum);
  }
}

std::string Program::getSourceLine(int lineNumber) {
  auto it = pg.find(lineNumber);
  if (it != pg.end()) {
    return it->second;
  }
  return "";
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {}

Statement *Program::getParsedStatement(int lineNumber) {
  auto it = sm.find(lineNumber);
  if (it != sm.end()) {
    return it->second;
  }
  return nullptr;
}

std::vector<int> Program::getLISTLineNumber() {
  std::vector<int> list;
  std::sort(index.begin(), index.end());
  if (index.size() == 0) {
    return list;
  }
  int i = 0;
  while (i != index.size()) {
    list.push_back(index[i]);
    if (go.find(index[i]) != go.end()) {
      auto it = std::find(index.begin(), index.end(), go[index[i]]);
      if (it != index.end()) {
        i = it - index.begin();
      } else {
        list.push_back(-2); // -2 represents error
        return list;
      }
    } else if (opif.find(index[i]) != opif.end()) {
      auto it = std::find(index.begin(), index.end(), opif[index[i]]);
      if (it != index.end()) {
        i = it - index.begin();
      } else {
        list.push_back(-2); // -2 represents error
        return list;
      }
    } else {
      i++;
    }
  }
  return list;
}

int Program::getNextLineNumber(int lineNumber) {
  std::sort(index.begin(), index.end());
  if (index.size() == 0) {
    return -1;
  }
  int i = std::find(index.begin(), index.end(), lineNumber) - index.begin();
  if (go.find(index[i]) != go.end()) {
    auto it = std::find(index.begin(), index.end(), go[index[i]]);
    if (it != index.end()) {
      i = it - index.begin();
    } else {
      return -2; // -2 represents error
    }
  } else if (opif.find(index[i]) != opif.end()) {
    auto it = std::find(index.begin(), index.end(), opif[index[i]]);
    if (it != index.end()) {
      i = it - index.begin();
    } else {
      return -2;
    }
  } else {
    i++;
  }
  if (i == index.size()) {
    return -1;
  }
  return index[i];
}

int Program::getLowestNumber() {
  if (min_index == -1) {
    return -1;
  }
  return index[min_index];
}

void Program::addGOTOoperation(int lineNumber, int nextLineNumber) {
  go[lineNumber] = nextLineNumber;
}

void Program::addIFoperation(int lineNumber, int nextLineNumber) {
  opif[lineNumber] = nextLineNumber;
}

void Program::IFclear() { opif.clear(); }