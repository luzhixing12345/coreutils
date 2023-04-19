

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
    char *src;
    argparse_option options[] = {XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
                                 XBOX_ARG_STR_GROUP(&src, [name = src][help = "source"]), XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, 0);
    XBOX_argparse_describe(&parser, "ls", "\nA brief description of what the program does and how it works.",
                          "\nAdditional description of the program after the description of the arguments.");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        return 0;
    }
    if (XBOX_ismatch(&parser, "src")) {
        XBOX_cat(src);
    } else {
        XBOX_argparse_info(&parser);
    }
    XBOX_free_argparse(&parser);
    free(src);
    return 0;
}
