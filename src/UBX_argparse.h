/**
 * Copyright (C) 2012-2015 Yecheng Fu <cofyc.jackson at gmail dot com>
 * All rights reserved.
 *
 * Use of this source code is governed by a MIT-style license that can be found
 * in the LICENSE file.
 */
#pragma once

#include <stdint.h>

#include "UBX_string.h"
#include "UBX_define.h"

struct argparse;
struct argparse_option;

typedef int argparse_callback(struct argparse *parser,
                              const struct argparse_option *option);

enum argparse_flag {
    ARGPARSE_STOP_AT_NON_OPTION = 1 << 0,
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
    char *name;
    char *help_info;
    char *default_value;
    int must;

} argparse_option;

/**
 * argpparse
 */
typedef struct {
    // user supplied
    const char *name;
    argparse_option *options;
    const char *description;  // a description after usage
    const char *epilog;       // a description at the end
    int args_number;
} argparse;

// built-in option macros
#define UBX_ARG_BOOL(...) \
    { ARGPARSE_OPT_BOOLEAN, #__VA_ARGS__ }
#define UBX_ARG_INT(...) \
    { ARGPARSE_OPT_INTEGER, #__VA_ARGS__ }
#define UBX_ARG_FLOAT(...) \
    { ARGPARSE_OPT_FLOAT, #__VA_ARGS__ }
#define UBX_ARG_STR(...) \
    { ARGPARSE_OPT_STRING, #__VA_ARGS__ }
#define UBX_ARG_GROUP(...) \
    { ARGPARSE_OPT_GROUP, #__VA_ARGS__ }
#define UBX_ARG_END(...) \
    { ARGPARSE_OPT_END, #__VA_ARGS__ }

void UBX_argparse_init(argparse *parser, argparse_option *options) {
    memset(parser, 0, sizeof(*parser));
    parser->name = NULL;
    parser->description = NULL;
    parser->epilog = NULL;
    parser->options = options;
    parser->args_number = 0;
    while (options->type != ARGPARSE_OPT_END) {
        options++;
        parser->args_number++;
    }
    for (int i=0;i<parser->args_number;i++) {
        parser->options[i].short_name = NULL;
        parser->options[i].long_name = NULL;
        parser->options[i].name = NULL;
        parser->options[i].help_info = NULL;
        parser->options[i].default_value = NULL;
        parser->options[i].must = 0;
    }
    return;
}

void UBX_argparse_describe(argparse *parser, const char *name, const char *description,
                           const char *epilog) {
    parser->name = name;
    parser->description = description;
    parser->epilog = epilog;
    return;
}


void free_argparse_option (argparse_option *options, int args_number) {
    fprintf(stderr, "argument parse error, free options\n");
    for (int i=0;i<args_number;i++) {
        if (options[i].short_name) {
            free(options[i].short_name);
        }
        if (options[i].long_name) {
            free(options[i].long_name);
        }
        if (options[i].name) {
            free(options[i].name);
        }
        if (options[i].help_info) {
            free(options[i].help_info);
        }
        if (options[i].default_value) {
            free(options[i].default_value);
        }
        
    }
    fprintf(stderr, "finished free options\n");
}

void split_argparse_option(char *str, argparse_option *option, int args_number) {
    int length = strlen(str);
    int p = 0;
    int match_flag = 0;
    for (int i = 0; i < length; i++) {
        if (str[i] == '[') {
            if (match_flag) {
                fprintf(stderr, "miss ] in %s\n", str);
                free_argparse_option(option, args_number);
                exit(UBX_FORMAT_ERROR);
            }
            p = i + 1;
            match_flag = 1;
        } else if (str[i] == ']') {
            if (!match_flag) {
                fprintf(stderr, "mismatch between [ and ] in %s\n", str);
                free_argparse_option(option, args_number);
                exit(UBX_FORMAT_ERROR);
            }
            match_flag = 0;
            char *argument = malloc(sizeof(char) * (i - p + 1));
            strncpy(argument, str + p, i - p);
            argument[i - p] = '\0';
            if (i - p < 2) {
                fprintf(stderr, "argument define [%s] too short in %s\n", argument, str);
                free_argparse_option(option, args_number);
                free(argument);
                exit(UBX_FORMAT_ERROR);
            }
            // 给 options 完善选项
            int argument_length = strlen(argument);
            if (argument[0] == '-') {
                // --long_name
                if (argument[1] == '-') {
                    if (argument_length < 4) {
                        fprintf(stderr, "[%s] should be at least 4 characters\n", argument);
                        free_argparse_option(option, args_number);
                        free(argument);
                        exit(UBX_FORMAT_ERROR);
                    } else {
                        option->long_name = argument;
                    }
                } else {
                    // -short_name
                    if (argument_length != 2) {
                        fprintf(stderr, "[%s] must be 2 characters for short name\n", argument);
                        free_argparse_option(option, args_number);
                        free(argument);
                        exit(UBX_FORMAT_ERROR);
                    } else {
                        option->short_name = argument;
                    }
                }
            }
            // printf("%s\n", argument);
            // free(argument);
        }
    }
}

void argparse_argument_parse(argparse *parser) {
    for (int i = 0; i < parser->args_number; i++) {
        int str_length = strlen(parser->options[i].argument_str);
        char str[str_length + 1];
        strcpy(str, parser->options[i].argument_str);
        split_argparse_option(str, &(parser->options[i]), parser->args_number);
    }
}

void UBX_argparse_info(argparse* parser) {
    printf("Usage: %s [OPTION]\n", parser->name);
    for (int i=0;i<parser->args_number;i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == ARGPARSE_OPT_GROUP) {
            continue;
        }
        if (option->short_name) {
            printf("  %s\t",option->short_name);
        } else {
            printf("    \t");
        }
        if (option->long_name) {
            printf("  %s\t",option->long_name);
        } else {
            printf("  \t");
        }
        if (option->help_info) {
            printf("  %s\t",option->help_info);
        }
        printf("\n");
    }
    printf("\n");
}

void UBX_argparse_parse(argparse *parser, int argc, const char **argv) {
    argparse_argument_parse(parser);
    UBX_argparse_info(parser);
    return;
}
