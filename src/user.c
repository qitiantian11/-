#include "user.h"

// 定义全局变量（头文件中用 extern 声明，仅在此处定义一次）
User userList[MAX_USER_COUNT];
int userCount = 0;
// 从文件加载所有用户，返回加载的用户数，失败返回-1
int loadUsersFromFile() {
    FILE *fp = fopen(USER_DATA_FILE, "r");
    // 文件不存在时，创建空文件并初始化默认管理员
    if (fp == NULL) {
        fp = fopen(USER_DATA_FILE, "w");
        if (fp == NULL) return ERROR_FILE;
        fclose(fp);
        userCount = 0;

        // 初始化默认管理员：账号 admin，密码 123456
        User admin = {0};
        admin.user_id = 1;
        strcpy(admin.account, "admin");
        strcpy(admin.password, "123456");
        strcpy(admin.role, "admin");
        admin.status = 1;
        addUser(admin);
        saveUsersToFile();
        return userCount;
    }

    userCount = 0;
    char line[256];
    // 逐行读取，用 | 分隔字段：user_id|account|password|role|status
    while (fgets(line, sizeof(line), fp) != NULL) {
        // 跳过空行和换行符
        if (strlen(line) <= 1) continue;
        // 去掉行尾的换行符
        line[strcspn(line, "\n")] = 0;

        User u = {0};
        char *token = strtok(line, "|");
        if (token == NULL) continue;
        u.user_id = atoi(token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        strcpy(u.account, token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        strcpy(u.password, token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        strcpy(u.role, token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        u.status = atoi(token);

        userList[userCount++] = u;
    }
    fclose(fp);
    return userCount;
}
// 把内存中所有用户保存到文件，成功返回 SUCCESS，失败返回 ERROR_FILE
int saveUsersToFile() {
    FILE *fp = fopen(USER_DATA_FILE, "w");
    if (fp == NULL) return ERROR_FILE;

    for (int i = 0; i < userCount; i++) {
        fprintf(fp, "%d|%s|%s|%s|%d\n",
                userList[i].user_id,
                userList[i].account,
                userList[i].password,
                userList[i].role,
                userList[i].status);
    }
    fclose(fp);
    return SUCCESS;
}
// 根据账号查找用户，返回数组下标，找不到返回 -1
int findUserByAccount(char *account) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userList[i].account, account) == 0) {
            return i;
        }
    }
    return -1;
}

// 添加新用户，自动生成自增ID，成功返回用户ID，失败返回 -1
int addUser(User newUser) {
    if (userCount >= MAX_USER_COUNT) return -1;
    // 账号唯一性校验
    if (findUserByAccount(newUser.account) != -1) return -1;

    // 计算最大ID，实现自增
    int maxId = 0;
    for (int i = 0; i < userCount; i++) {
        if (userList[i].user_id > maxId) {
            maxId = userList[i].user_id;
        }
    }
    newUser.user_id = maxId + 1;

    userList[userCount++] = newUser;
    return newUser.user_id;
}
// 登录验证：成功返回用户结构体指针，失败返回 NULL，errorCode 带回错误码
User* login(char *account, char *password, int *errorCode) {
    // 参数非空校验
    if (account == NULL || password == NULL ||
        strlen(account) == 0 || strlen(password) == 0) {
        *errorCode = ERROR_PARAM;
        return NULL;
    }

    int index = findUserByAccount(account);
    if (index == -1) {
        *errorCode = ERROR_ACCOUNT_NOT_FOUND;
        return NULL;
    }

    // 校验账号状态
    if (userList[index].status == 0) {
        *errorCode = ERROR_PERMISSION;
        return NULL;
    }

    // 校验密码
    if (strcmp(userList[index].password, password) != 0) {
        *errorCode = ERROR_PASSWORD_WRONG;
        return NULL;
    }

    *errorCode = SUCCESS;
    return &userList[index];
}
// 读者注册：成功返回用户ID，失败返回 -1，errorCode 带回错误码
int registerReader(User newUser, int *errorCode) {
    // 参数校验
    if (strlen(newUser.account) == 0 || strlen(newUser.password) == 0) {
        *errorCode = ERROR_PARAM;
        return -1;
    }

    // 账号重复校验
    if (findUserByAccount(newUser.account) != -1) {
        *errorCode = ERROR_PARAM;
        return -1;
    }

    // 固定注册角色为读者，状态默认为正常
    strcpy(newUser.role, "reader");
    newUser.status = 1;

    int userId = addUser(newUser);
    if (userId == -1) {
        *errorCode = ERROR_FILE;
        return -1;
    }

    // 持久化保存到文件
    int saveRet = saveUsersToFile();
    if (saveRet != SUCCESS) {
        *errorCode = ERROR_FILE;
        return -1;
    }

    *errorCode = SUCCESS;
    return userId;
}
