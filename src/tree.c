
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xargparse.h"
#include "xutils.h"



static const char *VERSION = "v0.0.1";

static char **directories = NULL;
static int all_files = 0;
static int directory_only = 0;
static int current_directory_only = 0;
static int no_color = 0;
static int level = 0;
static int full_name = 0;
static int unsort = 0;
static int reverse_sort = 0;
static int dir_count = 0;
static int file_count = 0;

static const char *file_end = "└";    // \u2514
static const char *file_item = "──";  // \u2500\u2500
static const char *file_mid = "├";    // \u251C
static const char *file_fill = "│";   // \u2502

static int sort_cmp(const void *p1, const void *p2) {
    XBOX_File **dp1 = (XBOX_File **)p1;
    XBOX_File **dp2 = (XBOX_File **)p2;
    if (reverse_sort) {
        // 对于不展示隐藏文件的情况把这些文件置顶
        if (!all_files) {
            if ((*dp1)->name[0] == '.')
                return 0;
            if ((*dp2)->name[0] == '.')
                return 1;
        }
        return strcmp((*dp2)->name, (*dp1)->name);
    } else {
        return strcmp((*dp1)->name, (*dp2)->name);
    }
}

void XBOX_tree(XBOX_Dir *dir) {
    if (!unsort) {
        qsort(dir->dp, dir->count, sizeof(XBOX_File *), sort_cmp);
    }

    int depth = 0;  // 目录深度
    XBOX_Dir *temp = dir;
    while (temp->parent) {
        temp = temp->parent;
        depth++;
    }
    char position[100];  // 记录每一层对应的是不是最后一个元素
    if (depth) {
        temp = dir;
        int i = 0;
        while (temp->parent) {
            position[depth - i - 1] = temp->is_last;
            i++;
            temp = temp->parent;
        }
    } else {
        printf("%s\n",XBOX_colorprint(dir->name, dir->name));
    }
    // 深度 -L
    if (level > 0 && depth >= level) {
        XBOX_free_directory(dir);
        return;
    }

    dir_count += dir->d_count;  // 不计算 . 和 ..
    file_count += dir->f_count;

    for (int i = 0; i < dir->count; i++) {
        // printf("[%d/%d]:[%s] = %s\n", i, dir->count, dir->name, dir->dp[i]->name);
        // . 开头默认隐藏
        if (dir->dp[i]->name[0] == '.' && !all_files) {
            if (XBOX_IS_DIR(dir->dp[i])) {
                dir_count--;
            } else {
                file_count--;
            }
            if (!all_files) {
                continue;
            }
        }
        if (XBOX_IS_DIR(dir->dp[i])) {
            for (int i = 0; i < depth; i++) {
                if (!position[i]) {
                    printf("%s   ", file_fill);
                } else {
                    printf("    ");
                }
            }
            printf("%s%s ", i == dir->count - 1 ? file_end : file_mid, file_item);
            if (no_color) {
                printf("%s\n", XBOX_get_last_path(dir->name));
            } else {
                printf("%s%s%s\n", XBOX_ANSI_COLOR_BLUE, XBOX_get_last_path(dir->dp[i]->name), XBOX_ANSI_COLOR_RESET);
            }

            if (current_directory_only) {
                continue;
            }
            char *sub_dir_name = XBOX_path_join(dir->name, dir->dp[i]->name, NULL);
            XBOX_Dir *sub_dir = XBOX_open_dir(sub_dir_name, XBOX_DIR_IGNORE_CURRENT);
            sub_dir->parent = dir;
            sub_dir->is_last = i == dir->count - 1;
            XBOX_tree(sub_dir);
        } else {
            if (directory_only) {
                continue;
            }
            for (int i = 0; i < depth; i++) {
                if (!position[i]) {
                    printf("%s   ", file_fill);
                } else {
                    printf("    ");
                }
            }
            printf("%s%s ", i == dir->count - 1 ? file_end : file_mid, file_item);
            if (no_color) {
                printf("%s", dir->dp[i]->name);
            } else {
                printf("%s",XBOX_colorprint(dir->dp[i]->name, XBOX_path_join(dir->name, dir->dp[i]->name, NULL)));
            }
            printf("\n");
        }
    }
    XBOX_free_directory(dir);
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
        XBOX_ARG_BOOLEAN(NULL, [-v][--version][help = "show version"]),
        XBOX_ARG_BOOLEAN(&all_files, [-a][name = "all_files"][help = "All files are listed."]),
        XBOX_ARG_BOOLEAN(&directory_only, [-d][name = "only directory"][help = "List directories only."]),
        XBOX_ARG_BOOLEAN(&current_directory_only,
                         [-x][name = "current-dir"][help = "Stay on current filesystem only."]),
        XBOX_ARG_STR_GROUPS(&directories, [name = directory]),
        XBOX_ARG_BOOLEAN(&no_color, [-n][name = "no-color"][help = "Turn colorization off always (-C overrides)."]),
        XBOX_ARG_BOOLEAN(NULL, [-C][name = "has-color"][help = "Turn colorization on always."]),
        XBOX_ARG_BOOLEAN(&unsort, [-U][name = "unsort"][help = "Leave files unsorted."]),
        XBOX_ARG_BOOLEAN(&reverse_sort, [-r][name = "reverse"][help = "Reverse the order of the sort."]),
        XBOX_ARG_INT(&level, [-L][name = "level"][help = "Descend only level directories deep."]),
        XBOX_ARG_BOOLEAN(&full_name, [-f][name="full-name"][help="Print the full path prefix for each file."]),
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
    if (XBOX_ismatch(&parser, "has-color")) {
        no_color = 0;
    }

    int n = XBOX_ismatch(&parser, "directory");

    if (XBOX_ismatch(&parser, "level")) {
        if (level <= 0) {
            printf("tree: Invalid level, must be greater than 0.\n");
            XBOX_free_args(directories, n);
            XBOX_free_argparse(&parser);
            return 1;
        }
    }

    if (n) {
        for (int i = 0; i < n; i++) {
            XBOX_Dir *directory = XBOX_open_dir(directories[i],XBOX_DIR_IGNORE_CURRENT);
            directory->parent = NULL;
            XBOX_tree(directory);
            printf("\n%d directories", dir_count);
            if (!directory_only) {
                printf(", %d files", file_count);
            }
            printf("\n");
        }
    } else {
        char *dir_name = ".";
        XBOX_Dir *directory = XBOX_open_dir(dir_name,XBOX_DIR_IGNORE_CURRENT);
        directory->parent = NULL;
        XBOX_tree(directory);
        printf("\n%d directories", dir_count);
        if (!directory_only) {
            printf(", %d files", file_count);
        }
        printf("\n");
    }

    XBOX_free_args(directories, n);
    XBOX_free_argparse(&parser);
    return 0;
}