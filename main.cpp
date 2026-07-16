#include <QApplication>
#include <QMessageBox>

#include "dbmanager.h"
#include "loginwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 初始化并连接 MySQL 数据库
    if (!DBManager::instance().openDatabase()) {
        QMessageBox::critical(
            0,
            QString::fromUtf8("数据库连接失败"),
            QString::fromUtf8(
                "无法连接 MySQL 数据库。\n\n"
                "请检查：\n"
                "1. MySQL80 服务是否启动\n"
                "2. 系统 DSN 是否为 LibraryMySQL\n"
                "3. 数据库用户名和密码是否正确\n"
                "4. library_system 数据库是否存在"
            )
        );

        return 1;
    }

    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}
