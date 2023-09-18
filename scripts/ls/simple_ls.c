
/*
    列出当前目录下所有的文件名
    Usage:

    gcc simple_ls.c -o ls
    ./ls
*/

#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char** argv) {
    DIR* dir;
    struct dirent* entry;

    char* path = ".";  // 目录路径
    dir = opendir(path);

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
    return 0;
}
