
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "xbox/xargparse.h"

// 全局变量
static int show_end = 0;
static int show_number = 0;
static int show_tab = 0;
static int nonblank = 0;
static int squeeze = 0;
static int line_number = 1;
char **files = NULL;

/**
 * @brief 显示文件内容
 *
 * @param file_name
 */
void XBOX_cat(const char *file_name) {
    struct stat st;
    stat(file_name, &st);
    if (S_ISDIR(st.st_mode)) {
        printf("cat: %s: Is a directory\n", file_name);
        return;
    }

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
        printf("%6d\t", line_number);
    }

    char old_c;
    int is_start = 1;
    while (c != EOF) {
        old_c = c;
        c = fgetc(fp);

        // 一些cat的参数的实现
        if (squeeze && old_c == '\n' && c == '\n') {
            // 如果是在开头
            char next_c = fgetc(fp);
            fseek(fp, -1, SEEK_CUR);

            if (is_start) {
                continue;
            } else {
                if (next_c == EOF) {
                    if (show_end) {
                        printf("$\n$\n");
                    } else {
                        printf("\n\n");
                    }
                    break;
                } else if (next_c != '\n') {
                    if (show_end) {
                        printf("$");
                    }
                    printf("\n");
                }
            }
        } else {
            if (show_end && old_c == '\n') {
                printf("$");
            }
            if (show_tab && old_c == '\t') {
                printf("^I");
                continue;
            }
            if (c != '\n') {
                is_start = 0;
            }
            printf("%c", old_c);
        }
        if (nonblank && old_c == '\n' && c != EOF) {
            if (c != old_c) {
                line_number++;
                printf("%6d\t", line_number);
            }
            continue;
        }
        if (show_number && old_c == '\n' && c != EOF) {
            line_number++;
            printf("%6d\t", line_number);
        }
    }
    fclose(fp);
    return;
}

int main(int argc, char const *argv[]) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, "-h", "--help", "display this help and exit", NULL, "help"),
        XBOX_ARG_BOOLEAN(NULL, NULL, "--version", "output version information and exit", NULL, "version"),
        XBOX_ARG_STRS_GROUP(&files, NULL, NULL, "source", NULL, "FILE"),
        XBOX_ARG_BOOLEAN(NULL, "-A", "--show-all", "equivalent to -vET", NULL, "show-all"),
        XBOX_ARG_BOOLEAN(&nonblank, "-b", "--number-nonblank", "number noempty output lines, overrides -n", NULL, NULL),
        XBOX_ARG_BOOLEAN(&show_end, "-e", NULL, "equivalent to -vE", NULL, NULL),
        XBOX_ARG_BOOLEAN(&show_end, "-E", "--show-ends", "display $ at end of each line", NULL, NULL),
        XBOX_ARG_BOOLEAN(&show_number, "-n", "--number", "number all output lines", NULL, NULL),
        XBOX_ARG_BOOLEAN(&squeeze, "-s", "--squeeze-blank", "suppress repeated empty output lines", NULL, NULL),
        XBOX_ARG_BOOLEAN(&show_tab, "-t", NULL, "equivalent to -vT", NULL, NULL),
        XBOX_ARG_BOOLEAN(&show_tab, "-T", NULL, "display TAB character as ^I", NULL, NULL),
        XBOX_ARG_BOOLEAN(NULL, "-u", NULL, NULL, NULL, NULL),
        XBOX_ARG_BOOLEAN(NULL, "-v", "--show-nonprinting", "use ^ and M- notation, except for LFD and TAB", NULL, NULL),

        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(&parser,
                           "cat",
                           "Concateate FILE(s) to standard output.\n\nWith no FILE, or when FILE is -, read standard "
                           "input.",
                           "Examples:\n  cat f - g Output f's contents, then standard input, then g's contents.\n  "
                           "cat       Copy standard input to standard output.\n\nXBOX coreutils online help: "
                           "<https://github.com/luzhixing12345/xbox>");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
    }
    if (XBOX_ismatch(&parser, "version")) {
        printf("%s\n", XBOX_VERSION);
    }

    if (XBOX_ismatch(&parser, "show-all")) {
        show_end = 1;
        show_tab = 1;
    }

    if (nonblank) {
        show_number = 0;
    }

    int n = XBOX_ismatch(&parser, "FILE");
    if (n) {
        // printf("%d\n",n);
        for (int i = 0; i < n; i++) {
            XBOX_cat(files[i]);
        }
    }
    XBOX_free_argparse(&parser);
    return 0;
}
