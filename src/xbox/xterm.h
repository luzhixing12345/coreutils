/*
 *Copyright (c) 2023 All rights reserved
 *@description: 终端显示设置
 *@author: Zhixing Lu
 *@date: 2023-08-27
 *@email: luzhixing12345@163.com
 *@Github: luzhixing12345
 */

#pragma once

#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "xstring.h"
// 终端字体设置

#define XBOX_TERM_FONT_DEFAULT "\033[0m"        // 默认字体
#define XBOX_TERM_FONT_BOLD "\033[1m"           // 高亮前景色
#define XBOX_TERM_FONT_NO_BLOD "\033[22m"       // 取消高亮前景色
#define XBOX_TERM_FONT_UNDERLINE "\033[4m"      // 下划线
#define XBOX_TERM_FONT_NO_UNDERLINE "\033[24m"  // 取消下划线

#define XBOX_TERM_COLOR_BLACK "\033[30m"
#define XBOX_TERM_COLOR_RED "\033[31m"
#define XBOX_TERM_COLOR_GREEN "\033[32m"
#define XBOX_TERM_COLOR_YELLOW "\033[33m"
#define XBOX_TERM_COLOR_BLUE "\033[34m"
#define XBOX_TERM_COLOR_MAGENTA "\033[35m"  // 品红色
#define XBOX_TERM_COLOR_CYAN "\033[36m"
#define XBOX_TERM_COLOR_WHITE "\033[37m"
#define XBOX_TERM_COLOR_EXTEND "\033[38m"  // 前景色扩展
#define XBOX_TERM_COLOR_DEFAULT "\033[39m"

#define DEFUALT_LS_COLORS                                                        \
    "rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;" \
    "33;01:or=40;31;01:mi=00:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st="   \
    "37;44:ex=01;32:"

void XBOX_print_invalid_color_option() {
    printf("Valid arguments are:\n");
    printf("- 'always', 'yes', 'force'\n");
    printf("- 'never', 'no', 'none'\n");
    printf("- 'auto', 'tty', 'if-tty'\n");
}

typedef struct {
    char *font_type;
    char *front_color;  // 前景色
    char *back_color;   // 背景色
    char *front_rgb;
    char *back_rgb;
    char *word;  // 文本信息
} XBOX_term_word;

// 最大长度, 一般来说后缀和虚拟控制序列序号都不会很长, 不想再 malloc/free 了
#define MAX_DC_KV_LENGTH 16

typedef struct {
    char key[MAX_DC_KV_LENGTH];
    char value[MAX_DC_KV_LENGTH];
} dc_kv;

typedef struct {
    char *rs, *di, *ln, *mh, *pi, *so, *door, *bd, *cd;
    char *orp, *mi, *su, *sg, *ca, *tw, *ow, *st, *ex;
    dc_kv *dc_kvs;  // 所有 LS_COLORS 的键值对
    int item_number;
} XBOX_dircolor_database;

/**
 * @brief 终端彩色打印
 *
 * @param word
 * @return char* 返回拼接后的字符串
 */
char *XBOX_colorful_print(XBOX_term_word *word) {
    static char vt_seq[1024];
    int pos = 0;
    char *p;
    if (!word->word) {
        return NULL;
    }
    if (word->font_type) {
        strcpy(vt_seq + pos, word->font_type);
        pos += strlen(word->font_type);
    }
    if (word->front_color) {
        strcpy(vt_seq + pos, word->front_color);
        pos += strlen(word->front_color);
    }
    if (word->back_color) {
        strcpy(vt_seq + pos, word->back_color);
        pos += strlen(word->back_color);
    }
    if (word->front_rgb) {
        strcpy(vt_seq + pos, word->front_rgb);
        pos += strlen(word->front_rgb);
    }
    if (word->back_rgb) {
        strcpy(vt_seq + pos, word->back_rgb);
        pos += strlen(word->back_rgb);
    }
    strcpy(vt_seq + pos, word->word);
    pos += strlen(word->word);
    p = (char *)XBOX_TERM_FONT_DEFAULT;
    strcpy(vt_seq + pos, p);
    pos += strlen(p);
    vt_seq[pos] = 0;
    return vt_seq;
}

