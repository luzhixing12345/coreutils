#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "xargparse.h"
#include "xutils.h"

#define XBOX_LS_ALIGN_SPACE 2

static int terminal_width = 0;

char **dirs = NULL;
int all_files = 0;
int long_list = 0;

static int sort_cmp(const void *p1, const void *p2) {
    XBOX_File **dp1 = (XBOX_File **)p1;
    XBOX_File **dp2 = (XBOX_File **)p2;
    return strcmp((*dp1)->name, (*dp2)->name);
}

/**
 * @brief 计算应该取多少行
 *
 * @param dir
 * @param col
 * @param terminal_width
 * @return int
 */
int XBOX_ls_calculaterow(XBOX_Dir *dir, int terminal_width) {
    int row = 1;
    int total_width = 0;

    int *file_widths = malloc(sizeof(int) * dir->count);
    for (int i = 0; i < dir->count; i++) {
        file_widths[i] = strlen(dir->dp[i]->name);
    }
    // for (int i = 0; i < dir->count; i++) {
    //     printf("file_width[%d]:[%s] = %d\n",i,dir->dp[i]->name,file_widths[i]);
    // }

    int end_flag = 0;

    for (;; row++) {
        int index = 0;
        int column_width = 0;
        // printf("terminal_width = %d\n",terminal_width);
        while (index < dir->count) {
            for (int i = 0; i < row; i++) {
                column_width = MAX(column_width, file_widths[index]);
                index++;
                if (index >= dir->count) {
                    total_width += column_width;
                    if (total_width <= terminal_width) {
                        end_flag = 1;
                    }
                    total_width = 0;
                    break;
                }
            }
            total_width += column_width + XBOX_LS_ALIGN_SPACE;
            column_width = 0;
            if (total_width > terminal_width) {
                total_width = 0;
                break;
            }
        }

        if (!end_flag) {
            continue;
        }
        if (total_width <= terminal_width) {
            end_flag = 1;
            break;
        }
    }
    free(file_widths);
    if (end_flag) {
        return row;
    } else {
        return -1;
    }
}

void XBOX_ls(const char *dir_name) {
    XBOX_Dir *dir = XBOX_open_dir(dir_name, all_files ? XBOX_DIR_ALL : XBOX_DIR_IGNORE_HIDDEN);
    qsort(dir->dp, dir->count, sizeof(struct dirent *), sort_cmp);
    int row = XBOX_ls_calculaterow(dir, terminal_width);
    if (row <= 0) {
        XBOX_free_directory(dir);
        printf("error: [%s]\n", dir_name);
        return;
    }
    // printf("col = %d\n", row);


    // 计算每一列的宽度
    int col_num = (dir->count+row-1) / row;
    int *ls_col_width = malloc(sizeof(int) * col_num);

    int index = 0;
    int col_index = 0;
    int end_flag = 0;
    while (index < dir->count) {
        int current_column_width = 0;
        for (int i = 0; i < row; i++) {
            current_column_width = MAX(current_column_width, strlen(dir->dp[index]->name));
            index++;
            if (index >= dir->count) {
                ls_col_width[col_index] = current_column_width;
                end_flag = 1;
                break;
            }
        }
        if (end_flag) {
            break;
        }
        ls_col_width[col_index++] = current_column_width;
    }

    // for (int i=0;i<col_num;i++) {
    //     printf("[%d] = [%d]\n",i,ls_col_width[i]);
    // }


    int dp_index = 0;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col_num; j++) {
            dp_index = row * j + i;
            if (dp_index >= dir->count) {
                continue;
            }
            char *file_name =
                XBOX_colorprint(dir->dp[dp_index]->name, XBOX_path_join(dir->name, dir->dp[dp_index]->name, NULL));
            printf("%-s", file_name);
            int left_space_number = ls_col_width[j] - strlen(dir->dp[dp_index]->name);
            for (int k=0;k<left_space_number;k++) {
                printf(" ");
            }
            if (j != col_num-1) {
                printf("  ");
            }
        }
        printf("\n");
    }

    free(ls_col_width);
    XBOX_free_directory(dir);
    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
        XBOX_ARG_BOOLEAN(&all_files, [-a][--all][help = "show help information"]),
        XBOX_ARG_BOOLEAN(&long_list, [-l][name = "long-list"][help = "use a long listing format"]),
        XBOX_ARG_STR_GROUPS(&dirs, [name = src][help = "source"]),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(&parser,
                           "ls",
                           "List information about the FILEs (the current directory by default).\nSort entries "
                           "alphabetically if none of -cftuvSUX nor --sort is specified.",
                           "");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
    }
    int n = XBOX_ismatch(&parser, "src");

    // 获取终端宽度用于后续计算
    struct winsize terminal_size;
    if (!isatty(1)) {
        printf("1 is not a tty\n");
        XBOX_free_args(dirs, n);
        XBOX_free_argparse(&parser);
        exit(1);
    }
    if (ioctl(1, TIOCGWINSZ, &terminal_size) < 0) {
        perror("ioctl");
        XBOX_free_args(dirs, n);
        XBOX_free_argparse(&parser);
        exit(1);
    }
    terminal_width = terminal_size.ws_col;

    if (n) {
        for (int i = 0; i < n; i++) {
            XBOX_ls(dirs[i]);
        }
    } else {
        XBOX_ls(".");
    }
    XBOX_free_args(dirs, n);
    XBOX_free_argparse(&parser);
    return 0;
}