// Matthew Tan
// mxtan
// cs104a
// asg4: stringset.cpp

#include <string>
#include <unordered_set>
using namespace std;

#include "auxlib.h"
#include "stringset.h"

unordered_set<string> stringset::set;

stringset::stringset() {
   set.max_load_factor (0.5);
}

const string* stringset::intern (const char* string) {
   auto handle = set.insert (string);
   //DEBUGF ('s', "inserted \"%s\" %s\n", handle.first->c_str(),
   //        handle.second ? "newly inserted" : "already there");
   return &*handle.first;
}

void stringset::dump (FILE* out) {
   static unordered_set<string>::hasher hash_fn
               = stringset::set.hash_function();
   size_t max_bucket_size = 0;
   for (size_t bucket = 0; bucket < set.bucket_count(); ++bucket) {
      bool need_index = true;
      size_t curr_size = set.bucket_size (bucket);
      if (max_bucket_size < curr_size) max_bucket_size = curr_size;
      for (auto itor = set.cbegin (bucket);
           itor != set.cend (bucket); ++itor) {
         if (need_index) fprintf (out, "stringset[%4zu]: ", bucket);
                    else fprintf (out, "          %4s   ", "");
         need_index = false;
         const string* str = &*itor;
         fprintf (out, "%22zu %p->\"%s\"\n", hash_fn(*str),
                  str, str->c_str());
      }
   }
   fprintf (out, "load_factor = %.3f\n", set.load_factor());
   fprintf (out, "bucket_count = %zu\n", set.bucket_count());
   fprintf (out, "max_bucket_size = %zu\n", max_bucket_size);
}

