// Matthew Tan
// mxtan
// cmps104a
// Wesley Mackey
// asg1: main.cpp to represent file
// in a stringset format
// functions chomp() and cpplines()
// were written by Professor Mackey

#include <iostream>
#include <string>
#include <vector>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "auxlib.h"
#include "stringset.h"

using namespace std;

string_set strSet;
string CPP = "/usr/bin/cpp -nostdinc";
string cat = "/usr/bin/cat";
constexpr size_t LINESIZE = 1024;
int yy_flex_debug = 0;
int yydebug = 0;

// Chomp the last character from a buffer if it is delim.
void chomp(char* string, char delim) 
{
    size_t len = strlen(string);
    if (len == 0) 
    {
        return;
    }
    char* nlpos = string + len - 1;
    if (*nlpos == delim) 
    {
        *nlpos = '\0';
    }
}

// Run cpp against the lines of the file.
void cpplines(FILE* pipe, const char* filename) 
{
    int linenr = 1;
    char inputname[LINESIZE];
    strcpy (inputname, filename);
    for (; ;) 
    {
        char buffer[LINESIZE];
        char* fgets_rc = fgets(buffer, LINESIZE, pipe);
        if (fgets_rc == NULL) 
        {
            break;
        }
        chomp(buffer, '\n');
        //printf("\n------------------------------------------\n");
        //printf("in cpplines():before sscanf_rc filename: 
        //%s, linenum: %d, buffer: %s\n", 
        //     filename, linenr, buffer);
        int sscanf_rc = sscanf(buffer, "# %d \"%[^\"]\"", 
               &linenr, inputname);
        //printf("in cpplines(): after sscanf_rc: sscanf_rc is: 
        //%d, buffer: %s, linenr: %d, inputname: %s\n"
        //           , sscanf_rc, buffer, linenr, inputname);
        if (sscanf_rc == 2) 
        {
            //printf("DIRECTIVE: line %d file \"%s\"\n", 
            //        linenr, inputname);
            continue;
        }
        char* savepos = NULL;
        char* bufptr = buffer;
        for (int tokenct = 1; ; tokenct++) 
        {
            char* token = strtok_r(bufptr, " \t\n", &savepos);
            bufptr = NULL;
            if (token == NULL) 
            {
                break;
            }
            //printf("token %d.%d: [%s]\n", linenr, tokenct, token);
            strSet.intern_stringset(token);
        }
        linenr++;
    }
}

void usage()
{
    fprintf(stderr, 
        "Usage: oc [-ly] [-D_OCLIB_OH] [-@flags] [filename.oc]\n");
}

int processFlags(int argc, char* argv[])
{
    int optionChar;
    while ((optionChar = getopt(argc, argv, "ly@D:")) != -1)
    {
        //printf("in procesFlags: optionChar is: %d\n", optionChar);
        switch(optionChar)
        {
            case 'l': yy_flex_debug = 1;
                break;
            case 'y': yydebug = 1;
                break;
            case '@': set_debugflags(optarg);
                break;
            case 'D': CPP = CPP + " " + optarg;
                break;
            default:
                usage();
                fprintf(stderr, "illegal option (%c)\n", optopt);
                break;
        }
    }
    //printf("in processFlags: 
    //after loop: optionChar: %d\n", optionChar);
    optind = argc;
    return optind;
}

int main(int argc, char* argv[]) 
{
    const char* execname = basename(argv[0]);
    int exit_status = EXIT_SUCCESS;
    int countFlags = processFlags(argc, argv);
    //printf("in main: countFlags is: %d\n", countFlags);
    for (int argi = countFlags - 1; argi < argc; argi++) 
    {
        char* filename = argv[argi];
        string outFileName = filename;
        if (outFileName.substr(
               outFileName.find_last_of(".") + 1) != "oc")
        {
            usage();
            continue;
        }
        //printf("in main: filename is: %s\n", filename);
        string command = CPP + " " + filename;
        //string command = cat + " " + filename;           
        //printf ("command=\"%s\"\n", command.c_str());
        //printf("\n--------------------------------\n");
        //printf("begin CPP: \n");
        //system(command.c_str());
        //printf("end CPP: \n");
        //printf("\n---------------------------------\n");
        FILE* pipe = popen(command.c_str(), "r");
        string newFileName = outFileName.substr(0, 
                             outFileName.find_last_of('.')) + ".str";
        FILE* out = fopen(newFileName.c_str(), "w");
        if (pipe == NULL) 
        {
            exit_status = EXIT_FAILURE;
            fprintf(stderr, "%s: %s: %s\n",
                    execname, command.c_str(), strerror (errno));
        }
        else 
        {
            cpplines(pipe, filename);
            int pclose_rc = pclose(pipe);
            eprint_status(command.c_str(), pclose_rc);
            if (pclose_rc != 0) 
            {
                exit_status = EXIT_FAILURE;
            }
        }
        strSet.dump_stringset(out);
        fclose(out);
    }
    return exit_status;
}

