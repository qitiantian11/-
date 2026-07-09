#ifndef COMMON_H
#define COMMON_H

// 通用标准库
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 公共宏定义
#define MAX_STR_LEN 50        // 字符串最大长度
#define MAX_USER_COUNT 1000   // 最大用户数
#define SUCCESS 200            // 操作成功
#define ERROR_PARAM 40001     // 参数错误
#define ERROR_ACCOUNT_NOT_FOUND 40401 // 账号不存在
#define ERROR_PASSWORD_WRONG 40101    // 密码错误
#define ERROR_PERMISSION 40301 // 无权限/账号停用
#define ERROR_FILE 50001      // 文件操作失败

// 数据文件路径统一管理
#define USER_DATA_FILE "data/users.dat"

#endif // COMMON_H
