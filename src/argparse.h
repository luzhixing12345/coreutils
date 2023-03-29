/**
 * Copyright (C) 2012-2015 Yecheng Fu <cofyc.jackson at gmail dot com>
 * All rights reserved.
 *
 * Use of this source code is governed by a MIT-style license that can be found
 * in the LICENSE file.
 */
#pragma once

// #include <stdint.h>
#include <ctype.h>

#include "define.h"
#include "string.h"

enum argparse_option_type {
    ARGPARSE_OPT_END,
    ARGPARSE_OPT_BOOLEAN,
    ARGPARSE_OPT_INTEGER,
    ARGPARSE_OPT_STRING,
    ARGPARSE_OPT_INT_GROUP,
    ARGPARSE_OPT_STR_GROUP
};

enum argparse_flag {
    UBX_ARGPARSE_IGNORE_WARNING = 1,        // 忽略警告
    UBX_ARGPARSE_ENABLE_STICK = 1 << 1,     // 允许参数粘连 -O1 -Iinclude/
    UBX_ARGPARSE_ENABLE_EQUAL = 1 << 2,     // 允许参数等号 -i=123
    UBX_ARGPARSE_ENABLE_MULTI = 1 << 3,     // 允许多个分离参数 -D __KERNEL__ -D __GNU__
    UBX_ARGPARSE_ACCEPT_MORE = 1 << 4,      // 允许参数过多
    UBX_ARGPARSE_ENABLE_ARG_STICK = 1 << 5  // 允许boolean类型参数粘连
};

typedef struct {
    enum argparse_option_type type;
    void *p;
    char *argument_str;
    char *short_name;
    char *long_name;
    char *name;
    char *help_info;
    char *value;
    int must;
    int match;
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
    int flag;
} UBX_argparse;

// built-in option macros

