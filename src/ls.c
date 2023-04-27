#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "argparse.h"

char **dirs;

/**
 * @brief 列出目录下的所有文件(不包含子目录)
 *
 * @param dir_name
 * @param file_names
 * @param self
 */
void XBOX_ls_listfiles(const char *dir_name) {
    DIR *dir_p;
    struct dirent *dp;
    if (!(dir_p = opendir(dir_name))) {
        perror("opendir");
        exit(1);
    }
    while ((dp = readdir(dir_p))) {
        printf("%s ", dp->d_name);
    }
    printf("\n");
    closedir(dir_p);
    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
                                 XBOX_ARG_BOOLEAN(NULL, [-a][--all][help = "show help information"]),
                                 XBOX_ARG_STR_GROUPS(&dirs, [name = src][help = "source"]),
                                 XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK | XBOX_ARGPARSE_ENABLE_MULTI);
    XBOX_argparse_describe(&parser,
                           "ls",
                           "List information about the FILEs (the current directory by default).\nSort entries "
                           "alphabetically if none of -cftuvSUX nor --sort is specified.",
                           "");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }
    int n = XBOX_ismatch(&parser, "src");

    if (n) {
        for (int i = 0; i < n; i++) {
            XBOX_ls_listfiles(dirs[i]);
            free(dirs[i]);
        }
        free(dirs);
    } else {
        XBOX_ls_listfiles(".");
    }
    XBOX_free_argparse(&parser);
    return 0;
}