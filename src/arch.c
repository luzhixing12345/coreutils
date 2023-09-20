
#include <sys/utsname.h>

#include "xbox/xargparse.h"

void XBOX_arch() {
    struct utsname unameData;
    if (uname(&unameData) == -1) {
        printf("Error: Unable to get system information.\n");
        return;
    }
    printf("%s\n", unameData.machine);
}

int main(int argc, const char **argv) {
    argparse_option options[] = {XBOX_ARG_BOOLEAN(NULL, "-h", "--help", "display this help and exit", NULL, "help"),
                                 XBOX_ARG_BOOLEAN(NULL, "-v", "--version", "output version information and exit", NULL, "version"),
                                 XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(&parser,
                           "stat",
                           "Display file or file system status.",
                           "XBOX coreutils online help: <https://github.com/luzhixing12345/xbox>");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
    }
    if (XBOX_ismatch(&parser, "version")) {
        printf("%s\n", XBOX_VERSION);
    }
    XBOX_arch();

    XBOX_free_argparse(&parser);
    return 0;
}