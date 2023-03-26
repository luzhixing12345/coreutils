#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../argparse.h"

int main(int argc, const char **argv)
{
    int i;
    int t;
    char *s, *dest;
    int src;
    argparse_option options[] = {
        UBX_ARG_BOOLEAN(NULL, [-h][--help][help="show help information"]),
        UBX_ARG_BOOLEAN(NULL, [-p][--hel][help="show help information"]),
        UBX_ARG_INT(&i, [-i][--input][help="input file"]),
        UBX_ARG_INT(&t,[-t][--target][help="target file"]),
        UBX_ARG_STR(&s,[-s][--string]),
        UBX_ARG_STR_GROUP(&dest,[name=dest][help="destination"]),
        UBX_ARG_INT_GROUP(&src,[name=src][help="source"]),
        UBX_ARG_END()
    }; 

    UBX_argparse parser;
    UBX_argparse_init(&parser, options, UBX_ARGPARSE_ENABLE_EQUAL);
    UBX_argparse_describe(&parser, "ls", "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
    UBX_argparse_parse(&parser, argc, argv);

    if (UBX_ismatch(&parser, "help")) {
        UBX_argparse_info(&parser);
    }
    if (UBX_ismatch(&parser, "input")) {
        printf("i = %d\n",i);
    }
    if (UBX_ismatch(&parser, "target")) {
        printf("t = %d\n",t);
    }
    if (UBX_ismatch(&parser, "string")) {
        printf("s = %s\n",s);
    }
    if (UBX_ismatch(&parser, "dest")) {
        printf("dest = %s\n",dest);
    }
    if (UBX_ismatch(&parser, "src")) {
        printf("src = %d\n",src);
    }
    UBX_free_argparse(&parser);
    
    free(s);
    free(dest);
    return 0;
}