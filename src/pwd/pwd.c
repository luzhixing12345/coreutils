

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../argparse.h"

int main(int argc, const char **argv)
{
    char **defines = NULL;
    int *integers = NULL;
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help="show help information"]),
        XBOX_ARG_INT(&integers, [-i][--input][help="input file"]),
        XBOX_ARG_STR(&defines,[-D][--define][help="defination macros"]),
        XBOX_ARG_END()
    }; 

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_MULTI|XBOX_ARGPARSE_ENABLE_STICK);
    XBOX_argparse_describe(&parser,"pwd","","");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
    }
    
    int d = XBOX_ismatch(&parser, "define");
    printf("define = ");
    for (int i=0;i<d;i++) {
        printf("%s ", defines[i]);
        free(defines[i]);
    }
    free(defines);
    printf("\n");

    int k = XBOX_ismatch(&parser, "input");
    printf("integer = ");
    for (int i=0;i<k;i++) {
        printf("%d ", integers[i]);
    }
    printf("\n");
    free(integers);
    XBOX_free_argparse(&parser);

        
    return 0;
}