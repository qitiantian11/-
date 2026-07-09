#ifndef MENU_H
#define MENU_H

#include "common.h"
#include "user.h"

// 菜单交互函数声明
int showLoginMenu();       // 显示登录主菜单
void handleLogin();        // 处理登录逻辑
void handleRegister();     // 处理注册逻辑
void showMainMenu(User *currentUser); // 登录后角色主菜单

#endif // MENU_H
