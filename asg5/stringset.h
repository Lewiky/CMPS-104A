// Matthew Tan
// mxtan
// cs104a
// asg5: stringset.h

#ifndef __STRINGSET__
#define __STRINGSET__

#include <string>
#include <unordered_set>
using namespace std;

#include <stdio.h>

struct stringset {
   stringset();
   static unordered_set<string> set;
   static const string* intern (const char*);
   static void dump (FILE*);
};

#endif

