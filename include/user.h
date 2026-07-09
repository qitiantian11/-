#ifndef USER_H
#define USER_H

#include "common.h"

// 用户结构体，严格对应详细设计的用户表字段
typedef struct {
    int user_id;        // 用户编号，主键自增
    char account[MAX_STR_LEN];  // 登录账号，唯一
    char password[MAX_STR_LEN]; // 登录密码
    char role[20];      // 角色：admin/librarian/reader
    int status;         // 状态：1正常，0停用
} User;

// 全局变量声明（extern 声明，源文件中定义，避免重复定义）
extern User userList[MAX_USER_COUNT];
extern int userCount;

// 数据层函数声明
int loadUsersFromFile();    // 从文件加载用户到内存
int saveUsersToFile();      // 内存用户数据持久化到文件
int findUserByAccount(char *account); // 按账号查找用户下标
int addUser(User newUser);  // 添加新用户到数组

// 业务层函数声明
User* login(char *account, char *password, int *errorCode); // 登录验证
int registerReader(User newUser, int *errorCode);            // 读者注册

#endif // USER_H
