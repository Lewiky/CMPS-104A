// Matthew Tan
// cmps104a
// mxtan
// Wesley Mackey
// asg1: stringset.h -- header file
// for stringset.cpp
// code credits goes to Professor Mackey

#ifndef __STRING_SET__
#define __STRING_SET__

#include <string>
#include <unordered_set>
using namespace std;

#include <stdio.h>

struct string_set {
    string_set();
    static unordered_set<string> set;
    static const string* intern_stringset(const char*);
    static void dump_stringset(FILE*);
};

#endif