#define UBX_ARG_BOOLEAN(a, ...) \
    { ARGPARSE_OPT_BOOLEAN, a, #__VA_ARGS__ }
#define UBX_ARG_INT(a, ...) \
    { ARGPARSE_OPT_INTEGER, a, #__VA_ARGS__ }
#define UBX_ARG_STR(a, ...) \
    { ARGPARSE_OPT_STRING, a, #__VA_ARGS__ }
#define UBX_ARG_INT_GROUP(a, ...) \
    { ARGPARSE_OPT_INT_GROUP, a, #__VA_ARGS__ }
#define UBX_ARG_STR_GROUP(a, ...) \
    { ARGPARSE_OPT_STR_GROUP, a, #__VA_ARGS__ }
#define UBX_ARG_END(...) \
    { ARGPARSE_OPT_END, #__VA_ARGS__ }

void UBX_argparse_init(UBX_argparse *parser, argparse_option *options, int flag) {
    memset(parser, 0, sizeof(*parser));
    parser->name = NULL;
    parser->description = NULL;
    parser->epilog = NULL;
    parser->options = options;
    parser->args_number = 0;
    parser->flag = flag;
    while (options->type != ARGPARSE_OPT_END) {
        options++;
        parser->args_number++;
    }
    for (int i = 0; i < parser->args_number; i++) {
        parser->options[i].short_name = NULL;
        parser->options[i].long_name = NULL;
        parser->options[i].name = NULL;
        parser->options[i].help_info = NULL;
        parser->options[i].value = NULL;
        parser->options[i].must = 0;
        parser->options[i].match = 0;
    }
    return;
}

void UBX_argparse_describe(UBX_argparse *parser, const char *name, const char *description,
                           const char *epilog) {
    parser->name = name;
    parser->description = description;
    parser->epilog = epilog;
    return;
}

void UBX_free_argparse(UBX_argparse *parser) {
    // fprintf(stderr, "argument parse error, free options\n");
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->short_name) {
            free(option->short_name);
        }
        if (option->long_name) {
            free(option->long_name);
        }
        if (option->name) {
            free(option->name);
        }
        if (option->help_info) {
            free(option->help_info);
        }
        if (option->value) {
            free(option->value);
        }
    }
    // fprintf(stderr, "finished free options\n");
}

void value_pass(UBX_argparse *parser, argparse_option *option);

int parse_optionstr(argparse_option *option) {
    char *str = option->argument_str;
    int length = strlen(str);
    int p = 0;
    int match_flag = 0;
    // printf("%s\n",str);
    for (int i = 0; i < length; i++) {
        if (str[i] == '[') {
            if (match_flag) {
                fprintf(stderr, "miss ] in %s\n", str);
                return UBX_FORMAT_ERROR;
            }
            p = i + 1;
            match_flag = 1;
        } else if (str[i] == ']') {
            if (!match_flag) {
                fprintf(stderr, "mismatch between [ and ] in %s\n", str);
                return UBX_FORMAT_ERROR;
            }
            match_flag = 0;
            char *argument = UBX_splice(str, p, i - 1);
            // printf("argument = [%s] [%d:%d]\n", argument, p ,i-1);
            if (i - p < 2) {
                fprintf(stderr, "argument define [%s] too short in %s\n", argument, str);
                free(argument);
                return UBX_FORMAT_ERROR;
            }
            // 给 options 完善选项
            int argument_length = strlen(argument);
            if (argument[0] == '-') {
                // --long_name
                if (argument[1] == '-') {
                    if (argument_length < 4) {
                        fprintf(stderr, "[%s] should be at least 4 characters\n", argument);
                        free(argument);
                        return UBX_FORMAT_ERROR;
                    } else {
                        option->long_name = argument;
                    }
                } else {
                    // -short_name
                    if (argument_length != 2) {
                        fprintf(stderr, "[%s] must be 2 characters for short name\n", argument);
                        free(argument);
                        return UBX_FORMAT_ERROR;
                    } else {
                        option->short_name = argument;
                    }
                }
            } else {
                // 按=分割
                int p = -1;
                for (int j = 0; j < argument_length; j++) {
                    if (argument[j] == '=') {
                        p = j;
                        break;
                    }
                }
                if (p == -1) {
                    fprintf(stderr, "miss = in argument [%s] in %s\n", argument, str);
                    free(argument);
                    return UBX_FORMAT_ERROR;
                }
                char *key = UBX_splice(argument, 0, p - 1);
                UBX_trim(&key);
                // printf("[%s][%s][%d]\n",argument, key, p-1);
                char *value = UBX_splice(argument, p + 1, argument_length - 1);
                UBX_trim(&value);
                // printf("[%s] -> [%s]:[%s]\n",argument, key, value);
                free(argument);
                if (!strcmp(key, "help")) {
                    option->help_info = value;
                } else if (!strcmp(key, "name")) {
                    option->name = value;
                }
                // else if (!strcmp(key, "flag")) {
                //     if (!strcmp(value, "UBX_MUST")) {
                //         free(value);
                //         option->must = 1;
                //     } else {
                //         fprintf(stderr, "unknown flag [flag=%s] in %s\n", value, str);
                //         free(key);
                //         return UBX_FORMAT_ERROR;
                //     }
                // }
                else {
                    fprintf(stderr, "unsupported argument [%s=%s] in %s\n", key, value, str);
                    free(key);
                    free(value);
                    return UBX_FORMAT_ERROR;
                }
                free(key);
            }
            // printf("%s\n", argument);
            // free(argument);
        }
    }
    return 0;
}

void argparse_option_parse(UBX_argparse *parser) {
    for (int i = 0; i < parser->args_number; i++) {
        if (parse_optionstr(&(parser->options[i]))) {
            fprintf(stderr, "parser init failed\n");
            UBX_free_argparse(parser);
            exit(UBX_FORMAT_ERROR);
        }
    }
}

void UBX_argparse_info(UBX_argparse *parser) {
    printf("Usage: %s ", parser->name);
    int counter = 0;
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == ARGPARSE_OPT_STR_GROUP || option->type == ARGPARSE_OPT_INT_GROUP) {
            printf("[%s] ", option->name);
            counter++;
        }
    }
    if (counter == parser->args_number) {
        printf("\n");
        return;
    } else {
        printf("[OPTION]...\n");
    }
    if (parser->description) {
        printf("%s\n", parser->description);
    }
    printf("\n");
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == ARGPARSE_OPT_INT_GROUP || option->type == ARGPARSE_OPT_STR_GROUP) {
            continue;
        }
        if (option->short_name) {
            printf("  %s", option->short_name);
        }
        printf("\t");

        if (option->long_name) {
            printf("%s", option->long_name);
        }
        int n = strlen(option->long_name);
        if (n < 8) {
            printf("\t\t");
        } else if (n >= 8 && n < 16) {
            printf("\t");
        }
        if (option->help_info) {
            printf("%s", option->help_info);
        }
        printf("\n");
    }
    if (parser->epilog) {
        printf("%s\n", parser->epilog);
    }
}

/**
 * @brief 判断长参数是否匹配, 并修改 option->match
 *
 * @param parser
 * @param str
 * @return argparse_option 返回匹配的option, 否则返回 NULL
 */
