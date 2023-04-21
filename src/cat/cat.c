

#include "cat.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../argparse.h"

// 全局变量
static int show_end = 0;
static int show_number = 0;
static int show_tab = 0;
static int line_number = 1;
static int nonblank = 0;
static int squeeze = 0;
char **files = NULL;


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
    char c = fgetc(fp);
    if (c == EOF) {
        fclose(fp);
        return;
    }
    if (nonblank && c == '\n') {
        line_number--;
    } else if (show_number) {
        printf("%6d",line_number);
    }
    char old_c;
    while (c != EOF) {

        // 一些cat的参数
        if (show_tab && c == '\t') {
            printf("^I");
        }
        old_c = c;
        c = fgetc(fp);
        if (squeeze && old_c == '\n' && c == '\n') {
            char next_c = fgetc(fp);
            if (next_c != '\n' && line_number) {
                if (show_end) {
                    printf("$");
                }
                printf("%c", old_c);
            }
            fseek(fp, -1, SEEK_CUR);
        } else {
            if (show_end && old_c == '\n') {
                printf("$");
            }
            printf("%c", old_c);
        }
        if (nonblank && old_c == '\n' && c != EOF) {
            if (c != old_c) {
                line_number++;
                printf("%6d\t",line_number);
            }
            continue;
        }
        if (show_number && old_c == '\n' && c != EOF) {
            line_number++;
            printf("%6d\t",line_number);
        }
        
    }
    fclose(fp);
    return;
}



int main(int argc, char const *argv[]) {
    
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "display this help and exit"]),
        XBOX_ARG_STR_GROUPS(&files, [name = FILE][help = "source"]),
        XBOX_ARG_BOOLEAN(NULL, [-A][--show-all][help = "equivalent to -vET"]),
        XBOX_ARG_BOOLEAN(&nonblank, [-b][--number-nonblank][help = "number noempty output lines, overrides -n"]),
        XBOX_ARG_BOOLEAN(NULL, [-e][name = e][help = "equivalent to -vE"]),
        XBOX_ARG_BOOLEAN(&show_end, [-E][--show-ends][help = "display $ at end of each line"]),
        XBOX_ARG_BOOLEAN(&show_number, [-n][--number][help = "number all output lines"]),
        XBOX_ARG_BOOLEAN(&squeeze, [-s][--squeeze-blank][help = "suppress repeated empty output lines"]),
        XBOX_ARG_BOOLEAN(NULL, [-t][name = t][help = "equivalent to -vT"]),
        XBOX_ARG_BOOLEAN(&show_tab, [-T][name = bigt][help = "display TAB character as ^I"]),
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
                           "cat       Copy standard input to standard output.\n\nXBOX coreutils online help: "
                           "<https://github.com/luzhixing12345/xbox>");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }
    if (XBOX_ismatch(&parser, "version")) {
        printf("cat (XBOX coreutils) 0.0.1\n");
        XBOX_free_argparse(&parser);
        return 0;
    }
    int n = XBOX_ismatch(&parser, "FILE");
    if (n) {
        // printf("%d\n",n);
        for (int i=0;i<n;i++) {
            XBOX_cat(files[i]);
            free(files[i]);
        }
        free(files);
    } else {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }
    XBOX_free_argparse(&parser);
    return 0;
}
