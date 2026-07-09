#include "menu.h"

// 显示登录主菜单，返回用户选择的选项
int showLoginMenu() {
    int choice;
    printf("\n====================\n");
    printf("  图书馆管理系统\n");
    printf("====================\n");
    printf("1. 用户登录\n");
    printf("2. 读者注册\n");
    printf("0. 退出系统\n");
    printf("====================\n");
    printf("请输入选项：");
    scanf("%d", &choice);
    // 清理输入缓冲区残留的换行符，避免后续输入异常
    while (getchar() != '\n');
    return choice;
}

// 处理登录逻辑
void handleLogin() {
    char account[MAX_STR_LEN];
    char password[MAX_STR_LEN];
    int errorCode;

    printf("\n--- 用户登录 ---\n");
    printf("请输入账号：");
    scanf("%s", account);
    printf("请输入密码：");
    scanf("%s", password);
    while (getchar() != '\n');

    User *currentUser = login(account, password, &errorCode);

    if (currentUser != NULL && errorCode == SUCCESS) {
        printf("登录成功！欢迎您，%s\n", currentUser->account);
        // 进入对应角色的主菜单
        showMainMenu(currentUser);
    } else {
        switch (errorCode) {
            case ERROR_PARAM:
                printf("登录失败：账号和密码不能为空\n");
                break;
            case ERROR_ACCOUNT_NOT_FOUND:
            case ERROR_PASSWORD_WRONG:
                printf("登录失败：账号或密码错误\n");
                break;
            case ERROR_PERMISSION:
                printf("登录失败：账号已停用，请联系管理员\n");
                break;
            default:
                printf("登录失败：未知错误\n");
        }
    }
}

// 处理读者注册逻辑
void handleRegister() {
    User newUser = {0};
    char confirmPwd[MAX_STR_LEN];
    int errorCode;

    printf("\n--- 读者注册 ---\n");
    printf("请设置登录账号：");
    scanf("%s", newUser.account);
    printf("请设置登录密码：");
    scanf("%s", newUser.password);
    printf("请确认密码：");
    scanf("%s", confirmPwd);
    while (getchar() != '\n');

    // 两次密码一致性校验
    if (strcmp(newUser.password, confirmPwd) != 0) {
        printf("注册失败：两次输入的密码不一致\n");
        return;
    }

    int userId = registerReader(newUser, &errorCode);

    if (userId > 0 && errorCode == SUCCESS) {
        printf("注册成功！您的用户编号为 %d，请返回登录\n", userId);
    } else {
        if (errorCode == ERROR_PARAM) {
            printf("注册失败：该账号已被注册\n");
        } else {
            printf("注册失败：系统错误，请重试\n");
        }
    }
}

// 登录成功后，根据角色显示对应主菜单（预留后续功能入口）
void showMainMenu(User *currentUser) {
    int choice;
    while (1) {
        printf("\n====================\n");
        if (strcmp(currentUser->role, "admin") == 0) {
            printf("  管理员主菜单\n");
            printf("1. 用户管理\n");
            printf("2. 图书管理\n");
            printf("3. 读者管理\n");
            printf("4. 借阅统计\n");
        } else if (strcmp(currentUser->role, "librarian") == 0) {
            printf("  图书管理员主菜单\n");
            printf("1. 图书管理\n");
            printf("2. 读者管理\n");
            printf("3. 借书还书\n");
        } else {
            printf("  读者主菜单\n");
            printf("1. 馆藏图书查询\n");
            printf("2. 我的借阅记录\n");
        }
        printf("0. 退出登录\n");
        printf("====================\n");
        printf("请输入选项：");
        scanf("%d", &choice);
        while (getchar() != '\n');

        if (choice == 0) {
            printf("已退出登录，返回登录页\n");
            break;
        } else {
            printf("功能开发中，敬请期待...\n");
        }
    }
}
