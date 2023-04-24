
# stat

```cpp
struct stat {
    dev_t     st_dev;         /* ID of device containing file */
    ino_t     st_ino;         /* Inode number */
    mode_t    st_mode;        /* File type and mode */
    nlink_t   st_nlink;       /* Number of hard links */
    uid_t     st_uid;         /* User ID of owner */
    gid_t     st_gid;         /* Group ID of owner */
    dev_t     st_rdev;        /* Device ID (if special file) */
    off_t     st_size;        /* Total size, in bytes */
    blksize_t st_blksize;     /* Block size for filesystem I/O */
    blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */

    /* Since Linux 2.6, the kernel supports nanosecond
        precision for the following timestamp fields.
        For the details before Linux 2.6, see NOTES. */

    struct timespec st_atim;  /* Time of last access */
    struct timespec st_mtim;  /* Time of last modification */
    struct timespec st_ctim;  /* Time of last status change */

#define st_atime st_atim.tv_sec      /* Backward compatibility */
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
};
```

```bash
  File: Makefile
  Size: 2107            Blocks: 8          IO Block: 4096   regular file
Device: 820h/2080d      Inode: 307167      Links: 1
Access: (0644/-rw-r--r--)  Uid: ( 1000/  kamilu)   Gid: ( 1000/  kamilu)
Access: 2023-04-23 20:03:37.289298652 +0800
Modify: 2023-04-22 15:09:17.407316384 +0800
Change: 2023-04-22 15:09:17.407316384 +0800
 Birth: 2023-04-22 12:15:39.547517747 +0800
```

stat 和 lstat

软链接

Birth?