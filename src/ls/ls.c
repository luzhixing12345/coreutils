#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../UBX_argparse.h"
#include "ls.h"

int main(int argc, const char **argv)
{
    int a;
    int b;
    char *c, *d, *e;
    argparse_option options[] = {
        UBX_ARG_INT(&a, [-i][--input][help="input file"]),
        UBX_ARG_INT(&b,[-t][--target][default=1][help="target file"]),
        UBX_ARG_STR(&c,[-s][--string][flag=UBX_MUST]),
        UBX_ARG_STR_GROUP(&d,[name=dest][help="destination"]),
        UBX_ARG_STR_GROUP(&e,[name=src][help="source"]),
        UBX_ARG_END()
    }; 

    argparse parser;
    UBX_argparse_init(&parser, options, 0);
    UBX_argparse_describe(&parser, "ls", "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
    UBX_argparse_parse(&parser, argc, argv);
    
    printf("a = %d\n",a);
    printf("c = %s\n",c);
    printf("src = %s\n",e);
    free(c);
    free(d);
    free(e);
    return 0;
}