argparse_option *check_argparse_loptions(UBX_argparse *parser, const char *str) {
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == ARGPARSE_OPT_INT_GROUP || option->type == ARGPARSE_OPT_STR_GROUP) {
            continue;
        }
        if (!strcmp(option->long_name, str)) {
            // printf("matched %s\n", option->long_name);
            return option;
        }
    }
    return NULL;
}

/**
 * @brief 判断短参数是否匹配, 并修改 option->match
 *
 * @param parser
 * @param str
 * @return argparse_option* 返回匹配的option, 否则返回 NULL
 */
argparse_option *check_argparse_soptions(UBX_argparse *parser, const char *str) {
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == ARGPARSE_OPT_INT_GROUP || option->type == ARGPARSE_OPT_STR_GROUP) {
            continue;
        }
        if (!strcmp(option->short_name, str)) {
            // printf("matched %s\n", option->short_name);
            return option;
        }
    }
    return NULL;
}

/**
 * @brief 解析组, 修改match和value
 *
 * @param parser
 * @param str
 * @return int 如果已经没有可解析的组则出现错误,返回1
 */
int check_argparse_groups(UBX_argparse *parser, const char *str) {
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->type == ARGPARSE_OPT_INT_GROUP || option->type == ARGPARSE_OPT_STR_GROUP) {
            if (!option->match) {
                if (option->value) {
                    free(option->value);
                }
                option->value = (char *)malloc(sizeof(char) * (strlen(str) + 1));
                strcpy(option->value, str);
                // printf("matched [%s] for group [%s]\n", option->value, option->name);
                value_pass(parser, option);
                return 0;
            }
        }
    }
    return 1;
}

/**
 * @brief 传递解析后的参数
 *
 * @param option
 * @param enable_multi 是否允许多个分离参数
 */
void value_pass(UBX_argparse *parser, argparse_option *option) {
    int enable_multi = parser->flag & UBX_ARGPARSE_ENABLE_MULTI;
    // 不允许多个
    if (!enable_multi) {
        if (option->match && !(parser->flag & UBX_ARGPARSE_IGNORE_WARNING)) {
            fprintf(stderr, "Warning: multi argument detected for [%s]\n", option->name);
        }
        if (option->type == ARGPARSE_OPT_STRING || option->type == ARGPARSE_OPT_STR_GROUP) {
            *(char **)option->p = (char *)malloc(strlen(option->value));
            strcpy(*(char **)option->p, option->value);
            option->match = 1;
        } else if (option->type == ARGPARSE_OPT_INTEGER || option->type == ARGPARSE_OPT_INT_GROUP) {
            int value = 0;
            char *temp = option->value;
            while (*temp != '\0') {
                if (*temp < '0' || *temp > '9') {
                    fprintf(
                        stderr, "Error: argument assign to be int but get [%s]\n", option->value);
                    UBX_free_argparse(parser);
                    exit(UBX_FORMAT_ERROR);
                }
                value = value * 10 + (*temp) - '0';
                temp++;
            }
            *(int *)option->p = value;
            option->match = 1;
        }
    } else {
        // 多个匹配的情况
        // -D __GNU__ -D __KERNEL
        int match_number = ++option->match;
        if (option->type == ARGPARSE_OPT_STRING || option->type == ARGPARSE_OPT_STR_GROUP) {
            char **new_p = (char **)realloc(*(char ***)option->p, sizeof(char *) * match_number);

            if (*(char ***)option->p && new_p != (*(char ***)option->p)) {
                for (int i = 0; i < match_number - 1; i++) {
                    new_p[i] = (*(char ***)option->p)[i];
                }
                free(*(char ***)option->p);
            }
            *(char ***)option->p = new_p;
            char *value_str = UBX_splice(option->value, 0, -1);
            (*(char ***)option->p)[match_number - 1] = value_str;

        } else if (option->type == ARGPARSE_OPT_INTEGER || option->type == ARGPARSE_OPT_INT_GROUP) {
            int value = 0;
            char *temp = option->value;
            while (*temp != '\0') {
                if (*temp < '0' || *temp > '9') {
                    fprintf(
                        stderr, "Error: argument assign to be int but get [%s]\n", option->value);
                    UBX_free_argparse(parser);
                    exit(UBX_FORMAT_ERROR);
                }
                value = value * 10 + (*temp) - '0';
                temp++;
            }
            int *new_p = (int *)realloc(*(int **)option->p, sizeof(int) * match_number);
            if (*(char ***)option->p && new_p != (*(int **)option->p)) {
                for (int i = 0; i < match_number - 1; i++) {
                    new_p[i] = (*(int **)option->p)[i];
                }
                free(*(int **)option->p);
            }
            (*(int **)option->p) = new_p;
            (*(int **)option->p)[match_number - 1] = value;
        }
    }
}

