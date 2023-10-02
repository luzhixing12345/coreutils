
#include <fcntl.h>
#include <grp.h>
#include <linux/limits.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "xbox/xargparse.h"
#include "xbox/xterm.h"
#include "xbox/xutils.h"

// 列间距
#define LS_INTERVAL_SPACE_NUMBER 2
#define LS_INTERVAL_SPACE "  "

static int terminal_width = 0;

char **dirs = NULL;
int all_files = 0;
int almost_all = 0;
int long_list = 0;
int sort_reverse = 0;
char *color = "auto";
XBOX_dircolor_database *dircolor_database = NULL;

// 对齐长度
typedef struct {
    int link;
    int user;
    int group;
    int block_size;
} longlist_align;

static int sort_cmp(const void *p1, const void *p2) {
    XBOX_File **dp1 = (XBOX_File **)p1;
    XBOX_File **dp2 = (XBOX_File **)p2;
    return strcmp((*dp1)->name, (*dp2)->name);
}

/**
 * @brief 根据终端宽度以及目录文件量计算应该取多少行
 *
 * @param dir 目录
 * @param terminal_width 当前的终端宽度
 * @return int
 */
int calculate_row(XBOX_Dir *dir, int terminal_width) {
    int row = 1;
    int total_width = 0;

    int *file_widths = malloc(sizeof(int) * dir->count);
    for (int i = 0; i < dir->count; i++) {
        file_widths[i] = strlen(dir->dp[i]->name);
    }
    // for (int i = 0; i < dir->count; i++) {
    //     printf("file_width[%d]:[%s] = %d\n",i,dir->dp[i]->name,file_widths[i]);
    // }
    for (;; row++) {
        if (row == dir->count) {
            // 到达上限, 直接返回
            return row;
        }
        int index = 0;
        int column_width = 0;
        // printf("terminal_width = %d\n",terminal_width);
        while (index < dir->count) {
            for (int i = 0; i < row; i++) {
                column_width = MAX(column_width, file_widths[index]);
                index++;
                if (index == dir->count) {
                    break;
                }
            }

            // GNU coreutils ls: 最后一列计算有间隔
            // total_width += column_width + LS_INTERVAL_SPACE_NUMBER;
            // 最后一列无间隔
            if (index == dir->count) {
                total_width += column_width;
            } else {
                total_width += column_width + LS_INTERVAL_SPACE_NUMBER;
            }
            column_width = 0;
        }
        if (total_width <= terminal_width) {
            break;
        } else {
            // printf("total width = %d, row = %d\n", total_width, row);
            total_width = 0;
        }
    }
    free(file_widths);
    return row;
}

/**
 * @brief
 *
 * @param dir_name
 */
