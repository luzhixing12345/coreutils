
#ifndef XBOX_XUTILS_H
#define XBOX_XUTILS_H

#include <dirent.h>
#include <linux/limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define XBOX_LOG(fmt, ...) printf("[%s]:[%4d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define XBOX_TYPE(t) #t
#define XBOX_NAME(name) _##name

#define XBOX_ANSI_COLOR_RED "\033[1;91m"
#define XBOX_ANSI_COLOR_GREEN "\033[1;92m"
#define XBOX_ANSI_COLOR_YELLOW "\033[1;93m"
#define XBOX_ANSI_COLOR_BLUE "\033[1;94m"
#define XBOX_ANSI_COLOR_MAGENTA "\033[1;95m"
#define XBOX_ANSI_COLOR_CYAN "\033[1;96m"
#define XBOX_ANSI_COLOR_RESET "\033[1;0m"

typedef struct XBOX_File {
    unsigned char type;
    char name[256];
} XBOX_File;

typedef struct XBOX_Dir {
    struct XBOX_Dir* parent;  // 父目录
    int is_last;              // 是否是父目录的最后一个
    char name[256];
    int count;    // 目录+文件数量
    int d_count;  // 目录数量
    int f_count;  // 文件数量
    XBOX_File** dp;
} XBOX_Dir;

/**
 * @brief 打开一个目录并读取其中所有的文件和目录
 *
 * @param path
 * @return XBOX_Dir* (需要释放)
 */
XBOX_Dir* XBOX_open_dir(const char* path) {
    DIR* dir;
    struct dirent* entry;

    int length;
    dir = opendir(path);
    if (dir == NULL) {
        static char error_info[1024];
        memset(error_info, 0, 1024);
        sprintf(error_info, "open failed %s", path);
        perror(error_info);
        exit(1);
    }
    XBOX_Dir* directory = (XBOX_Dir*)malloc(sizeof(XBOX_Dir));
    memset(directory, 0, sizeof(XBOX_Dir));
    while ((entry = readdir(dir)) != NULL) {
        directory->count++;
        if (entry->d_type == DT_DIR) {
            // 目录
            directory->d_count++;
        } else {
            directory->f_count++;
        }
    }
    rewinddir(dir);
    directory->dp = (XBOX_File**)malloc(sizeof(XBOX_File*) * directory->count);
    for (int i = 0; i < directory->count; i++) {
        directory->dp[i] = (XBOX_File*)malloc(sizeof(XBOX_File));
    }
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        length = strlen(entry->d_name);
        strncpy(directory->dp[i]->name, entry->d_name, length);
        directory->dp[i]->name[length] = 0;
        directory->dp[i]->type = entry->d_type;
        i++;
    }
    length = strlen(path);
    strncpy(directory->name, path, length);
    closedir(dir);
    return directory;
}

void XBOX_free_directory(XBOX_Dir* directory) {
    directory->parent = NULL;
    for (int i = 0; i < directory->count; i++) {
        free(directory->dp[i]);
    }
    free(directory->dp);
    free(directory);
}

/**
 * @brief 连接路径, 可变参数, 最后一个参数传 NULL
 *
 * @param path
 * @param ...
 * @return char*
 */
char* XBOX_path_join(const char* path, ...) {
    static char result[PATH_MAX];
    memset(result, 0, PATH_MAX);
    va_list args;
    char* arg;
    size_t len = strlen(path);
    strncpy(result, path, len);
    va_start(args, path);

    while ((arg = va_arg(args, char*))) {
        size_t arg_len = strlen(arg);
        if (len + arg_len + 1 > PATH_MAX) {
            fprintf(stderr, "Error: maximum path length exceeded\n");
            return NULL;
        }
        strcat(result, "/");
        len += arg_len + 1;
        strncat(result, arg, arg_len);
    }
    va_end(args);
    return result;
}

/**
 * @brief 获取路径的最后一个目录
 *
 * @param path
 * @return const char*
 */
const char* XBOX_get_last_path(const char* path) {
    const char* p = strrchr(path, '/');
    if (p == NULL) {
        // 如果路径中没有斜杠，则返回整个路径
        return path;
    } else {
        // 如果路径中有斜杠，则返回最后一个斜杠后面的部分
        return p + 1;
    }
}

int is_image(const char* name) {
    const char* ext = strrchr(name, '.');
    if (ext == NULL) {
        return 0;
    }
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0 || strcasecmp(ext, ".png") == 0 ||
        strcasecmp(ext, ".bmp") == 0 || strcasecmp(ext, ".svg") == 0 || strcasecmp(ext, ".gif") == 0) {
        return 1;
    }
    return 0;
}

void XBOX_colorprint(const char* name) {
    char* color_code = NULL;
    struct stat file_stat;
    if (stat(name, &file_stat) == -1) {
        // error occurred while getting file status
        color_code = XBOX_ANSI_COLOR_RESET;  // set the color to default
        printf("%s%s%s", color_code, XBOX_get_last_path(name), XBOX_ANSI_COLOR_RESET);
        return;
    }
    if (S_ISREG(file_stat.st_mode)) {
        // regular file
        if (file_stat.st_mode & S_IXUSR || file_stat.st_mode & S_IXGRP || file_stat.st_mode & S_IXOTH) {
            // file has execute permission
            color_code = XBOX_ANSI_COLOR_GREEN;  // set the color to green
        } else if (is_image(name)) {
            // image file
            color_code = XBOX_ANSI_COLOR_MAGENTA;  // set the color to magenta
        } else {
            color_code = XBOX_ANSI_COLOR_RESET;  // set the color to default
        }
    } else if (S_ISDIR(file_stat.st_mode)) {
        // directory
        color_code = XBOX_ANSI_COLOR_BLUE;
    } else if (S_ISLNK(file_stat.st_mode)) {
        // symbolic link
        color_code = XBOX_ANSI_COLOR_CYAN;
    } else if (S_ISFIFO(file_stat.st_mode)) {
        // named pipe (FIFO)
        color_code = XBOX_ANSI_COLOR_MAGENTA;
    } else if (S_ISSOCK(file_stat.st_mode)) {
        // local-domain socket
        color_code = XBOX_ANSI_COLOR_YELLOW;
    } else {
        // unknown file type
        color_code = XBOX_ANSI_COLOR_RESET;
    }
    printf("%s%s%s", color_code, XBOX_get_last_path(name), XBOX_ANSI_COLOR_RESET);
}

#endif  // XBOX_XUTILS_H
