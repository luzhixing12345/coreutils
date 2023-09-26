
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xbox/xargparse.h"
#include "xbox/xterm.h"

// 预估了一下当前 LS_COLORS 的长度是 1508
#define MAX_DC_LENGTH 1600

int print_database = 0;
int bash_dircolors = 0;
int csh_dircolors = 0;

// coreutils 采用的是 local.mk + dcgen 为 dircolors.c 中的 G_line 在编译前做批量替换
// 将 dircolors.hin 的内容直接输入进去
// 这里简化了这一步, 直接写死在文件里

const static char *builtin_dircolors_str =
    "# Configuration file for dircolors, a utility to help you set the\n"
    "# LS_COLORS environment variable used by GNU ls with the --color option.\n"
    "\n"
    "# Copyright (C) 1996-2023 Free Software Foundation, Inc.\n"
    "# Copying and distribution of this file, with or without modification,\n"
    "# are permitted provided the copyright notice and this notice are preserved.\n"
    "\n"
    "#\n"
    "# The keywords COLOR, OPTIONS, and EIGHTBIT (honored by the\n"
    "# slackware version of dircolors) are recognized but ignored.\n"
    "\n"
    "# Global config options can be specified before TERM or COLORTERM entries\n"
    "\n"
    "# ===================================================================\n"
    "# Terminal filters\n"
    "# ===================================================================\n"
    "# Below are TERM or COLORTERM entries, which can be glob patterns, which\n"
    "# restrict following config to systems with matching environment variables.\n"
    "COLORTERM ?*\n"
    "TERM Eterm\n"
    "TERM ansi\n"
    "TERM *color*\n"
    "TERM con[0-9]*x[0-9]*\n"
    "TERM cons25\n"
    "TERM console\n"
    "TERM cygwin\n"
    "TERM *direct*\n"
    "TERM dtterm \n"
    "TERM gnome\n"
    "TERM hurd\n"
    "TERM jfbterm\n"
    "TERM konsole\n"
    "TERM kterm\n"
    "TERM linux\n"
    "TERM linux-c\n"
    "TERM mlterm\n"
    "TERM putty\n"
    "TERM rxvt*\n"
    "TERM screen*\n"
    "TERM st\n"
    "TERM terminator\n"
    "TERM tmux*\n"
    "TERM vt100\n"
    "TERM xterm*\n"
    "\n"
    "# ===================================================================\n"
    "# Basic file attributes\n"
    "# ===================================================================\n"
    "# Below are the color init strings for the basic file types.\n"
    "# One can use codes for 256 or more colors supported by modern terminals.\n"
    "# The default color codes use the capabilities of an 8 color terminal\n"
    "# with some additional attributes as per the following codes:\n"
    "# Attribute codes:\n"
    "# 00=none 01=bold 04=underscore 05=blink 07=reverse 08=concealed\n"
    "# Text color codes:\n"
    "# 30=black 31=red 32=green 33=yellow 34=blue 35=magenta 36=cyan 37=white\n"
    "# Background color codes:\n"
    "# 40=black 41=red 42=green 43=yellow 44=blue 45=magenta 46=cyan 47=white\n"
    "#NORMAL 00	# no color code at all\n"
    "#FILE 00	# regular file: use no color at all\n"
    "RESET 0		# reset to \"normal\" color\n"
    "DIR 01;34	# directory\n"
    "LINK 01;36	# symbolic link.  (If you set this to 'target' instead of a\n"
    "                # numerical value, the color is as for the file pointed to.)\n"
    "MULTIHARDLINK 00	# regular file with more than one link\n"
    "FIFO 40;33	# pipe\n"
    "SOCK 01;35	# socket\n"
    "DOOR 01;35	# door\n"
    "BLK 40;33;01	# block device driver\n"
    "CHR 40;33;01	# character device driver\n"
    "ORPHAN 40;31;01 # symlink to nonexistent file, or non-stat'able file ...\n"
    "MISSING 00      # ... and the files they point to\n"
    "SETUID 37;41	# file that is setuid (u+s)\n"
    "SETGID 30;43	# file that is setgid (g+s)\n"
    "CAPABILITY 00	# file with capability (very expensive to lookup)\n"
    "STICKY_OTHER_WRITABLE 30;42 # dir that is sticky and other-writable (+t,o+w)\n"
    "OTHER_WRITABLE 34;42 # dir that is other-writable (o+w) and not sticky\n"
    "STICKY 37;44	# dir with the sticky bit set (+t) and not other-writable\n"
    "\n"
    "# This is for files with execute permission:\n"
    "EXEC 01;32\n"
    "\n"
    "# ===================================================================\n"
    "# File extension attributes\n"
    "# ===================================================================\n"
    "# List any file extensions like '.gz' or '.tar' that you would like ls\n"
    "# to color below. Put the suffix, a space, and the color init string.\n"
    "# (and any comments you want to add after a '#').\n"
    "# Suffixes are matched case insensitively, but if you define different\n"
    "# init strings for separate cases, those will be honored.\n"
    "#\n"
    "\n"
    "# If you use DOS-style suffixes, you may want to uncomment the following:\n"
    "#.cmd 01;32 # executables (bright green)\n"
    "#.exe 01;32\n"
    "#.com 01;32\n"
    "#.btm 01;32\n"
    "#.bat 01;32\n"
    "# Or if you want to color scripts even if they do not have the\n"
    "# executable bit actually set.\n"
    "#.sh  01;32\n"
    "#.csh 01;32\n"
    "\n"
    "# archives or compressed (bright red)\n"
    ".tar 01;31\n"
    ".tgz 01;31\n"
    ".arc 01;31\n"
    ".arj 01;31\n"
    ".taz 01;31\n"
    ".lha 01;31\n"
    ".lz4 01;31\n"
    ".lzh 01;31\n"
    ".lzma 01;31\n"
    ".tlz 01;31\n"
    ".txz 01;31\n"
    ".tzo 01;31\n"
    ".t7z 01;31\n"
    ".zip 01;31\n"
    ".z   01;31\n"
    ".dz  01;31\n"
    ".gz  01;31\n"
    ".lrz 01;31\n"
    ".lz  01;31\n"
    ".lzo 01;31\n"
    ".xz  01;31\n"
    ".zst 01;31\n"
    ".tzst 01;31\n"
    ".bz2 01;31\n"
    ".bz  01;31\n"
    ".tbz 01;31\n"
    ".tbz2 01;31\n"
    ".tz  01;31\n"
    ".deb 01;31\n"
    ".rpm 01;31\n"
    ".jar 01;31\n"
    ".war 01;31\n"
    ".ear 01;31\n"
    ".sar 01;31\n"
    ".rar 01;31\n"
    ".alz 01;31\n"
    ".ace 01;31\n"
    ".zoo 01;31\n"
    ".cpio 01;31\n"
    ".7z  01;31\n"
    ".rz  01;31\n"
    ".cab 01;31\n"
    ".wim 01;31\n"
    ".swm 01;31\n"
    ".dwm 01;31\n"
    ".esd 01;31\n"
    "\n"
    "# image formats\n"
    ".avif 01;35\n"
    ".jpg 01;35\n"
    ".jpeg 01;35\n"
    ".mjpg 01;35\n"
    ".mjpeg 01;35\n"
    ".gif 01;35\n"
    ".bmp 01;35\n"
    ".pbm 01;35\n"
    ".pgm 01;35\n"
    ".ppm 01;35\n"
    ".tga 01;35\n"
    ".xbm 01;35\n"
    ".xpm 01;35\n"
    ".tif 01;35\n"
    ".tiff 01;35\n"
    ".png 01;35\n"
    ".svg 01;35\n"
    ".svgz 01;35\n"
    ".mng 01;35\n"
    ".pcx 01;35\n"
    ".mov 01;35\n"
    ".mpg 01;35\n"
    ".mpeg 01;35\n"
    ".m2v 01;35\n"
    ".mkv 01;35\n"
    ".webm 01;35\n"
    ".webp 01;35\n"
    ".ogm 01;35\n"
    ".mp4 01;35\n"
    ".m4v 01;35\n"
    ".mp4v 01;35\n"
    ".vob 01;35\n"
    ".qt  01;35\n"
    ".nuv 01;35\n"
    ".wmv 01;35\n"
    ".asf 01;35\n"
    ".rm  01;35\n"
    ".rmvb 01;35\n"
    ".flc 01;35\n"
    ".avi 01;35\n"
    ".fli 01;35\n"
    ".flv 01;35\n"
    ".gl 01;35\n"
    ".dl 01;35\n"
    ".xcf 01;35\n"
    ".xwd 01;35\n"
    ".yuv 01;35\n"
    ".cgm 01;35\n"
    ".emf 01;35\n"
    "\n"
    "# https://wiki.xiph.org/MIME_Types_and_File_Extensions\n"
    ".ogv 01;35\n"
    ".ogx 01;35\n"
    "\n"
    "# audio formats\n"
    ".aac 00;36\n"
    ".au 00;36\n"
    ".flac 00;36\n"
    ".m4a 00;36\n"
    ".mid 00;36\n"
    ".midi 00;36\n"
    ".mka 00;36\n"
    ".mp3 00;36\n"
    ".mpc 00;36\n"
    ".ogg 00;36\n"
    ".ra 00;36\n"
    ".wav 00;36\n"
    "\n"
    "# https://wiki.xiph.org/MIME_Types_and_File_Extensions\n"
    ".oga 00;36\n"
    ".opus 00;36\n"
    ".spx 00;36\n"
    ".xspf 00;36";

