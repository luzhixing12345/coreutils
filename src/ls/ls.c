#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../argparse.h"

/**
 * @brief 列出目录下的所有文件(不包含子目录)
 *
 * @param dir_name
 * @param file_names
 * @param self
 */
void XBOX_ls_listfiles(const char *dir_name, char ***file_names, int self) {
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
}

int main(int argc, const char **argv) {
    char *src;
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
        XBOX_ARG_BOOLEAN(NULL, [-a][--all][help = "show help information"]),
        XBOX_ARG_STR_GROUP(&src, [name = src][help = "source"]),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_EQUAL);
    XBOX_argparse_describe(&parser, "ls", "", "");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        return 0;
    }
    if (!XBOX_ismatch(&parser, "src")) {
        src = malloc(sizeof(char) * 2);
        strcpy(src, ".");
    }

    if (XBOX_ismatch(&parser, "all")) {
    }

    free(src);
    XBOX_free_argparse(&parser);
    return 0;
}