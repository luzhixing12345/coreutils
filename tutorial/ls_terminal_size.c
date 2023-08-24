/*
 *Copyright (c) 2023 All rights reserved
 *@description: 控制终端窗口 size 的变化
 *@author: Zhixing Lu
 *@date: 2023-08-24
 *@email: luzhixing12345@163.com
 *@Github: luzhixing12345
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, const char **argv) {
    struct winsize ws;

    // 获取当前终端窗口的大小
    // TIOCGWINSZ -> get
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }

    printf("当前终端窗口大小:行数:%d,列数:%d\n", ws.ws_row, ws.ws_col);

    // 设置新的终端窗口大小
    if (argc == 2) {
        printf("变为原先两倍 => 还原\n");
        ws.ws_col = ws.ws_col * 2;
    } else {
        ws.ws_col = ws.ws_col / 2;
    }

    // TIOCSWINSZ -> set
    if (ioctl(STDOUT_FILENO, TIOCSWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }

    printf("已将终端窗口大小设置为:行数:%d,列数:%d\n", ws.ws_row, ws.ws_col);

    return 0;
}
