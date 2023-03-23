#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../UBX_argparse.h"
#include "ls.h"

int main(int argc, const char **argv)
{
    argparse_option options[] = {
        UBX_ARG_BOOL(-i, --input,help="input file"),
        UBX_ARG_INT(-t, --target, default=1, help="target file"),
        UBX_ARG_STR(-s, --string, UBX_MUST),
        UBX_ARG_GROUP(name=dest, help="destination"),
        UBX_ARG_GROUP(name=src, help="source"),
        UBX_ARG_END()
    };

    argparse parser;
    UBX_argparse_init(&parser, options);
    // argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
    UBX_argparse_parse(&parser, argc, argv);
    
    return 0;
}