

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * @brief 分割字符串, 返回数组. 
 *
 * @param str 待分割的字符串
 * @param c 分割所使用的字符
 * @param result 返回数组 (call UBX_freeSplitStr to free)
 * @param length 数组长度
 */
void UBX_splitStr(char *str, char c, char ***result, int *length) {
    int n = strlen(str);
    int number = 1;
    int i;
    for (i = 0; i < n; i++) {
        number += c == str[i];
    }
    *length = number;
    *result = (char **)malloc(sizeof(char *) * number);

    char split_str[] = {c};
    char *token;
    token = strtok(str, split_str);
    i = 0;
    while (token != NULL) {
        (*result)[i] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy((*result)[i], token);
        token = strtok(NULL, split_str);
        i++;
    }
}


int UBX_freeSplitStr(char ***result, int length) {
    for (int i = 0; i < length; i++) {
        free((*result)[i]);
    }
    free(*result);
    *result = NULL;
    return 0;
}


/**
 * @brief 去除字符串开头结尾的的单词
 * 
 * @param str
 * @return char* 
 */
char* UBX_trim(char *str) {
    char *new_str;
    while (*str == ' ') {
        str ++;
    }
    int length = strlen(str);
    while (str[length-1] == ' ') {
        length--;
    }
    new_str = (char*)malloc(length+1);
    strncpy(new_str, str, length);
    new_str[length] = '\0';
    return new_str;
}