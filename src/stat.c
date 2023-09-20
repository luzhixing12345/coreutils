
#include <grp.h>
#include <pwd.h>
#include <sys/sysmacros.h>
#include <sys/vfs.h>
#include <time.h>

#include "xbox/xbox.h"

char **dirs;
static int dereference = 0;
static int file_system = 0;
static int terse = 0;

// [?]: 没有 Birth, 和 stat 不同, 和 busybox 相同

typedef struct FS_TYPE {
    unsigned long long x;
    char *name;
} FS_TYPE;

static const FS_TYPE FS_TYPE_LIST[] = {{0xADFF, "affs"},
                                       {0x1CD1, "devpts"},
                                       {0x137D, "ext"},
                                       {0xEF51, "ext2"},
                                       {0xEF53, "ext2/ext3"},
                                       {0x3153464a, "jfs"},
                                       {0x58465342, "xfs"},
                                       {0xF995E849, "hpfs"},
                                       {0x9660, "isofs"},
                                       {0x4000, "isofs"},
                                       {0x4004, "isofs"},
                                       {0x137F, "minix"},
                                       {0x138F, "minix (30 char.)"},
                                       {0x2468, "minix v2"},
                                       {0x2478, "minix v2 (30 char.)"},
                                       {0x4d44, "msdos"},
                                       {0x4006, "fat"},
                                       {0x564c, "novell"},
                                       {0x6969, "nfs"},
                                       {0x9fa0, "proc"},
                                       {0x517B, "smb"},
                                       {0x012FF7B4, "xenix"},
                                       {0x012FF7B5, "sysv4"},
                                       {0x012FF7B6, "sysv2"},
                                       {0x012FF7B7, "coh"},
                                       {0x00011954, "ufs"},
                                       {0x012FD16D, "xia"},
                                       {0x5346544e, "ntfs"},
                                       {0x1021994, "tmpfs"},
                                       {0x52654973, "reiserfs"},
                                       {0x28cd3d45, "cramfs"},
                                       {0x7275, "romfs"},
                                       {0x858458f6, "ramfs"},
                                       {0x73717368, "squashfs"},
                                       {0x62656572, "sysfs"}};

char *get_fs_type_name(int f_type) {
    int length = sizeof(FS_TYPE_LIST) / sizeof(FS_TYPE);
    for (int i = 0; i < length; i++) {
        if (f_type == FS_TYPE_LIST[i].x)
            return FS_TYPE_LIST[i].name;
    }
    return "unknown";
}

char *stat_file_type(struct stat *st) {
    switch (st->st_mode & S_IFMT) {
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

void XBOX_stat(const char *name) {
    struct stat st;
    struct passwd *pwd;
    struct group *grp;

    int (*stat_method)(const char *, struct stat *) = (dereference) ? stat : lstat;
    if (stat_method(name, &st) < 0) {
        char error_info[1024];
        sprintf(error_info, "stat: cannot statx '%s'", name);
        perror(error_info);
        return;
    }

    if (terse) {
        // char *terse_format = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %W %o %C";
        printf("%s %llu %llu %lx %lu %lu %llx %llu %lu %lx %lx %lu %lu %lu %lu\n",
               name,
               (unsigned long long)st.st_size,
               (unsigned long long)st.st_blocks,
               (unsigned long)st.st_mode,
               (unsigned long)st.st_uid,
               (unsigned long)st.st_gid,
               (unsigned long long)st.st_dev,
               (unsigned long long)st.st_ino,
               (unsigned long)st.st_nlink,
               (unsigned long)major(st.st_rdev),
               (unsigned long)minor(st.st_rdev),
               (unsigned long)st.st_atime,
               (unsigned long)st.st_mtime,
               (unsigned long)st.st_ctime,
               (unsigned long)st.st_blksize);
        return;
    }

    if (file_system) {
        struct statfs stats;
        if (statfs(name, &stats) == -1) {
            perror("statfs");
            exit(1);
        }
        printf("  File: \"%s\"\n", name);
        printf("    ID: %x%x Namelen: %-7ld Type: %s\n",
               (unsigned int)stats.f_fsid.__val[0],
               (unsigned int)stats.f_fsid.__val[1],
               stats.f_namelen,
               get_fs_type_name(stats.f_type));
        printf("Block size: %-10ld Fundamental block size: %-10ld\n", stats.f_bsize, stats.f_frsize);
        printf("Blocks: Total: %-10lu Free: %-10lu Available: %lu\n", stats.f_blocks, stats.f_bfree, stats.f_bavail);
        printf("Inodes: Total: %-10lu Free: %-10lu\n", stats.f_files, stats.f_ffree);
        return;
    }

    char *st_mode_rwx = XBOX_stat_access_mode(st.st_mode);

    // 获取用户ID和用户名 组ID和组名
    pwd = getpwuid(st.st_uid);
    grp = getgrgid(st.st_gid);

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

    // 链接做处理
    if (S_ISLNK(st.st_mode)) {
        char linkname[1024];
        ssize_t r = readlink(name, linkname, sizeof(linkname) - 1);
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
        st_mode_rwx,
        (unsigned long)st.st_uid,
        (pwd == NULL) ? "UNKNOWN" : pwd->pw_name,
        (unsigned long)st.st_gid,
        (grp == NULL) ? "UNKNOWN" : grp->gr_name);

    // 输出时间信息
    printf("Access: %s.%09ld %s\n", access_time, st.st_atim.tv_nsec, timezone_offset);  // 访问时间
    printf("Modify: %s.%09ld %s\n", modify_time, st.st_mtim.tv_nsec, timezone_offset);  // 修改时间
    printf("Change: %s.%09ld %s\n", change_time, st.st_ctim.tv_nsec, timezone_offset);  // 变更时间
    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, "-h", "--help", "display this help and exit", NULL, "help"),
        XBOX_ARG_BOOLEAN(NULL, "-v", "--version", "output version information and exit", NULL, "version"),
        XBOX_ARG_STRS(&dirs, NULL, NULL, NULL, NULL, NULL),
        XBOX_ARG_BOOLEAN(&dereference, "-L", "--dereference", "follow links", NULL, NULL),
        XBOX_ARG_BOOLEAN(
            &file_system, "-f", "--file-system", "display file system status instead of file status", NULL, NULL),
        XBOX_ARG_BOOLEAN(&terse, "-t", "--terse", "print the information in terse form", NULL, NULL),
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

    int n = XBOX_ismatch(&parser, "FILE");
    if (XBOX_ismatch(&parser, "version")) {
        printf("%s\n", XBOX_VERSION);
        XBOX_free_argparse(&parser);
        return 0;
    }
    if (n) {
        for (int i = 0; i < n; i++) {
            XBOX_stat(dirs[i]);
        }
    } else {
        XBOX_argparse_info(&parser);
    }

    XBOX_free_argparse(&parser);
    return 0;
}