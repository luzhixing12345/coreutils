
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../argparse.h"

char **dirs;
static int dereference = 0;
static int file_system = 0;
char *cached;
static int terse = 0;

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
            return "UNKNOWN";
    }
}

/**
 * @brief 返回文件的[读/写/执行]权限的字符串(需要free)
 *
 * @param mode
 * @return char*
 */
char *XBOX_stat_access_mode(mode_t mode) {
    char *buf = malloc(sizeof(char) * 11);
    strcpy(buf, "-rwxrwxrwx");
    if (S_ISREG(mode)) {
        buf[0] = '-';
    } else if (S_ISDIR(mode)) {
        buf[0] = 'd';
    } else if (S_ISCHR(mode)) {
        buf[0] = 'c';
    } else if (S_ISBLK(mode)) {
        buf[0] = 'b';
    } else if (S_ISFIFO(mode)) {
        buf[0] = 'p';
    } else if (S_ISLNK(mode)) {
        buf[0] = 'l';
        return buf;
    } else if (S_ISSOCK(mode)) {
        buf[0] = 's';
    } else {
        buf[0] = '-';
        // UNKNOWN type?
    }
    mode_t umask_val = umask(0);                // 获取当前 umask 值
    umask(umask_val);                           // 恢复原来的 umask 值
    mode_t effective_mode = mode & ~umask_val;  // 计算生效的权限值
    buf[1] = (effective_mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (effective_mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (effective_mode & S_IXUSR) ? 'x' : '-';
    buf[4] = (effective_mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (effective_mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (effective_mode & S_IXGRP) ? 'x' : '-';
    buf[7] = (effective_mode & S_IROTH) ? 'r' : '-';
    buf[8] = (effective_mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (effective_mode & S_IXOTH) ? 'x' : '-';
    buf[10] = '\0';
    return buf;
}

void XBOX_stat(const char *name) {
    struct stat st;  // stat结构体
    struct passwd *pwd;   // passwd结构体
    struct group *grp;    // group结构体
    if (lstat(name, &st) < 0) {
        char error_info[1024];
        sprintf(error_info, "stat: cannot statx '%s'",name);
        perror(error_info);
        return;
    }
    char *mode_str = XBOX_stat_access_mode(st.st_mode);

    // 获取用户ID和用户名 组ID和组名
    pwd = getpwuid(st.st_uid);
    grp = getgrgid(st.st_gid);

    // 链接做处理
    if (S_ISLNK(st.st_mode)) {
        char linkname[1024];
        ssize_t r = readlink(name, linkname, sizeof(linkname)-1);
        if (r == -1) {
            perror("readlink");
            return;
        }
        linkname[r] = '\0';
        printf("  File: %s -> %s\n", name, linkname);
    } else {
        printf("  File: %s\n", name);
    }
    printf(
        "  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
        "Device: %llxh/%llud\tInode: %-10llu  Links: %lu\n"
        "Access: (%04o/%s)  Uid: (%5ld/%8s)   Gid: (%5ld/%8s)\n",
        (unsigned long long)st.st_size,
        (unsigned long long)st.st_blocks,
        (unsigned long)st.st_blksize,
        stat_file_type(&st),
        (unsigned long long)st.st_dev,
        (unsigned long long)st.st_dev,
        (unsigned long long)st.st_ino,
        (unsigned long)st.st_nlink,
        (st.st_mode & ~S_IFMT),
        mode_str,
        (unsigned long)st.st_uid,
        (pwd == NULL) ? "UNKNOWN" : pwd->pw_name,
        (unsigned long)st.st_gid,
        (grp == NULL) ? "UNKNOWN" : grp->gr_name);

    // 时间相关
    time_t current_time;
    struct tm *tm;
    char timezone_offset[6];
    char access_time[20], modify_time[20], change_time[20];
    // 格式化时间
    current_time = time(NULL);
    tm = localtime(&current_time);
    strftime(timezone_offset, sizeof(timezone_offset), "%z", tm);

    tm = localtime(&st.st_atime);
    strftime(access_time, sizeof(access_time), "%Y-%m-%d %H:%M:%S", tm);
    tm = localtime(&st.st_mtime);
    strftime(modify_time, sizeof(modify_time), "%Y-%m-%d %H:%M:%S", tm);
    tm = localtime(&st.st_ctime);
    strftime(change_time, sizeof(change_time), "%Y-%m-%d %H:%M:%S", tm);
    free(mode_str);

    // 输出时间信息
    printf("Access: %s.%09ld %s\n", access_time, st.st_atim.tv_nsec, timezone_offset);  // 访问时间
    printf("Modify: %s.%09ld %s\n", modify_time, st.st_mtim.tv_nsec, timezone_offset);  // 修改时间
    printf("Change: %s.%09ld %s\n", change_time, st.st_ctim.tv_nsec, timezone_offset);  // 变更时间

    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "display this help and exit"]),
        XBOX_ARG_STR_GROUPS(&dirs, [name = FILE]),
        XBOX_ARG_BOOLEAN(&dereference, [-L][--dereference][help = "follow links"]),
        XBOX_ARG_BOOLEAN(&file_system,
                         [-f][--file-system][help = "display file system status instead of file status"]),
        XBOX_ARG_STR(&cached, [--cached][help = "specify how to use cached attributes"]),
        XBOX_ARG_BOOLEAN(&terse, [-t][--terse][help = "print the information in terse form"]),
        XBOX_ARG_BOOLEAN(NULL, [-v][--version][help = "output version information and exit"]),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK | XBOX_ARGPARSE_ENABLE_MULTI);
    XBOX_argparse_describe(&parser,
                           "stat",
                           "Display file or file system status.",
                           "XBOX coreutils online help: <https://github.com/luzhixing12345/xbox>");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }
    if (XBOX_ismatch(&parser, "version")) {
        printf("stat (GNU coreutils) 8.32\n");
        XBOX_free_argparse(&parser);
        return 0;
    }

    int n = XBOX_ismatch(&parser, "FILE");
    if (n) {
        for (int i = 0; i < n; i++) {
            XBOX_stat(dirs[i]);
            free(dirs[i]);
        }
        free(dirs);
    } else {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }

    XBOX_free_argparse(&parser);
    return 0;
}