// 后面这部分都是后加的
// "\n"
// "# backup files\n"
// "*~ 00;90\n"
// "*# 00;90\n"
// ".bak 00;90\n"
// ".crdownload 00;90\n"
// ".dpkg-dist 00;90\n"
// ".dpkg-new 00;90\n"
// ".dpkg-old 00;90\n"
// ".dpkg-tmp 00;90\n"
// ".old 00;90\n"
// ".orig 00;90\n"
// ".part 00;90\n"
// ".rej 00;90\n"
// ".rpmnew 00;90\n"
// ".rpmorig 00;90\n"
// ".rpmsave 00;90\n"
// ".swp 00;90\n"
// ".tmp 00;90\n"
// ".ucf-dist 00;90\n"
// ".ucf-new 00;90\n"
// ".ucf-old 00;90\n"
// "\n"
// "#\n"
// "# Subsequent TERM or COLORTERM entries, can be used to add / override\n"
// "# config specific to those matching environment variables.";

// dircolors.hin 中的特殊字符串与 LS_COLORS 对应名字的映射关系

typedef struct {
    char *full_name;
    char *short_name;
    int name_length;
} dc_map;

const dc_map builtin_dc_map[18] = {{"RESET", "rs", 5},
                                   {"DIR", "di", 3},
                                   {"LINK", "ln", 4},
                                   {"MULTIHARDLINK", "mh", 13},
                                   {"FIFO", "pi", 4},
                                   {"SOCK", "so", 4},
                                   {"DOOR", "do", 4},
                                   {"BLK", "bd", 3},
                                   {"CHR", "cd", 3},
                                   {"ORPHAN", "or", 6},
                                   {"MISSING", "mi", 7},
                                   {"SETUID", "su", 6},
                                   {"SETGID", "sg", 6},
                                   {"CAPABILITY", "ca", 10},
                                   {"STICKY_OTHER_WRITABLE", "tw", 21},
                                   {"OTHER_WRITABLE", "ow", 14},
                                   {"STICKY", "st", 6},
                                   {"EXEC", "ex", 4}};

