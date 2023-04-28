

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "xutils.h"

static int logical = 1;  // default
static int physical = 0;

// [?]: 如果在一个软链接中修改PWD的话和pwd结果不同, 和 busybox 相同

#define SAME_INODE(st1, st2) ((st1.st_ino == st2.st_ino) && (st1.st_dev == st2.st_dev))

static char *logical_getcwd(void) {
    struct stat st1;
    struct stat st2;
    char *wd = getenv("PWD");
    char *p;

    /* Textual validation first.  */
    if (!wd || wd[0] != '/')
        return NULL;
    p = wd;
    while ((p = strstr(p, "/."))) {
        if (!p[2] || p[2] == '/' || (p[2] == '.' && (!p[3] || p[3] == '/')))
            return NULL;
        p++;
    }
    /* System call validation.  */
    if (stat(wd, &st1) == 0 && stat(".", &st2) == 0 && SAME_INODE(st1, st2))
        return wd;
    return NULL;
}

char *xgetcwd(void) {
    char *cwd = getcwd(NULL, 0);
    if (!cwd && errno == ENOMEM) {
        perror("getcwd");
        exit(0);
    }
    return cwd;
}

void XBOX_pwd() {
    char *pwd;
    if (logical & !physical) {
        pwd = logical_getcwd();
        if (pwd) {
            printf("%s\n", pwd);
            return;
        }
    }

    pwd = xgetcwd();
    if (pwd) {
        printf("%s\n", pwd);
        free(pwd);
        return;
    }
    // https://github.com/MaiZure/coreutils-8.3/blob/master/src/pwd.c
    // else {
    //     struct file_name *file_name = file_name_init();
    //     robust_getcwd(file_name);
    //     puts(file_name->start);
    //     file_name_free(file_name);
    // }
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
        XBOX_ARG_INT(NULL, [-v][--version][help = "show version"]),
        XBOX_ARG_BOOLEAN(&logical,
                         [-L][--logical][help = "print the value of $PWD if it names the current working directory"]),
        XBOX_ARG_BOOLEAN(&physical,
                         [-P][--physical][help = "print the physical directory, without any symbolic links"]),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_MULTI | XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(&parser, "pwd", "Print the name of the current working directory", "");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }

    if (XBOX_ismatch(&parser, "version")) {
        printf("v0.0.1\n");
        XBOX_free_argparse(&parser);
        return 0;
    }
    XBOX_pwd();

    XBOX_free_argparse(&parser);
    return 0;
}