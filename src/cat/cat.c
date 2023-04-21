

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../argparse.h"

int XBOX_cat(const char *file_name) {
    struct stat file_state;
    int fd = open(file_name, O_RDONLY, 0);
    if (fd < 0) {
        perror("open failed");
        exit(1);
    }
    fstat(fd, &file_state);

    char *bufp = mmap(NULL, file_state.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return write(1, bufp, file_state.st_size);
}

int main(int argc, char const *argv[]) {
    char *file;
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "display this help and exit"]),
        XBOX_ARG_STR_GROUP(&file, [name = FILE][help = "source"]),
        XBOX_ARG_BOOLEAN(NULL,[-A][--show-all][help="equivalent to -vET"]),
        XBOX_ARG_BOOLEAN(NULL,[-b][--number-nonblank][help="number noempty output lines, overrides -n"]),
        XBOX_ARG_BOOLEAN(NULL,[-e][name=e][help="equivalent to -vE"]),\
        XBOX_ARG_BOOLEAN(NULL,[-E][--show-ends][help="display $ at end of each line"]),
        XBOX_ARG_BOOLEAN(NULL,[-n][--number][help="number all output lines"]),
        XBOX_ARG_BOOLEAN(NULL,[-s][--squeeze-blank][help=suppress repeated empty output lines]),
        XBOX_ARG_BOOLEAN(NULL,[-t][name=t][help="equivalent to -vT"]),
        XBOX_ARG_BOOLEAN(NULL,[-T][name=bigt][help="display TAB character as ^I"]),
        XBOX_ARG_BOOLEAN(NULL,[-u][name=u][help=(ignored)]),
        XBOX_ARG_BOOLEAN(NULL,[-v][--show-nonprinting][help="use ^ and M- notation, except for LFD and TAB"]),
        XBOX_ARG_BOOLEAN(NULL,[--version][help="output version information and exit"]),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, 0);
    XBOX_argparse_describe(
        &parser,
        "cat",
        "Concateate FILE(s) to standard output.\n\nWith no FILE, or when FILE is -, read standard input.",
        "\nExamples:\n  cat f - g Output f's contents, then standard input, then g's contents.\n  cat       Copy standard input to standard output.\nXBOX coreutils online help: <https://github.com/luzhixing12345/xbox>");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        return 0;
    }
    if (XBOX_ismatch(&parser, "src")) {
        XBOX_cat(file);
    } else {
        XBOX_argparse_info(&parser);
    }
    XBOX_free_argparse(&parser);
    free(file);
    return 0;
}
