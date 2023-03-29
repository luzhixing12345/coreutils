

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../argparse.h"

int UBX_cat(const char *file_name) {
    struct stat file_state;
    int fd = open(file_name, O_RDONLY, 0);
    fstat(fd, &file_state);

    char *bufp = mmap(NULL, file_state.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    return write(1, bufp, file_state.st_size);
}

int main(int argc, char const *argv[]) {
    char *src;
    argparse_option options[] = {UBX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
                                 UBX_ARG_STR_GROUP(&src, [name = src][help = "source"]), UBX_ARG_END()};

    UBX_argparse parser;
    UBX_argparse_init(&parser, options, 0);
    UBX_argparse_describe(&parser, "ls", "\nA brief description of what the program does and how it works.",
                          "\nAdditional description of the program after the description of the arguments.");
    UBX_argparse_parse(&parser, argc, argv);

    if (UBX_ismatch(&parser, "help")) {
        UBX_argparse_info(&parser);
        return 0;
    }
    if (UBX_ismatch(&parser, "src")) {
        UBX_cat(src);
    } else {
        UBX_argparse_info(&parser);
    }
    UBX_free_argparse(&parser);
    free(src);
    return 0;
}
