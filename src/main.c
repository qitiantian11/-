#include "common.h"
#include "menu.h"



int main() {
    printf("正在加载系统数据...\n");
    // 程序启动时，从文件加载所有用户数据
    int loadRet = loadUsersFromFile();
    if (loadRet < 0) {
        printf("数据加载失败，程序退出\n");
        return 1;
    }
    printf("数据加载完成，共 %d 个用户\n", userCount);

    // 主循环：登录菜单交互
    int choice;
    while (1) {
        choice = showLoginMenu();
        switch (choice) {
            case 1:
                handleLogin();
                break;
            case 2:
                handleRegister();
                break;
            case 0:
                printf("正在保存数据...\n");
                saveUsersToFile();
                printf("数据已保存，感谢使用！\n");
                return 0;
            default:
                printf("输入无效，请重新选择\n");
        }
    }
}
