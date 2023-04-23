
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "../argparse.h"


char **files;
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
    char *buf = malloc(sizeof(char)*11);
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
    } else if (S_ISSOCK(mode)) {
        buf[0] = 's';
    } else {
        buf[0] = '-';
        // UNKNOWN type?
    }
    mode_t umask_val = umask(0);        // 获取当前 umask 值
    umask(umask_val);                   // 恢复原来的 umask 值
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

    struct stat statbuf; // stat结构体
    struct passwd *pwd; // passwd结构体
    struct group *grp; // group结构体
    stat(name, &statbuf);
    char *mode_str = XBOX_stat_access_mode(statbuf.st_mode);

    // 获取用户ID和用户名 组ID和组名
    pwd = getpwuid(statbuf.st_uid);
    grp = getgrgid(statbuf.st_gid);

    printf("  File: %s\n", name);
    printf(
        "  Size: %-10llu\tBlocks: %-10llu IO Block: %-6lu %s\n"
        "Device: %llxh/%llud\tInode: %-10llu  Links: %lu\n"
        "Access: (%04o/%s)  Uid: (%5ld/%8s)   Gid: (%5ld/%8s)\n",
        (unsigned long long)statbuf.st_size,
        (unsigned long long)statbuf.st_blocks,
        (unsigned long)statbuf.st_blksize,
        stat_file_type(&statbuf),
        (unsigned long long)statbuf.st_dev,
        (unsigned long long)statbuf.st_dev,
        (unsigned long long)statbuf.st_ino,
        (unsigned long)statbuf.st_nlink,
        (statbuf.st_mode & ~S_IFMT),
        mode_str,
        (unsigned long) statbuf.st_uid,
        (pwd == NULL)?"UNKNOWN":pwd->pw_name,
        (unsigned long) statbuf.st_gid,
        (grp == NULL)?"UNKNOWN":grp->gr_name
    );

    // 时间相关
    time_t current_time;
    struct tm *tm;
    char timezone_offset[6];
    char access_time[20], modify_time[20], change_time[20];
    // 格式化时间
    current_time = time(NULL);
    tm = localtime(&current_time);
    strftime(timezone_offset, sizeof(timezone_offset), "%z", tm);

    tm = localtime(&statbuf.st_atime);
    strftime(access_time, sizeof(access_time), "%Y-%m-%d %H:%M:%S", tm);
    tm = localtime(&statbuf.st_mtime);
    strftime(modify_time, sizeof(modify_time), "%Y-%m-%d %H:%M:%S", tm);
    tm = localtime(&statbuf.st_ctime);
    strftime(change_time, sizeof(change_time), "%Y-%m-%d %H:%M:%S", tm);
    free(mode_str);

    // 输出时间信息
    printf("Access: %s.%09ld %s\n", access_time, statbuf.st_atim.tv_nsec,timezone_offset); // 访问时间
    printf("Modify: %s.%09ld %s\n", modify_time, statbuf.st_mtim.tv_nsec,timezone_offset); // 修改时间
    printf("Change: %s.%09ld %s\n", change_time, statbuf.st_ctim.tv_nsec,timezone_offset); // 变更时间

    return;
}

int main(int argc, const char **argv) {

    
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "display this help and exit"]),
        XBOX_ARG_STR_GROUPS(&files, [name = FILE]),
        XBOX_ARG_BOOLEAN(&dereference, [-L][--dereference][help="follow links"]),
        XBOX_ARG_BOOLEAN(&file_system, [-f][--file-system][help="display file system status instead of file status"]),
        XBOX_ARG_STR(&cached, [--cached][help="specify how to use cached attributes"]),
        XBOX_ARG_BOOLEAN(&terse, [-t][--terse][help="print the information in terse form"]),
        XBOX_ARG_BOOLEAN(NULL,[-v][--version][help="output version information and exit"]),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK|XBOX_ARGPARSE_ENABLE_MULTI);
    XBOX_argparse_describe(&parser, "stat", "Display file or file system status.", "XBOX coreutils online help: <https://github.com/luzhixing12345/xbox>");
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
        for (int i=0;i<n;i++) {
            XBOX_stat(files[i]);
            free(files[i]);
        }
        free(files);
    } else {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }

    XBOX_free_argparse(&parser);
    return 0;
}