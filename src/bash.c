/*
 *Copyright (c) 2023 All rights reserved
 *@description: bash
 *@author: Zhixing Lu
 *@date: 2023-05-05
 *@email: luzhixing12345@163.com
 *@Github: luzhixing12345
 */

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "xbox/xargparse.h"

static const char *VERSION = "v0.0.1";
typedef void handler_t(int);

/* Misc manifest constants */
#define MAXLINE 1024   /* max line size */
#define MAXARGS 128    /* max args on a command line */
#define MAXJOBS 16     /* max jobs at any point in time */
#define MAXJID 1 << 16 /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

extern char **environ;
char prompt[] = "tsh> ";
int verbose = 0;
char sbuf[MAXLINE];

void unix_error(char *msg) {
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */

int parseline(const char *cmdline, char **argv) {
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf) - 1] = ' ';   /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'' || *buf == '\"') {
        char quote = *buf;
        buf++;
        delim = strchr(buf, quote);
    } else {
        delim = strchr(buf, ' ');
    }

    while (delim) {
        if (*(delim - 1) == '\'' || *(delim - 1) == '\"') {
            delim = strchr(delim + 1, *(delim - 1));
        } else {
            argv[argc++] = buf;
            *delim = 0;
            buf = delim + 1;
            while (*buf && (*buf == ' ')) buf++;

            if (*buf == '\'' || *buf == '\"') {
                char quote = *buf;
                buf++;
                delim = strchr(buf, quote);
            } else {
                delim = strchr(buf, ' ');
            }
        }
    }
    argv[argc] = NULL;

    if (argc == 0) /* ignore blank line */
        return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0) {
        argv[--argc] = NULL;
    }

    for (int i = 0; i < argc; i++) {
        printf("i = %d: [%s]\n", i, argv[i]);
    }
    return bg;
}

int builtin_cmd(char **argv) {
    if (!strcmp(argv[0], "quit")) {
        exit(0);
    }
    return 0; /* not a builtin command */
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid) {
    sigset_t mask;
    sigemptyset(&mask);
    //   while (pid == fgpid(jobs))
    //     sigsuspend(&mask);
    return;
}

void eval(char *cmdline) {
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
        return;
    return;
    sigset_t mask_all, mask_one, prev;
    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);
    if (!builtin_cmd(argv)) {
        sigprocmask(SIG_BLOCK, &mask_one, &prev);
        if ((pid = fork()) == 0) {
            sigprocmask(SIG_SETMASK, &prev, NULL);
            setpgid(0, 0);
            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found\n", argv[0]);
                exit(0);
            }
        }
        if (!bg) {
            // 对于前台进程，添加job后解除阻塞，并通过waitfg等待子进程结束后回收
            sigprocmask(SIG_BLOCK, &mask_all, NULL);
            sigprocmask(SIG_SETMASK, &prev, NULL);
            waitfg(pid);
        } else {
            // 后台进程不需要等待子进程，进程结束之后收到SIGCHLD信号回收即可
            sigprocmask(SIG_BLOCK, &mask_all, NULL);
            sigprocmask(SIG_SETMASK, &prev, NULL);
        }
    }

    return;
}

handler_t *Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg) {
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

int main(int argc, const char **argv) {
    argparse_option options[] = {XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
                                 XBOX_ARG_INT(NULL, [-v][--version][help = "show version"]),
                                 XBOX_ARG_END()};

    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(&parser, "pwd", "Print the name of the current working directory", "");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
    }

    if (XBOX_ismatch(&parser, "version")) {
        printf("%s\n", VERSION);
    }

    // 输出
    dup2(1, 2);
    // Signal(SIGINT, sigint_handler);   /* ctrl-c */
    // Signal(SIGTSTP, sigtstp_handler); /* ctrl-z */
    // Signal(SIGCHLD, sigchld_handler); /* Terminated or stopped child */
    // Signal(SIGQUIT, sigquit_handler);

    char cmdline[MAXLINE];
    int emit_prompt = 1;

    /* Execute the shell's read/eval loop */
    while (1) {
        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */

    XBOX_free_argparse(&parser);
    return 0;
}