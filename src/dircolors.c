
#include <stdlib.h>

#include "xbox/xargparse.h"
#include "xbox/xterm.h"

int print_database = 0;



void dircolors() {
    char *ls_colors = getenv("LS_COLORS");
    if (print_database) {
        
    } else {
        printf("LS_COLORS='%s;'\n", ls_colors);
        printf("export LS_COLORS\n");
    }
    
    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
                                 XBOX_ARG_BOOLEAN(NULL, [--version][help = "show version"]),
                                 XBOX_ARG_BOOLEAN(&print_database, [-p]["--print-database"][help = "output defaults"]),
                                 XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(&parser, "dircolors", "","");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }

    dircolors();

    XBOX_free_argparse(&parser);
}