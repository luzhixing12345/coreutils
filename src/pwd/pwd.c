

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../argparse.h"

static int logical = 1; // default
static int physical = 0;

void XBOX_pwd() {
    if (logical & !physical) {
        char *pwd = getenv("PWD");
        if (pwd == NULL) {
            printf("PWD environment variable is not set\n");
        } else {
            printf("%s\n", pwd);
        }
        return;
    }

    char buf[PATH_MAX];
    char *cwd = getcwd(buf, PATH_MAX);
    if (cwd == NULL) {
        perror("getcwd");
        exit(1);
    }
    printf("%s\n", buf);
    return;
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