

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../UBX_argparse.h"

int main(int argc, const char **argv)
{
    char **defines = NULL;
    int *integers = NULL;
    argparse_option options[] = {
        UBX_ARG_BOOLEAN(NULL, [-h][--help][help="show help information"]),
        UBX_ARG_INT(&integers, [-i][--input][help="input file"]),
        UBX_ARG_STR(&defines,[-D][--define][help="defination macros"]),
        UBX_ARG_END()
    }; 

    argparse parser;
    UBX_argparse_init(&parser, options, UBX_ARGPARSE_ENABLE_MULTI);
    UBX_argparse_parse(&parser, argc, argv);

    if (UBX_ismatch(&parser, "help")) {
        UBX_argparse_info(&parser);
    }
    
    int d = UBX_ismatch(&parser, "define");
    printf("define = ");
    for (int i=0;i<d;i++) {
        printf("%s ", defines[i]);
        free(defines[i]);
    }
    free(defines);
    printf("\n");

    int k = UBX_ismatch(&parser, "input");
    printf("integer = ");
    for (int i=0;i<k;i++) {
        printf("%d ", integers[i]);
    }
    printf("\n");
    free(integers);
    UBX_free_argparse(&parser);

        
    return 0;
}