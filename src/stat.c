
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <time.h>
#include <unistd.h>

#include "argparse.h"

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
                                {0x62656572, "sysfs"}
                                };

char *get_fs_type_name(int f_type) {

    int length = sizeof(FS_TYPE_LIST)/sizeof(FS_TYPE);
    for(int i=0;i<length;i++) {
        if (f_type == FS_TYPE_LIST[i].x) return FS_TYPE_LIST[i].name;
    }
    return "unknown";
}

/* Return *ST's birth time, if available; otherwise return a value
   with tv_sec and tv_nsec both equal to -1.  */
struct timespec get_stat_birthtime(struct stat const *st) {
    struct timespec t;

#if (defined HAVE_STRUCT_STAT_ST_BIRTHTIMESPEC_TV_NSEC || defined HAVE_STRUCT_STAT_ST_BIRTHTIM_TV_NSEC)
    t = STAT_TIMESPEC(st, st_birthtim);
#elif defined HAVE_STRUCT_STAT_ST_BIRTHTIMENSEC
    t.tv_sec = st->st_birthtime;
    t.tv_nsec = st->st_birthtimensec;
#elif defined _WIN32 && !defined __CYGWIN__
    /* Native Windows platforms (but not Cygwin) put the "file creation
       time" in st_ctime (!).  See
       <https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/stat-functions>.  */
#if _GL_WINDOWS_STAT_TIMESPEC
    t = st->st_ctim;
#else
    t.tv_sec = st->st_ctime;
    t.tv_nsec = 0;
#endif
#else
    /* Birth time is not supported.  */
    t.tv_sec = -1;
    t.tv_nsec = -1;
#endif

#if (defined HAVE_STRUCT_STAT_ST_BIRTHTIMESPEC_TV_NSEC || defined HAVE_STRUCT_STAT_ST_BIRTHTIM_TV_NSEC || \
     defined HAVE_STRUCT_STAT_ST_BIRTHTIMENSEC)
    /* FreeBSD and NetBSD sometimes signal the absence of knowledge by
       using zero.  Attempt to work around this problem.  Alas, this can
       report failure even for valid timestamps.  Also, NetBSD
       sometimes returns junk in the birth time fields; work around this
       bug if it is detected.  */
    if (!(t.tv_sec && 0 <= t.tv_nsec && t.tv_nsec < 1000000000)) {
        t.tv_sec = -1;
        t.tv_nsec = -1;
    }
#endif

#if HAVE_GETATTRAT
    if (t.tv_nsec < 0) {
        nvlist_t *response;
        if ((fd < 0 ? getattrat(AT_FDCWD, XATTR_VIEW_READWRITE, filename, &response)
                    : fgetattr(fd, XATTR_VIEW_READWRITE, &response)) == 0) {
            uint64_t *val;
            uint_t n;
            if (nvlist_lookup_uint64_array(response, A_CRTIME, &val, &n) == 0 && 2 <= n &&
                val[0] <= TYPE_MAXIMUM(time_t) && val[1] < 1000000000 * 2 /* for leap seconds */) {
                t.tv_sec = val[0];
                t.tv_nsec = val[1];
            }
            nvlist_free(response);
        }
    }
#endif

    return t;
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

    struct timespec birth_ts = get_stat_birthtime(&st);
    char birth_time[20];
    int has_birth_time = 0;
    if (birth_ts.tv_nsec < 0) {
        has_birth_time = 0;
        birth_time[0] = '-';
        birth_time[1] = '\0';
    } else {
        has_birth_time = 1;
        struct tm *birth_tm;
        time_t seconds = birth_ts.tv_sec;
        gmtime_r(&seconds, birth_tm);
        strftime(birth_time, sizeof(birth_time), "%Y-%m-%d %H:%M:%S", birth_tm);
    }

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
    if (has_birth_time) {
        printf("Change: %s.%09ld %s\n", birth_time, birth_ts.tv_nsec, timezone_offset);
    } else {
        printf(" Birth: -\n");
    }

    free(st_mode_rwx);
    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {
        XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "display this help and exit"]),
        XBOX_ARG_STR_GROUPS(&dirs, [name = FILE]),
        XBOX_ARG_BOOLEAN(&dereference, [-L][--dereference][help = "follow links"]),
        XBOX_ARG_BOOLEAN(&file_system,
                         [-f][--file-system][help = "display file system status instead of file status"]),
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