#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "xbox/xargparse.h"
#include "xbox/xbox.h"

// 也是一种方式, 但是不够精准
// #include <utime.h>
// #include <time.h>

// void extend() {
//     struct utimbuf new_times;
//     time_t current_time;
//     time(&current_time);
//     new_times.actime = current_time;                      //设置访问时间为当前时间
//     new_times.modtime = current_time;                     //设置修改时间为当前时间
//     int result = utime("/path/to/file.txt", &new_times);  //修改文件时间戳
// }

char **files = NULL;
int only_access = 0;
int donot_create = 0;

void XBOX_touch(char *file) {
    // 尝试打开文件以检查它是否存在
    // GNU coreutils 的 touch 中的 access time 和 modify time 有时间差, 也就是说没有打开文件?
    struct stat st;
    if (donot_create && stat(file, &st) < 0) {
        return;
    }

    int fd = open(file, O_WRONLY | O_CREAT | O_NOCTTY | O_NONBLOCK, 0644);
    if (fd == -1) {
        perror("open failed");
        return;
    }
    close(fd);

    // 时间相关
    struct timespec current_time[2];
    current_time[0].tv_nsec = UTIME_NOW;                             // access time
    current_time[1].tv_nsec = only_access ? UTIME_OMIT : UTIME_NOW;  // UTIME_OMIT 不变
    int result = utimensat(AT_FDCWD, file, current_time, 0);         //修改文件时间戳
    if (result == -1) {
        perror("Error");
    }
    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, "-h", "--help", "display this help and exit", NULL, "help"),
        XBOX_ARG_BOOLEAN(NULL, "-v", "--version", "output version information and exit", NULL, "version"),
        XBOX_ARG_STRS_GROUP(&files, NULL, NULL, NULL, NULL, NULL),
        XBOX_ARG_BOOLEAN(&only_access, "-a", NULL, "change only the access time", NULL, NULL),
        XBOX_ARG_BOOLEAN(&donot_create, "-c", "--no-create", "do not create any files", NULL, NULL),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(
        &parser, "touch", "Update the access and modification times of each FILE to the current time.", "");
    XBOX_argparse_parse(&parser, argc, argv);

    int n = XBOX_ismatch(&parser, "FILE");

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
    if (n) {
        for (int i = 0; i < n; i++) {
            XBOX_touch(files[i]);
        }
    } else {
        XBOX_argparse_info(&parser);
    }

    XBOX_free_argparse(&parser);
    return 0;
}