void ls(const char *dir_name) {
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

    // 对于 stdout 非终端的情况, 依次输出即可
    if (!isatty(1)) {
        for (int i = 0; i < dir->count; i++) {
            if (!dircolor_database) {
                printf("%s\n", dir->dp[i]->name);
            } else {
                printf("%s\n",
                       XBOX_filename_print(
                           dir->dp[i]->name, XBOX_path_join(dir->name, dir->dp[i]->name, NULL), dircolor_database));
            }
        }
        XBOX_free_directory(dir);
        return;
    }

    int row = calculate_row(dir, terminal_width);

    // 对于行数和目录文件数相同的情况, 全部竖排即可, 不需要计算对齐
    if (row == dir->count) {
        for (int i = 0; i < dir->count; i++) {
            if (!dircolor_database) {
                printf("%s\n", dir->dp[i]->name);
            } else {
                printf("%s\n",
                       XBOX_filename_print(
                           dir->dp[i]->name, XBOX_path_join(dir->name, dir->dp[i]->name, NULL), dircolor_database));
            }
        }
        XBOX_free_directory(dir);
        return;
    }

    // 计算每一列的宽度, 用于后续的对齐计算
    // 这里不放在 calculate_row 中计算是想减少内存读写次数, 尽管最后重复计算了一次宽度
    int col_num = (dir->count + row - 1) / row;
    int *ls_col_width = malloc(sizeof(int) * col_num);

    int index = 0;
    int col_index = 0;
    int end_flag = 0;
    while (index < dir->count) {
        int current_column_width = 0;
        for (int i = 0; i < row; i++) {
            int length = strlen(dir->dp[index]->name);
            current_column_width = MAX(current_column_width, length);
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
            // 因为是按行输出, 所以需要考虑优先计算当前行输出的文件名对应的 index
            dp_index = row * j + i;
            if (dp_index >= dir->count) {
                // 最后一列后几个可能为空
                continue;
            }
            // database 空说明不需要颜色
            if (!dircolor_database) {
                printf("%-*s", ls_col_width[j], dir->dp[dp_index]->name);
            } else {
                char *file_name = XBOX_filename_print(dir->dp[dp_index]->name,
                                                      XBOX_path_join(dir->name, dir->dp[dp_index]->name, NULL),
                                                      dircolor_database);
                printf("%-s", file_name);
                // 虚拟控制序列导致字符串长度变化, 需要额外计算剩余空格长度
                int left_space_number = ls_col_width[j] - strlen(dir->dp[dp_index]->name);
                for (int k = 0; k < left_space_number; k++) {
                    printf(" ");
                }
            }
            if (j != col_num - 1) {
                printf(LS_INTERVAL_SPACE);
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

    struct stat fs;

    struct passwd *pwd;
    struct group *grp;
    longlist_align ls_align;
    memset(&ls_align, 0, sizeof(longlist_align));
    for (int i = 0; i < dir->count; i++) {
        if (lstat(XBOX_path_join(dir->name, dir->dp[i]->name, NULL), &fs) < 0) {
            XBOX_free_directory(dir);
            perror("lstat");
            return;
        }
        total_block_number += fs.st_blocks;

        pwd = getpwuid(fs.st_uid);
        grp = getgrgid(fs.st_gid);

        char *user_name, *group_name;
        user_name = !pwd ? "UNKNOWN" : pwd->pw_name;
        group_name = !grp ? "UNKNOWN" : grp->gr_name;

        // 计算对齐所需要的长度
        int link_length = XBOX_number_length(fs.st_nlink);
        ls_align.link = MAX(ls_align.link, link_length);

        int user_length = strlen(user_name);
        ls_align.user = MAX(ls_align.user, user_length);

        int group_length = strlen(group_name);
        ls_align.group = MAX(ls_align.group, group_length);

        int size_length = XBOX_number_length(fs.st_size);
        ls_align.block_size = MAX(ls_align.block_size, size_length);
    }

    printf("total %d\n", total_block_number / 2);

    struct tm *tm_modify;
    time_t current_time;
    time(&current_time);
    // 获取当前时间的年份和月份
    // 这里需要提前计算是因为 localtime 返回的是一个指针
    // 后面需要根据每一个文件执行 tm_modify = localtime(&fs.st_mtime);
    // 此时 tm_current 的值同样会被刷新
    struct tm *tm_current = localtime(&current_time);
    int current_year = tm_current->tm_year;
    int current_month = tm_current->tm_mon;

    char modify_time[20];
    for (int i = 0; i < dir->count; i++) {
        char *full_path = XBOX_path_join(dir->name, dir->dp[i]->name, NULL);
        if (lstat(full_path, &fs) < 0) {
            XBOX_free_directory(dir);
            perror("lstat");
            return;
        }

        char *access_mode = XBOX_stat_access_mode(fs.st_mode);

        pwd = getpwuid(fs.st_uid);
        grp = getgrgid(fs.st_gid);

        char *user_name, *group_name;
        user_name = !pwd ? "UNKNOWN" : pwd->pw_name;
        group_name = !grp ? "UNKNOWN" : grp->gr_name;

        printf("%s %*ld %-*s %-*s %*ld ",
               access_mode,
               ls_align.link,
               fs.st_nlink,
               ls_align.user,
               user_name,
               ls_align.group,
               group_name,
               ls_align.block_size,
               fs.st_size);

        // 格式化时间 TODO: 自由配置
        tm_modify = localtime(&fs.st_mtime);
        int months_difference = (current_year - tm_modify->tm_year) * 12 +
                           (current_month - tm_modify->tm_mon);
        if (months_difference <= 6) {
            strftime(modify_time, sizeof(modify_time), "%b %e %H:%M", tm_modify);
            printf("%s ", modify_time);
        } else {
            strftime(modify_time, sizeof(modify_time), "%b %e", tm_modify);
            printf("%s %5d ", modify_time, tm_modify->tm_year + 1900);
        }

        printf("%s", XBOX_filename_print(dir->dp[i]->name, full_path, dircolor_database));
        struct stat fs;
        if (lstat(full_path, &fs) != -1 && (S_ISLNK(fs.st_mode))) {
            char linkname[PATH_MAX];
            memset(linkname, 0, PATH_MAX);
            if (readlink(full_path, linkname, sizeof(linkname) - 1) == -1) {
                perror("readlink");
                continue;
            }
            printf(" -> %s\n",
                   XBOX_filename_print(linkname, XBOX_path_join(dir->name, linkname, NULL), dircolor_database));
        } else {
            printf("\n");
        }
    }
    XBOX_free_directory(dir);
    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, NULL, "--help", "show help information", NULL, "help"),
        XBOX_ARG_BOOLEAN(&all_files, "-a", "--all", "list all files", NULL, NULL),
        XBOX_ARG_BOOLEAN(&long_list, "-l", "--long-list", "use a long listing format", NULL, NULL),
        XBOX_ARG_BOOLEAN(&almost_all, "-A", NULL, "do not list implied . and ..", NULL, "almost-all"),
        XBOX_ARG_BOOLEAN(NULL, NULL, "--version", "show version", NULL, "version"),
        XBOX_ARG_STR(&color,
                     NULL,
                     "--color",
                     "colorize the output; WHEN can be 'always' (default if omitted), 'auto', or "
                     "'never'",
                     "[=WHEN]",
                     NULL),
        XBOX_ARG_BOOLEAN(&sort_reverse, "-r", "--reverse", "reverse order while sorting", NULL, NULL),
        XBOX_ARG_STRS_GROUP(&dirs, NULL, NULL, "source", NULL, "src"),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(
        &parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK | XBOX_ARGPARSE_ENABLE_EQUAL | XBOX_ARGPARSE_ENABLE_STICK);
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

    if (!strcmp(color, "always")) {
        XBOX_init_dc_database(&dircolor_database);
    } else if (!strcmp(color, "auto")) {
        if (isatty(STDOUT_FILENO)) {
            XBOX_init_dc_database(&dircolor_database);
        }
    } else if (!strcmp(color, "never")) {
        // do not use color
    } else {
        XBOX_print_invalid_color_option();
        XBOX_free_argparse(&parser);
        return 1;
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
    if (dircolor_database) {
        XBOX_free_dc_database(dircolor_database);
    }
    XBOX_free_argparse(&parser);
    return 0;
}