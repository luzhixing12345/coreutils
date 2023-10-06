#include <unistd.h>

#include "xbox/xargparse.h"
#include "xbox/xterm.h"
#include "xbox/xutils.h"

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
XBOX_dircolor_database *dircolor_database = NULL;

static const char *file_end = "└";    // \u2514
static const char *file_item = "──";  // \u2500\u2500
static const char *file_mid = "├";    // \u251C
static const char *file_fill = "│";   // \u2502

static int sort_cmp(const void *p1, const void *p2) {
    XBOX_File **dp1 = (XBOX_File **)p1;
    XBOX_File **dp2 = (XBOX_File **)p2;
    if (reverse_sort) {
        return strcmp((*dp2)->name, (*dp1)->name);
    } else {
        return strcmp((*dp1)->name, (*dp2)->name);
    }
}

void tree(XBOX_Dir *dir) {
    if (!unsort) {
        qsort(dir->dp, dir->count, sizeof(XBOX_File *), sort_cmp);
    }
    // 找到最后一个元素, 作为 tree 的结尾
    int last_index = -1;
    if (!directory_only) {
        if (all_files) {
            last_index = dir->count - 1;
        } else {
            // 找到第一个不是 . 开头的文件
            for (int i = dir->count - 1; i >= 0; i--) {
                if (dir->dp[i]->name[0] != '.') {
                    last_index = i;
                    break;
                }
            }
        }
    } else {
        for (int i = dir->count - 1; i >= 0; i--) {
            if (XBOX_IS_DIR(dir->dp[i])) {
                if (all_files) {
                    last_index = i;
                    break;
                } else {
                    if (dir->dp[i]->name[0] != '.') {
                        last_index = i;
                        break;
                    }
                }
            }
        }
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
        printf("%s\n", XBOX_filename_print(dir->name, dir->name, dircolor_database));
    }
    // 深度 -L
    if (level > 0 && depth >= level) {
        XBOX_freedir(dir);
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
            for (int j = 0; j < depth; j++) {
                if (!position[j]) {
                    printf("%s   ", file_fill);
                } else {
                    printf("    ");
                }
            }
            printf("%s%s ", i == last_index ? file_end : file_mid, file_item);
            printf("%s\n",
                   XBOX_filename_print(XBOX_get_last_path(dir->dp[i]->name), dir->dp[i]->name, dircolor_database));

            if (current_directory_only) {
                continue;
            }
            char *sub_dir_name = XBOX_path_join(dir->name, dir->dp[i]->name, NULL);
            XBOX_Dir *sub_dir = XBOX_opendir(sub_dir_name, XBOX_DIR_IGNORE_CURRENT);
            sub_dir->parent = dir;
            sub_dir->is_last = i == last_index;
            tree(sub_dir);
        } else {
            if (directory_only) {
                continue;
            }
            for (int j = 0; j < depth; j++) {
                if (!position[j]) {
                    printf("%s   ", file_fill);
                } else {
                    printf("    ");
                }
            }
            printf("%s%s ", i == last_index ? file_end : file_mid, file_item);
            printf("%s",
                   XBOX_filename_print(
                       dir->dp[i]->name, XBOX_path_join(dir->name, dir->dp[i]->name, NULL), dircolor_database));

            printf("\n");
        }
    }
    XBOX_freedir(dir);
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(&all_files, "-a", NULL, "All files are listed.", NULL, NULL),
        XBOX_ARG_BOOLEAN(&directory_only, "-d", NULL, "List directories only.", NULL, NULL),
        XBOX_ARG_BOOLEAN(&current_directory_only, "-x", NULL, "Stay on current filesystem only.", NULL, NULL),
        XBOX_ARG_STRS_GROUP(&directories, NULL, NULL, NULL, NULL, "directory"),
        XBOX_ARG_BOOLEAN(&no_color, "-n", NULL, "Turn colorization off always (-C overrides).", NULL, NULL),
        XBOX_ARG_BOOLEAN(NULL, "-C", NULL, "Turn colorization on always.", NULL, "has-color"),
        XBOX_ARG_BOOLEAN(&unsort, "-U", NULL, "Leave files unsorted.", NULL, "unsort"),
        XBOX_ARG_BOOLEAN(&reverse_sort, "-r", NULL, "Reverse the order of the sort.", NULL, "reverse"),
        XBOX_ARG_INT(&level, "-L", NULL, "Descend only level directories deep.", NULL, "level"),
        XBOX_ARG_BOOLEAN(&full_name, "-f", NULL, "Print the full path prefix for each file.", NULL, NULL),
        XBOX_ARG_BOOLEAN(NULL, "-h", "--help", "display this help and exit", NULL, "help"),
        XBOX_ARG_BOOLEAN(NULL, "-v", "--version", "output version information and exit", NULL, "version"),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(
        &parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK | XBOX_ARGPARSE_ENABLE_EQUAL | XBOX_ARGPARSE_ENABLE_STICK);
    XBOX_argparse_describe(&parser, "tree", "list the directory in tree format", "");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }

    if (XBOX_ismatch(&parser, "version")) {
        printf("%s\n", XBOX_VERSION);
    }
    if (XBOX_ismatch(&parser, "has-color")) {
        XBOX_init_dc_database(&dircolor_database);
    } else {
        if (!no_color && isatty(STDOUT_FILENO)) {
            XBOX_init_dc_database(&dircolor_database);
        }
    }

    int n = XBOX_ismatch(&parser, "directory");

    if (XBOX_ismatch(&parser, "level")) {
        if (level <= 0) {
            printf("tree: Invalid level, must be greater than 0.\n");
            XBOX_free_argparse(&parser);
            return 1;
        }
    }

    if (n) {
        for (int i = 0; i < n; i++) {
            XBOX_Dir *directory = XBOX_opendir(directories[i], XBOX_DIR_IGNORE_CURRENT);
            directory->parent = NULL;
            tree(directory);
            printf("\n%d directories", dir_count);
            if (!directory_only) {
                printf(", %d files", file_count);
            }
            printf("\n");
        }
    } else {
        char *dir_name = ".";
        XBOX_Dir *directory = XBOX_opendir(dir_name, XBOX_DIR_IGNORE_CURRENT);
        directory->parent = NULL;
        tree(directory);
        printf("\n%d directories", dir_count);
        if (!directory_only) {
            printf(", %d files", file_count);
        }
        printf("\n");
    }
    if (dircolor_database) {
        XBOX_free_dc_database(dircolor_database);
    }
    XBOX_free_argparse(&parser);
    return 0;
}