void argparse_parse_argv(UBX_argparse *parser, int argc, const char **argv) {
    for (int i = 1; i < argc; i++) {
        int argv_length = strlen(argv[i]);
        if (argv_length >= 2) {
            if (argv[i][0] == '-') {
                // --long_name
                argparse_option *option;
                if (argv[i][1] == '-') {
                    option = check_argparse_loptions(parser, argv[i]);
                } else {
                    option = check_argparse_soptions(parser, argv[i]);
                }
                if (option == NULL) {
                    if (parser->flag & UBX_ARGPARSE_ENABLE_ARG_STICK && argv[i][1] != '-') {
                        char s[3] = {'-', '0', '\0'};
                        int n = strlen(argv[i]);
                        for (int j = 1; j < n; j++) {
                            s[1] = argv[i][j];
                            option = check_argparse_soptions(parser, s);
                            if (option == NULL) {
                                fprintf(stderr,
                                        "Error: no match options for [%c] in [%s]\n",
                                        argv[i][j],
                                        argv[i]);
                                UBX_free_argparse(parser);
                                exit(UBX_FORMAT_ERROR);
                            } else {
                                if (option->type != ARGPARSE_OPT_BOOLEAN) {
                                    fprintf(stderr,
                                            "Error: only boolean type should be sticky for [%c] in "
                                            "[%s]\n",
                                            argv[i][j],
                                            argv[i]);
                                    UBX_free_argparse(parser);
                                    exit(UBX_FORMAT_ERROR);
                                } else {
                                    option->match = 1;
                                }
                            }
                        }
                        continue;
                    }

                    if (parser->flag & UBX_ARGPARSE_ENABLE_EQUAL) {
                        int pos = UBX_findChar(argv[i], '=');
                        if (pos != -1 && pos == 2) {
                            char *short_name = UBX_splice(argv[i], 0, 1);
                            option = check_argparse_soptions(parser, short_name);
                            free(short_name);
                            if (option) {
                                char *value = UBX_splice(argv[i], 3, -1);
                                option->value = value;
                                value_pass(parser, option);
                                continue;
                            }
                        }
                    }
                    if (parser->flag & UBX_ARGPARSE_ENABLE_STICK) {
                        char *short_name = UBX_splice(argv[i], 0, 1);
                        option = check_argparse_soptions(parser, short_name);
                        free(short_name);
                        if (option) {
                            char *value = UBX_splice(argv[i], 2, -1);
                            option->value = value;
                            value_pass(parser, option);
                            continue;
                        }
                    }
                    fprintf(stderr, "Error: no match options for [%s]\n", argv[i]);
                    UBX_free_argparse(parser);
                    exit(UBX_FORMAT_ERROR);

                } else {
                    if (option->type == ARGPARSE_OPT_BOOLEAN) {
                        option->match = 1;
                        continue;
                    }
                    // 正确解析, 读取下一个参数
                    if (i == argc - 1) {
                        fprintf(
                            stderr, "Error: option [%s] needs one argument\n", option->long_name);
                        UBX_free_argparse(parser);
                        exit(UBX_FORMAT_ERROR);
                    }
                    if (argv[i + 1][0] == '-' && !(parser->flag & UBX_ARGPARSE_IGNORE_WARNING)) {
                        fprintf(stderr,
                                "Warning: [%s] will be passed as the argument for [%s]\n",
                                argv[i + 1],
                                argv[i]);
                    }
                    if (option->value) {
                        free(option->value);
                    }
                    option->value = (char *)malloc(sizeof(char) * (strlen(argv[i + 1]) + 1));
                    strcpy(option->value, argv[i + 1]);
                    value_pass(parser, option);
                    // printf("matched [%s]:[%s]\n", option->long_name, argv[i + 1]);
                    i++;
                }
                continue;
            }
        }
        // 当作正常参数传入
        if (check_argparse_groups(parser, argv[i])) {
            if (!(parser->flag & UBX_ARGPARSE_ACCEPT_MORE)) {
                fprintf(stderr, "Error: no groups left for [%s] in options\n", argv[i]);
                UBX_free_argparse(parser);
                exit(UBX_FORMAT_ERROR);
            }
        }
    }

    // UBX_MUST 检查必传参数
    // for (int i = 0; i < parser->args_number; i++) {
    //     argparse_option *option = &(parser->options[i]);
    //     if (option->must) {
    //         if (!option->match) {
    //             fprintf(stderr, "Error: argument option [%s] is must\n", option->name);
    //             UBX_free_argparse(parser);
    //             exit(UBX_FORMAT_ERROR);
    //         }
    //     }
    // }
}

