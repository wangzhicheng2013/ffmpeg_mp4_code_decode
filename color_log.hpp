#pragma once
#include <stdio.h>
#define TAG "ColorTag"

// 红色
#define LOG_E(fmt, ...)                                               \
    printf("\033[1m\033[40;31m [E][%d](%s|%d)" fmt " \033[0m\n", \
           gettid(), __func__, __LINE__, ##__VA_ARGS__)
// 黄色
#define LOG_W(fmt, ...)                                               \
    printf("\033[1m\033[40;33m [W][%d](%s|%d)" fmt " \033[0m\n", \
           gettid(), __func__, __LINE__, ##__VA_ARGS__)
// 白色
#define LOG_I(fmt, ...)                                               \
    printf("\033[1m\033[40;37m [I][%d](%s|%d)" fmt " \033[0m\n", \
           gettid(), __func__, __LINE__, ##__VA_ARGS__)
// 深绿色
#define LOG_D(fmt, ...)                                               \
    printf("\033[1m\033[40;36m [D][%d](%s|%d)" fmt " \033[0m\n", \
           gettid(), __func__, __LINE__, ##__VA_ARGS__)