char *dc_parse_file(const char *dc_str) {
    char *ls_colors = malloc(sizeof(char) * MAX_DC_LENGTH);
    int ls_colors_p = 0;

    int p = -1;
    int length = -1;

    int str_length = strlen(dc_str);
    for (int i = 0; i < str_length; i++) {
        // printf("[%d]: %c\n", i, dc_str[i]);
        if (dc_str[i] == '#') {
            // 跳过注释
            while (i < str_length && dc_str[i] != '\n') {
                i++;
            }
            continue;
        } else if (dc_str[i] == '\n' || dc_str[i] == ' ') {
            // 跳过空行和空格
            continue;
        } else {
            // 正常的 key value
            p = i;
            length = 0;
            while (i < str_length && dc_str[i] != ' ') {
                length++;
                i++;
            }
            // printf("key length = %d\n", length);
            if (length == 0) {
                continue;
            }
            if (!strncmp(dc_str + p, "TERM", length) || !strncmp(dc_str + p, "COLORTERM", length)) {
                // 对于 TERM 和 COLORTERM 直接跳过
                while (i < str_length && dc_str[i] != '\n') {
                    i++;
                }
                continue;
            } else {
                // printf("dc_str[0] = %c\n", dc_str[p]);
                if (dc_str[p] == '.') {
                    ls_colors[ls_colors_p++] = '*';
                    strncpy(ls_colors + ls_colors_p, dc_str + p, length);
                    ls_colors_p += length;
                    ls_colors[ls_colors_p++] = '=';
                } else {
                    int match = 0;
                    for (int j = 0; j < 18; j++) {
                        if (builtin_dc_map[j].name_length == length &&
                            !strncmp(builtin_dc_map[j].full_name, dc_str + p, length)) {
                            strcpy(ls_colors + ls_colors_p, builtin_dc_map[j].short_name);

                            // 因为默认的 short_name 长度都为2, 所以这里写作2, 如果后续添加了其他缩写形式这里改为
                            // strlen(builtin_dc_map[j].short_name) 即可
                            ls_colors_p += 2;
                            ls_colors[ls_colors_p++] = '=';
                            match = 1;
                        }
                    }
                    if (!match) {
                        printf("error: [%s] no match in builtin dircolors map!\n", dc_str + p);
                        free(ls_colors);
                        return NULL;
                    }
                }
                // 越过空格
                while (i < str_length && (dc_str[i] == ' ' || dc_str[i] == '\t')) {
                    i++;
                }
                p = i;
                length = 0;
                while (i < str_length && (dc_str[i] == ';' || (dc_str[i] >= '0' && dc_str[i] <= '9'))) {
                    length++;
                    i++;
                }
                // printf("value length = %d\n", length);
                if (length == 0) {
                    continue;
                }
                strncpy(ls_colors + ls_colors_p, dc_str + p, length);
                ls_colors_p += length;
                ls_colors[ls_colors_p++] = ':';
            }
            while (i < str_length && (dc_str[i] == ' ' || dc_str[i] == '\t')) {
                i++;
            }
            i--;
        }
    }
    ls_colors[ls_colors_p] = 0;
    return ls_colors;
}

