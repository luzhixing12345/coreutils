

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * @brief 分割字符串, 返回数组. 
 *
 * @param str 待分割的字符串
 * @param c 分割所使用的字符
 * @param result 返回数组 (call XBOX_freeSplitStr to free)
 * @param length 数组长度
 */
void XBOX_splitStr(char *str, char c, char ***result, int *length) {
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


int XBOX_freeSplitStr(char ***result, int length) {
    for (int i = 0; i < length; i++) {
        free((*result)[i]);
    }
    free(*result);
    *result = NULL;
    return 0;
}


/**
 * @brief (原地操作)去除字符串开头结尾的的空格和 ""
 * 
 * @param str
 * @return void
 */
void XBOX_trim(char **str_p) {
    char *new_str;
    char *str = *str_p;
    while (*str == ' ' || *str == '\"') {
        str ++;
    }
    int length = strlen(str);
    while (str[length-1] == ' ' || str[length-1] == '\"') {
        length--;
    }
    new_str = (char*)malloc(length+1);
    strncpy(new_str, str, length);
    new_str[length] = '\0';
    free((char*)*str_p);
    *str_p = (char*)new_str;
}

/**
 * @brief 切片
 * 
 * @param str 
 * @param start 起点index(包含)
 * @param end 终点index(包含), end = -1 表示结尾
 * @return char* 
 */
char* XBOX_splice(const char *str, int start, int end) {
    
    if (end == -1) {
        end = strlen(str)-1;
    }
    char *s = (char*)malloc(sizeof(char) * (end - start+2));
    strncpy(s, str + start, end-start+1);
    s[end-start+1] = '\0';
    return s;
}

/**
 * @brief 找到字符串中一个字符最先出现的位置
 * 
 * @param str 
 * @param c 
 * @return int 未找到返回-1
 */
int XBOX_findChar(const char *str,char c) {
    
    int n = strlen(str);
    for (int i=0;i<n;i++) {
        if (str[i] == c) return i;
    }
    return -1;
}