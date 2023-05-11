
#include <dirent.h>
#include <stdlib.h>
#include "xargparse.h"
#include "xutils.h"

static const char *VERSION = "v0.0.1";

char **directories = NULL;
int all_files = 0;
int directory_only = 0;
int current_directory_only = 0;

static const char *file_end = "└";
static const char *file_item = "──";
static const char *file_mid = "├";
// static const char *file_fill = "│";

static int sort_cmp(const void *p1, const void *p2) {
    struct dirent **dp1 = (struct dirent**)p1;
    struct dirent **dp2 = (struct dirent**)p2;
    return strcmp((*dp1)->d_name, (*dp2)->d_name);
}

void sort_files(XBOX_Dir *dir) {
    qsort(dir->dp, dir->count,sizeof(struct dirent*), sort_cmp);
}

void XBOX_tree(XBOX_Dir *dir) {
    sort_files(dir);
    int dir_count = dir->d_count;
    int file_count = dir->f_count;
    printf("%s%s%s\n", XBOX_ANSI_COLOR_BLUE,dir->d_name, XBOX_ANSI_COLOR_RESET);
    for (int i = 0; i < dir->count; i++) {
        if (!all_files && dir->dp[i]->d_name[0] == '.') {
            if (dir->dp[i]->d_type == DT_DIR) dir_count--;
            else file_count--;
            continue;
        }
        printf("%s%s",i==dir->count-1?file_end:file_mid,file_item);
        XBOX_colorprint_dir(dir->dp[i]);
        printf("\n");
    }
    printf("\n%d directories, %d files\n", dir_count, file_count);
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
        XBOX_ARG_INT(NULL, [-v][--version][help = "show version"]),
        XBOX_ARG_BOOLEAN(&all_files, [-a][name = "all_files"][help = "All files are listed."]),
        XBOX_ARG_BOOLEAN(&directory_only, [-d][name = "only directory"][help = "List directories only."]),
        XBOX_ARG_BOOLEAN(&current_directory_only,
                         [-x][name = "current-dir"][help = "Stay on current filesystem only."]),
        XBOX_ARG_STR_GROUPS(&directories, [name = directory]),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(&parser, "tree", "list the directory in tree format", "");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }

    if (XBOX_ismatch(&parser, "version")) {
        printf("%s\n", VERSION);
    }

    int n = XBOX_ismatch(&parser, "directory");

    if (n) {
        for (int i = 0; i < n; i++) {
            XBOX_Dir *directory = XBOX_open_dir(directories[i]);
            XBOX_tree(directory);
            XBOX_free_directory(directory);
        }
    } else {
        char *dir_name = ".";
        XBOX_Dir *directory = XBOX_open_dir(dir_name);
        if (!directory) {
            XBOX_free_args(directories, n);
            XBOX_free_argparse(&parser);
            return 1;
        }
        XBOX_tree(directory);
        XBOX_free_directory(directory);
    }

    XBOX_free_args(directories, n);
    XBOX_free_argparse(&parser);
    return 0;
}