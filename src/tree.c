
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xargparse.h"
#include "xutils.h"

#define IS_DIR(dp) (dp->type == DT_DIR)

static const char *VERSION = "v0.0.1";

char **directories = NULL;
int all_files = 0;
int directory_only = 0;
int current_directory_only = 0;

int dir_count = 0;
int file_count = 0;

static const char *file_end = "└"; // \u2514
static const char *file_item = "──"; // \u2500\u2500
static const char *file_mid = "├"; // \u251C
static const char *file_fill = "│"; // \u2502


static int sort_cmp(const void *p1, const void *p2) {
    XBOX_File **dp1 = (XBOX_File **)p1;
    XBOX_File **dp2 = (XBOX_File **)p2;
    return strcmp((*dp1)->name, (*dp2)->name);
}

void XBOX_tree(XBOX_Dir *dir) {
    qsort(dir->dp, dir->count, sizeof(struct dirent *), sort_cmp);

    dir_count += dir->d_count;
    file_count += dir->f_count;

    int depth = 0;  // 目录深度
    XBOX_Dir *temp;
    temp = dir;
    while (temp->parent) {
        temp = temp->parent;
        depth++;
    }
    char position[100];  // 记录每一层对应的是不是最后一个元素
    memset(position, 0, 100);
    if (depth) {
        temp = dir;
        int i = 0;
        while (temp->parent) {
            position[depth - i - 1] = temp->is_last;
            i++;
            temp = temp->parent;
        }
    }

    if (depth) {
        for (int i = 0; i < depth - 1; i++) {
            if (!position[i]) {
                printf("%s   ", file_fill);
            } else {
                printf("    ");
            }
        }
        printf("%s%s ", position[depth-1]?file_end:file_mid, file_item);
    }

    printf("%s%s%s\n", XBOX_ANSI_COLOR_BLUE, XBOX_get_last_path(dir->name), XBOX_ANSI_COLOR_RESET);
    for (int i = 0; i < dir->count; i++) {
        // printf("[%d/%d]:[%s] = %s\n", i, dir->count, dir->d_name, dir->dp[i]->name);
        if (!all_files && dir->dp[i]->name[0] == '.') {
            if (IS_DIR(dir->dp[i]))
                dir_count--;
            else
                file_count--;
            continue;
        }
        if (IS_DIR(dir->dp[i])) {
            if ((!strcmp(".", dir->dp[i]->name) || !strcmp("..", dir->dp[i]->name))) {
                continue;
            } else {
                char *sub_dir_name = XBOX_path_join(dir->name, dir->dp[i]->name, NULL);
                XBOX_Dir *sub_dir = XBOX_open_dir(sub_dir_name);
                sub_dir->parent = dir;
                sub_dir->is_last = i == dir->count - 1;
                XBOX_tree(sub_dir);
            }
        } else {
            if (depth) {
                for (int i = 0; i < depth; i++) {
                    if (!position[i]) {
                        printf("%s   ", file_fill);
                    } else {
                        printf("    ");
                    }
                }
            }
            printf("%s%s ", i == dir->count - 1 ? file_end : file_mid, file_item);
            XBOX_colorprint(XBOX_path_join(dir->name, dir->dp[i]->name, NULL));
            printf("\n");
        }
    }
    XBOX_free_directory(dir);
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
            directory->parent = NULL;
            XBOX_tree(directory);
            printf("\n%d directories, %d files\n", dir_count, file_count);
        }
    } else {
        char *dir_name = ".";
        XBOX_Dir *directory = XBOX_open_dir(dir_name);
        directory->parent = NULL;
        XBOX_tree(directory);
        printf("\n%d directories, %d files\n", dir_count, file_count);
    }

    XBOX_free_args(directories, n);
    XBOX_free_argparse(&parser);
    return 0;
}