int check_valid_character(const char *str) {
    int length = strlen(str);
    for (int i = 0; i < length; i++) {
        if (!(islower(str[i]) || str[i] == '_' || str[i] == '-')) {
            return 1;
        }
    }
    return 0;
}

void check_valid_options(UBX_argparse *parser) {
    // group 不重名

    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (option->long_name) {
            char *l_name = UBX_splice(option->long_name, 2, -1);
            char *p = l_name;

            // long_name 合法性 -> 小写 - _
            if (check_valid_character(p)) {
                fprintf(stderr, "Error: only [a-z_-]are legal characters instead of [%s]\n", p);
                free(p);
                exit(UBX_FORMAT_ERROR);
            }
            if (option->name) {
                // long_name 和 name 必须有一个 -> 同时存在的时候必须统一
                if (strcmp(option->name, l_name)) {
                    fprintf(stderr,
                            "long_name --[%s] and name [%s] must be the same\n",
                            l_name,
                            option->name);
                    free(l_name);
                    UBX_free_argparse(parser);
                    exit(UBX_FORMAT_ERROR);
                }
            } else {
                // 将 long_name 赋值给 name
                // printf("name -> [%s]\n", l_name);
                option->name = l_name;
            }
        } else {
            // 没有long_name 的时候必须有 name
            if (!option->name) {
                fprintf(stderr, "long_name and name have at least one\n");
                UBX_free_argparse(parser);
                exit(UBX_FORMAT_ERROR);
            }
        }
    }

    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option1 = &(parser->options[i]);
        for (int j = i + 1; j < parser->args_number; j++) {
            argparse_option *option2 = &(parser->options[j]);
            if (!strcmp(option1->name, option2->name)) {
                fprintf(stderr, "Error: options have the same name [%s]\n", option1->name);
                UBX_free_argparse(parser);
                exit(UBX_FORMAT_ERROR);
            }
            if (option1->long_name && option2->long_name) {
                if (!strcmp(option1->long_name, option2->long_name)) {
                    fprintf(stderr,
                            "Error: options have the same long_name [%s]\n",
                            option1->long_name);
                    UBX_free_argparse(parser);
                    exit(UBX_FORMAT_ERROR);
                }
            }
            if (option1->short_name && option2->short_name) {
                if (!strcmp(option1->short_name, option2->short_name)) {
                    fprintf(stderr,
                            "Error: options have the same short_name [%s]\n",
                            option1->short_name);
                    UBX_free_argparse(parser);
                    exit(UBX_FORMAT_ERROR);
                }
            }
        }
    }

    if (parser->flag & UBX_ARGPARSE_ENABLE_ARG_STICK &&
        ((parser->flag & UBX_ARGPARSE_ENABLE_EQUAL) ||
         (parser->flag & UBX_ARGPARSE_ENABLE_STICK))) {
        fprintf(
            stderr,
            "Error: flag collasp for UBX_ARGPARSE_ENABLE_EQUAL with UBX_ARGPARSE_ENABLE_EQUAL or "
            "UBX_ARGPARSE_ENABLE_STICK\n");
        UBX_free_argparse(parser);
        exit(UBX_FORMAT_ERROR);
    }
}

/**
 * @brief 参数是否匹配
 *
 * @param parser
 * @param name
 * @return int 如果未匹配返回0; 如果匹配,返回值为匹配的个数
 */
int UBX_ismatch(UBX_argparse *parser, char *name) {
    for (int i = 0; i < parser->args_number; i++) {
        argparse_option *option = &(parser->options[i]);
        if (!strcmp(option->name, name)) {
            return option->match;
        }
    }
    fprintf(stderr, "no matched name in options for [%s]\n", name);
    return 0;
}

/**
 * @brief 解析命令行参数
 *
 * @param parser
 * @param argc
 * @param argv
 */
void UBX_argparse_parse(UBX_argparse *parser, int argc, const char **argv) {
    argparse_option_parse(parser);
    check_valid_options(parser);
    argparse_parse_argv(parser, argc, argv);
    return;
}
