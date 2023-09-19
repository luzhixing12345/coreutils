
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "xbox/xargparse.h"
#include "xbox/xbox.h"

#define XBOX_LS_ALIGN_SPACE 2

static int terminal_width = 0;

char **dirs = NULL;
int all_files = 0;
int almost_all = 0;
int long_list = 0;
char *color = "always";

static int sort_cmp(const void *p1, const void *p2) {
    XBOX_File **dp1 = (XBOX_File **)p1;
    XBOX_File **dp2 = (XBOX_File **)p2;
    return strcmp((*dp1)->name, (*dp2)->name);
}

/**
 * @brief 根据终端宽度以及目录文件量计算应该取多少行
 *
 * @param dir
 * @param col
 * @param terminal_width
 * @return int
 */
int calculaterow(XBOX_Dir *dir, int terminal_width) {
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

/**
 * @brief
 *
 * @param dir_name
 */
void ls(const char *dir_name) {
    int flag = 0;
    printf("color = %s\n", color);
    if (all_files) {
        flag = XBOX_DIR_ALL;
    } else if (almost_all) {
        flag = XBOX_DIR_IGNORE_CURRENT;
    } else {
        flag = XBOX_DIR_IGNORE_HIDDEN;
    }

    XBOX_Dir *dir = XBOX_open_dir(dir_name, flag);
    qsort(dir->dp, dir->count, sizeof(XBOX_File *), sort_cmp);

    // 对于 stdout 非终端的情况, 依次输出即可
    if (!isatty(1)) {
        for (int i = 0; i < dir->count; i++) {
            printf("%s\n", dir->dp[i]->name);
        }
        XBOX_free_directory(dir);
        return;
    }

    int row = calculaterow(dir, terminal_width);
    if (row <= 0) {
        XBOX_free_directory(dir);
        printf("error: [%s]\n", dir_name);
        return;
    }
    // printf("col = %d\n", row);

    // 计算每一列的宽度
    int col_num = (dir->count + row - 1) / row;
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
                XBOX_file_print(dir->dp[dp_index]->name, XBOX_path_join(dir->name, dir->dp[dp_index]->name, NULL));
            printf("%-s", file_name);
            int left_space_number = ls_col_width[j] - strlen(dir->dp[dp_index]->name);
            for (int k = 0; k < left_space_number; k++) {
                printf(" ");
            }
            if (j != col_num - 1) {
                printf("  ");
            }
        }
        printf("\n");
    }

    free(ls_col_width);
    XBOX_free_directory(dir);
    return;
}

void ls_longlist(const char *dir_name) {
    int flag = 0;
    if (all_files) {
        flag = XBOX_DIR_ALL;
    } else if (almost_all) {
        flag = XBOX_DIR_IGNORE_CURRENT;
    } else {
        flag = XBOX_DIR_IGNORE_HIDDEN;
    }

    XBOX_Dir *dir = XBOX_open_dir(dir_name, flag);
    qsort(dir->dp, dir->count, sizeof(XBOX_File *), sort_cmp);
    int total_block_number = 0;
    int max_block_size_length = 0;
    int max_link_length = 0;
    struct stat file_stat;
    for (int i = 0; i < dir->count; i++) {
        if (stat(XBOX_path_join(dir->name, dir->dp[i]->name, NULL), &file_stat) < 0) {
            XBOX_free_directory(dir);
            perror("stat");
            return;
        }
        total_block_number += file_stat.st_blocks;
        int size_length = XBOX_number_length(file_stat.st_size);
        int link_length = XBOX_number_length(file_stat.st_nlink);
        max_block_size_length = MAX(max_block_size_length, size_length);
        max_link_length = MAX(max_link_length, link_length);
    }
    printf("total %d\n", total_block_number / 2);
    struct passwd *pwd;
    struct group *grp;

    for (int i = 0; i < dir->count; i++) {
        if (stat(XBOX_path_join(dir->name, dir->dp[i]->name, NULL), &file_stat) < 0) {
            XBOX_free_directory(dir);
            perror("stat");
            return;
        }
        char *st_mode_rwx = XBOX_stat_access_mode(file_stat.st_mode);
        pwd = getpwuid(file_stat.st_uid);
        grp = getgrgid(file_stat.st_gid);

        struct tm *tm;
        char modify_time[20];
        // 格式化时间

        tm = localtime(&file_stat.st_mtime);
        strftime(modify_time, sizeof(modify_time), "%b %e %H:%M", tm);

        printf("%s %*ld %s %s %*ld %s %s\n",
               st_mode_rwx,
               max_link_length,
               file_stat.st_nlink,
               (pwd == NULL) ? "UNKNOWN" : pwd->pw_name,
               (grp == NULL) ? "UNKNOWN" : grp->gr_name,
               max_block_size_length,
               file_stat.st_size,
               modify_time,
               XBOX_file_print(dir->dp[i]->name, XBOX_path_join(dir->name, dir->dp[i]->name, NULL)));
    }
    XBOX_free_directory(dir);
    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, "-h","--help", "show help information", NULL, NULL),
        XBOX_ARG_BOOLEAN(&all_files, "-a", "--all", "list all files", NULL, NULL),
        XBOX_ARG_BOOLEAN(&long_list, "-l", "long-list", "use a long listing format", NULL, NULL),
        XBOX_ARG_BOOLEAN(&almost_all, "-A", NULL, "do not list implied . and ..", NULL, "almost-all"),
        XBOX_ARG_BOOLEAN(NULL, NULL, "--version", "show version", NULL, NULL),
        XBOX_ARG_STR(&color,NULL,
                     "--color","colorize the output; WHEN can be 'always' (default if omitted), 'auto', or "
                                      "'never'; more info below","[=WHEN]", NULL),
        XBOX_ARG_STRS_GROUP(&dirs, NULL, NULL, "source", NULL, "src"),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK|XBOX_ARGPARSE_ENABLE_EQUAL|XBOX_ARGPARSE_ENABLE_STICK);
    XBOX_argparse_describe(&parser,
                           "ls",
                           "List information about the FILEs (the current directory by default).\nSort entries "
                           "alphabetically if none of -cftuvSUX nor --sort is specified.",
                           "Using color to distinguish file types is disabled both by default and\n"
                           "with --color=never.  With --color=auto, ls emits color codes only when\n"
                           "standard output is connected to a terminal.  The LS_COLORS environment\n"
                           "variable can change the settings.  Use the dircolors command to set it.\n");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }
    if (XBOX_ismatch(&parser, "version")) {
        printf("%s\n", XBOX_VERSION);
        XBOX_free_argparse(&parser);
        return 0;
    }
    int n = XBOX_ismatch(&parser, "src");

    // 对于 stdout 为终端窗口的, 获取终端宽度用于后续计算
    if (isatty(STDOUT_FILENO)) {
        struct winsize terminal_size;

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_size) < 0) {
            perror("ioctl");
            XBOX_free_argparse(&parser);
            exit(1);
        }
        terminal_width = terminal_size.ws_col;
    }

    if (n) {
        for (int i = 0; i < n; i++) {
            if (long_list) {
                ls_longlist(dirs[i]);
            } else {
                ls(dirs[i]);
            }
        }
    } else {
        if (long_list) {
            ls_longlist(".");
        } else {
            ls(".");
        }
    }
    XBOX_free_argparse(&parser);
    return 0;
}