int check_default_dc_config(char *name, char default_dc_config[18][3]) {
    for (int i = 0; i < 18; i++) {
        // 简化处理
        // TODO
        if (name[0] == '*') {
            return -1;
        }
        if (!strcmp(name, default_dc_config[i])) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 按 : 分割键值对, 按 = 分割 key value
 *
 * @param lscolors_str
 * @param database
 */
void parse_ls_colors(char *lscolors_str, XBOX_dircolor_database *database) {
    int length = strlen(lscolors_str);
    int item_number = 0;
    for (int i = 0; i < length; i++) {
        if (lscolors_str[i] == ':') {
            item_number++;
        }
    }
    database->dc_kvs = (dc_kv *)malloc(sizeof(dc_kv) * item_number);
    database->item_number = item_number;
    memset(database->dc_kvs, 0, sizeof(dc_kv));
    int p = 0;
    int index = 0;
    char default_dc_config[18][3] = {
        "rs", "di", "ln", "mh", "pi", "so", "do", "bd", "cd", "or", "mi", "su", "sg", "ca", "tw", "ow", "st", "ex"};
    for (int i = 0; i < length; i++) {
        if (lscolors_str[i] == '=') {
            strncpy(database->dc_kvs[index].key, lscolors_str + p, i - p);
            database->dc_kvs[index].key[i - p] = 0;
            p = i + 1;
        } else if (lscolors_str[i] == ':') {
            strncpy(database->dc_kvs[index].value, lscolors_str + p, i - p);
            database->dc_kvs[index].value[i - p] = 0;
            p = i + 1;
            int offset = check_default_dc_config(database->dc_kvs[index].key, default_dc_config);
            if (offset != -1) {
                // 结构体内存地址赋值
                // a little tricky
                char *p = (char *)&(database->dc_kvs[index].value);
                *(u_int64_t *)((char *)database + offset * sizeof(char *)) = (u_int64_t)p;
            }
            index++;
        }
    }
    // for (int i=0;i<index;i++) {
    //     printf("key = %s, value = %s\n", database->dc_kvs[i].key,
    //     database->dc_kvs[i].value);
    // }
    // printf("rs = %s, di = %s\n", database->rs, database->di);
}

/**
 * @brief 初始化 dircolors 的颜色数据库
 *
 * @param database (need free)
 */
void XBOX_init_dc_database(XBOX_dircolor_database **database) {
    char *lscolors_str = getenv("LS_COLORS");
    *database = (XBOX_dircolor_database *)malloc(sizeof(XBOX_dircolor_database));
    if (!lscolors_str) {
        lscolors_str = (char *)DEFUALT_LS_COLORS;
    }
    parse_ls_colors(lscolors_str, *database);
}

void XBOX_free_dc_database(XBOX_dircolor_database *database) {
    free(database->dc_kvs);
    free(database);
}

/**
 * @brief 使用 ASNI 虚拟控制序列终端彩色打印
 *
 * @param word 打印的字
 * @param full_path 全路径
 */
char *XBOX_filename_print(char *file_name, const char *full_path, XBOX_dircolor_database *database) {
    if (!database) {
        // database 为 NULL 说明不显示颜色
        return file_name;
    }
    char *color_code = NULL;
    struct stat fs;
    static char result[PATH_MAX];
    memset(result, 0, PATH_MAX);
    // lstat 查看 link 本身, stat 查看 link 指向的 file
    if (lstat(full_path, &fs) == -1) {
        // 文件缺失
        color_code = database->mi;
    } else {
        switch (fs.st_mode & S_IFMT) {
            case S_IFBLK:  // 块设备文件
                color_code = database->bd;
                break;
            case S_IFCHR:  // 字符设备文件
                color_code = database->cd;
                break;
            case S_IFDIR:  // 目录
                if ((fs.st_mode & S_ISVTX)) {
                    if (fs.st_mode & S_IWOTH) {
                        // 其他用户可以写入目录,但仅允许用户删除自己的文件
                        // chmod +t
                        // chmod o+w
                        color_code = database->tw;
                    } else {
                        // chmod +t
                        color_code = database->st;
                    }
                } else if ((fs.st_mode & S_IWOTH)) {
                    // 其他用户可以写入文件
                    // chmod o+w
                    color_code = database->ow;
                } else {
                    color_code = database->di;
                }
                break;
            case S_IFIFO:  // 管道
                color_code = database->pi;
                break;
            case S_IFLNK:  // 符号链接
                if (stat(full_path, &fs) == -1) {
                    // 孤立的符号链接
                    color_code = database->orp;
                } else {
                    // 正常的符号链接
                    color_code = database->ln;
                }
                break;
            case S_IFREG:  // 普通设备
                if (fs.st_mode & S_IXUSR || fs.st_mode & S_IXGRP || fs.st_mode & S_IXOTH) {
                    color_code = database->ex;
                } else if ((fs.st_mode & S_ISUID)) {
                    // 文件拥有者
                    color_code = database->su;
                } else if (fs.st_mode & S_ISGID) {
                    // 文件组
                    color_code = database->sg;
                } else {
                    // 文件名后缀匹配
                    int dot_pos = XBOX_findChar(file_name, '.', -1);
                    int suffix_match = 0;
                    if (dot_pos != -1) {
                        for (int i = 0; i < database->item_number; i++) {
                            if (strlen(database->dc_kvs[i].key) >= 2 && database->dc_kvs[i].key[0] == '*' &&
                                database->dc_kvs[i].key[1] == '.') {
                                // printf("[%s] [%s]\n", database->dc_kvs[i].key + 1, file_name + dot_pos);
                                if (!strcmp(database->dc_kvs[i].key + 1, file_name + dot_pos)) {
                                    color_code = database->dc_kvs[i].value;
                                    suffix_match = 1;
                                    break;
                                }
                            }
                        }
                    }
                    if (!suffix_match) {
                        color_code = (char *)"0";
                    }
                }
                break;
            case S_IFSOCK:  // 套接字
                color_code = database->so;
                break;
            default:
                // 未知文件类型
                color_code = database->mi;
                break;
        }
        if (!color_code && fs.st_nlink > 1) {
            color_code = database->mh;
        }
    }
    sprintf(result, "\033[%sm%s%s", color_code, file_name, XBOX_TERM_FONT_DEFAULT);
    return result;
}