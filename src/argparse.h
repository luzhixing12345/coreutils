/**
 * Copyright (C) 2012-2015 Yecheng Fu <cofyc.jackson at gmail dot com>
 * All rights reserved.
 *
 * Use of this source code is governed by a MIT-style license that can be found
 * in the LICENSE file.
 */
#ifndef ARGPARSE_H
#define ARGPARSE_H

/* For c++ compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct argparse;
struct argparse_option;

typedef int argparse_callback (struct argparse *parser,
                               const struct argparse_option *option);

enum argparse_flag {
    ARGPARSE_STOP_AT_NON_OPTION  = 1 << 0,
    ARGPARSE_IGNORE_UNKNOWN_ARGS = 1 << 1,
};

enum argparse_option_type {
    ARGPARSE_OPT_END,
    ARGPARSE_OPT_BOOLEAN,
    ARGPARSE_OPT_INTEGER,
    ARGPARSE_OPT_FLOAT,
    ARGPARSE_OPT_STRING,
    ARGPARSE_OPT_GROUP,
};

typedef struct {
    enum argparse_option_type type;
    char *argument_str;
    char *short_name;
    char *long_name;
    char *help_info;
    char *default_value;
    int must;
} argparse_option;

static const char argparse_separate_char = ',';
/**
 * argpparse
 */
typedef struct {
    // user supplied
    argparse_option *options;
    const char *description;    // a description after usage
    const char *epilog;         // a description at the end
    // internal context
    int argc;
    int args_number;
    const char **argv;
} argparse;

// built-in option macros
#define BST_ARG_BOOL(...) { ARGPARSE_OPT_BOOLEAN, #__VA_ARGS__ }
#define BST_ARG_INT(...) { ARGPARSE_OPT_INTEGER, #__VA_ARGS__ }
#define BST_ARG_FLOAT(...)   { ARGPARSE_OPT_FLOAT, #__VA_ARGS__ }
#define BST_ARG_STR(...)  { ARGPARSE_OPT_STRING, #__VA_ARGS__ }
#define BST_ARG_GROUP(...)  { ARGPARSE_OPT_GROUP, #__VA_ARGS__ }
#define BST_ARG_END(...) { ARGPARSE_OPT_END, #__VA_ARGS__ }

void BST_argparse_init(argparse *parser, argparse_option *options) {
    memset(parser, 0, sizeof(*parser));
    parser->description = NULL;
    parser->epilog = NULL;
    parser->options = options;
    parser->args_number = 0;
    return;
}

void BST_argparse_describe(argparse *parser, const char *description, const char *epilog) {
    parser->description = description;
    parser->epilog = epilog;
    return;
}

void argparse_argument_parse(argparse *parser) {
    argparse_option *p = parser->options;
    while (p->type!=ARGPARSE_OPT_END) {
        p++;
        parser->args_number++;
    }
    for (int i=0;i<parser->args_number;i++) {
        char *str = parser->options[i].argument_str;
        char *p = str;
        while (*str) {
            while (*p && *p!= argparse_separate_char) {
                p++;
            }
        }
    }
}

void BST_argparse_parse(argparse *parser, int argc, const char **argv) {
    parser->argc = argc - 1;
    parser->argv = argv + 1;
    
    argparse_argument_parse(parser);

    return;
}

#ifdef __cplusplus
}
#endif

#endif