void dircolors() {
    if (print_database) {
        printf("%s\n", builtin_dircolors_str);
    } else {
        char *ls_colors = dc_parse_file(builtin_dircolors_str);
        if (!ls_colors) {
            return;
        }
        if (csh_dircolors) {
            printf("setenv LS_COLORS '%s'\n", ls_colors);
        } else {
            printf("LS_COLORS='%s';\n", ls_colors);
            printf("export LS_COLORS\n");
        }
        free(ls_colors);
    }

    return;
}

int main(int argc, const char **argv) {
    argparse_option options[] = {

        XBOX_ARG_BOOLEAN(&print_database, "-p", "--print-database", "output defaults", NULL, NULL),
        XBOX_ARG_BOOLEAN(&bash_dircolors, "-b", "--sh", "output Bourne shell code to set LS_COLORS", NULL, "bash"),
        XBOX_ARG_BOOLEAN(&csh_dircolors, "-c", "--csh", "output C shell code to set LS_COLORS", NULL, "csh"),
        XBOX_ARG_BOOLEAN(NULL, "-h", "--help", "display this help and exit", NULL, "help"),
        XBOX_ARG_BOOLEAN(NULL, "-v", "--version", "output version information and exit", NULL, "version"),
        XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(&parser, "dircolors", "Output commands to set the LS_COLORS environment variable.", "");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }

    if (XBOX_ismatch(&parser, "version")) {
        printf("%s\n", XBOX_VERSION);
        XBOX_free_argparse(&parser);
        return 0;
    }

    // 同时出现使用最后一个
    if (bash_dircolors && csh_dircolors) {
        if (XBOX_match_pos(&parser, "bash") > XBOX_match_pos(&parser, "csh")) {
            csh_dircolors = 0;
        } else {
            bash_dircolors = 0;
        }
    }

    dircolors();

    XBOX_free_argparse(&parser);
}