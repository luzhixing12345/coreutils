
#ifndef XBOX_XUTILS_H
#define XBOX_XUTILS_H

#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define XBOX_LOG(fmt, ...) printf("[%s]:[%4d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define XBOX_TYPE(t) #t
#define XBOX_NAME(name) _##name

#define XBOX_ANSI_COLOR_RED "\x1b[91m"
#define XBOX_ANSI_COLOR_GREEN "\x1b[92m"
#define XBOX_ANSI_COLOR_YELLOW "\x1b[93m"
#define XBOX_ANSI_COLOR_BLUE "\x1b[94m"
#define XBOX_ANSI_COLOR_MAGENTA "\x1b[95m"
#define XBOX_ANSI_COLOR_CYAN "\x1b[96m"
#define XBOX_ANSI_COLOR_RESET "\x1b[0m"

typedef struct {
    char d_name[256];
    int count;    // 目录+文件数量
    int d_count;  // 目录数量
    int f_count;  // 文件数量
    struct dirent** dp;
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
    dir = opendir(path);
    if (dir == NULL) {
        perror("open failed");
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
    directory->dp = (struct dirent**)malloc(sizeof(struct dirent*) * directory->count);
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        directory->dp[i++] = entry;
    }
    strcpy(directory->d_name, path);
    closedir(dir);
    return directory;
}

void XBOX_free_directory(XBOX_Dir* directory) {
    free(directory->dp);
    free(directory);
}

void XBOX_colorprint_dir(struct dirent* entry) {
    char* color_code = NULL;
    switch (entry->d_type) {
        case DT_REG:  // regular file
            struct stat file_stat;
            if (stat(entry->d_name, &file_stat) == -1) {
                // error occurred while getting file status
                color_code = XBOX_ANSI_COLOR_RESET; // set the color to default
                break;
            }
            if (file_stat.st_mode & S_IXUSR || file_stat.st_mode & S_IXGRP || file_stat.st_mode & S_IXOTH) {
                // file has execute permission
                color_code = XBOX_ANSI_COLOR_GREEN; // set the color to green
            } else {
                color_code = XBOX_ANSI_COLOR_RESET; // set the color to default
            }
            break;
        case DT_DIR:  // directory
            color_code = XBOX_ANSI_COLOR_BLUE;
            break;
        case DT_LNK:  // symbolic link
            color_code = XBOX_ANSI_COLOR_CYAN;
            break;
        case DT_FIFO:  // named pipe (FIFO)
            color_code = XBOX_ANSI_COLOR_MAGENTA;
            break;
        case DT_SOCK:  // local-domain socket
            color_code = XBOX_ANSI_COLOR_YELLOW;
            break;
        default:  // unknown file type
            color_code = XBOX_ANSI_COLOR_RESET;
            break;
    }
    printf("%s%s%s", color_code, entry->d_name, XBOX_ANSI_COLOR_RESET);
}

#endif  // XBOX_XUTILS_H
