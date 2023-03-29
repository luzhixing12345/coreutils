
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../argparse.h"

char *stat_file_type(struct stat *statbuf) {
    switch (statbuf->st_mode & S_IFMT) {
        case S_IFBLK:
            return "block device";
        case S_IFCHR:
            return "character device";
        case S_IFDIR:
            return "directory";
        case S_IFIFO:
            return "FIFO/pipe";
        case S_IFLNK:
            return "symlink";
        case S_IFREG:
            return "regular file";
        case S_IFSOCK:
            return "socket";
        default:
            return "unknown?";
    }
}

void UBX_stat(const char *name) {
    struct stat statbuf;
    stat(name, &statbuf);
    printf("  File: %s\n", name);
    printf(
        "  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
        "Device: %llxh/%llud\tInode: %-10llu  Links: %-5lu\n",
        (unsigned long long)statbuf.st_size,
        (unsigned long long)statbuf.st_blocks,
        (unsigned long)statbuf.st_blksize,
        stat_file_type(&statbuf),
        (unsigned long long)statbuf.st_dev,
        (unsigned long long)statbuf.st_dev,
        (unsigned long long)statbuf.st_ino,
        (unsigned long)statbuf.st_nlink);
}

int main(int argc, const char **argv) {
    char *src;
    argparse_option options[] = {
        UBX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
        UBX_ARG_STR_GROUP(&src, [name = src][help = "source"]),
        UBX_ARG_END()};

    UBX_argparse parser;
    UBX_argparse_init(&parser, options, UBX_ARGPARSE_ENABLE_EQUAL);
    UBX_argparse_describe(&parser, "ls", "", "");
    UBX_argparse_parse(&parser, argc, argv);

    if (UBX_ismatch(&parser, "help")) {
        UBX_argparse_info(&parser);
        return 0;
    }
    if (!UBX_ismatch(&parser, "src")) {
        UBX_argparse_info(&parser);
        return 0;
    }
    UBX_stat(src);
    free(src);
    UBX_free_argparse(&parser);
    return 0;
}