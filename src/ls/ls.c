#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../argparse.h"

int main(int argc, const char **argv)
{
    argparse_option options[] = {
        BST_ARG_BOOL(-i,--input,help="input file"),
        BST_ARG_INT(-t,--target, default=1, help="target file"),
        BST_ARG_STR(-s, --string, BST_MUST),
        BST_ARG_GROUP(dest, help="destination"),
        BST_ARG_GROUP(src, help="source"),
        BST_ARG_END()
    };

    argparse parser;
    BST_argparse_init(&parser, options);
    // argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
    BST_argparse_parse(&parser, argc, argv);

    return 0;
}