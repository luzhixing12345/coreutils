

#include "cat.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../argparse.h"




/**
 * @brief 显示文件内容
 * 
 * @param file_name 
 */
void XBOX_cat(const char *file_name) {
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        char error_info[256 + 5];
        sprintf(error_info, "cat: %s", file_name);
        perror(error_info);
        exit(1);
    }
    char c;
    while ((c = fgetc(fp)) != EOF) {
        printf("%c",c);
    }
    return;
}



int main(int argc, char const *argv[]) {
    char **file = NULL;
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "display this help and exit"]),
        XBOX_ARG_STR_GROUPS(&file, [name = FILE][help = "source"]),
        XBOX_ARG_BOOLEAN(NULL, [-A][--show-all][help = "equivalent to -vET"]),
        XBOX_ARG_BOOLEAN(NULL, [-b][--number-nonblank][help = "number noempty output lines, overrides -n"]),
        XBOX_ARG_BOOLEAN(NULL, [-e][name = e][help = "equivalent to -vE"]),
        XBOX_ARG_BOOLEAN(NULL, [-E][--show-ends][help = "display $ at end of each line"]),
        XBOX_ARG_BOOLEAN(NULL, [-n][--number][help = "number all output lines"]),
        XBOX_ARG_BOOLEAN(NULL, [-s][--squeeze-blank][help = "suppress repeated empty output lines"]),
        XBOX_ARG_BOOLEAN(NULL, [-t][name = t][help = "equivalent to -vT"]),
        XBOX_ARG_BOOLEAN(NULL, [-T][name = bigt][help = "display TAB character as ^I"]),
        XBOX_ARG_BOOLEAN(NULL, [-u][name = u][help = (ignored)]),
        XBOX_ARG_BOOLEAN(NULL, [-v][--show-nonprinting][help = "use ^ and M- notation, except for LFD and TAB"]),
        XBOX_ARG_BOOLEAN(NULL, [--version][help = "output version information and exit"]),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK|XBOX_ARGPARSE_ENABLE_MULTI);
    XBOX_argparse_describe(&parser,
                           "cat",
                           "Concateate FILE(s) to standard output.\n\nWith no FILE, or when FILE is -, read standard "
                           "input.",
                           "\nExamples:\n  cat f - g Output f's contents, then standard input, then g's contents.\n  "
                           "cat       Copy standard input to standard output.\nXBOX coreutils online help: "
                           "<https://github.com/luzhixing12345/xbox>");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        return 0;
    }
    int n = XBOX_ismatch(&parser, "FILE");
        // XBOX_cat(file);
    printf("%d\n",n);
    for (int i=0;i<n;i++) {
        printf("%s\n",file[i]);
        free(file[i]);
    }
    free(file);
    

    if (XBOX_ismatch(&parser, "version")) {
        printf("v0.0.1\n");
    }

    XBOX_free_argparse(&parser);
    return 0;
}
