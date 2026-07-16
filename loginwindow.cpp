#include "loginwindow.h"
#include "ui_loginwindow.h"

#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>

#include "dbmanager.h"
#include "mainwindow.h"
#include "registerdialog.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    setWindowTitle(
        QString::fromUtf8("图书馆管理系统 - 登录")
    );

    ui->passwordEdit->setEchoMode(
        QLineEdit::Password
    );

    connect(
        ui->loginButton,
        SIGNAL(clicked()),
        this,
        SLOT(handleLogin())
    );

    connect(
        ui->registerButton,
        SIGNAL(clicked()),
        this,
        SLOT(openRegisterDialog())
    );

    connect(
        ui->passwordEdit,
        SIGNAL(returnPressed()),
        this,
        SLOT(handleLogin())
    );
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::handleLogin()
{
    QString account =
        ui->accountEdit->text().trimmed();

    QString password =
        ui->passwordEdit->text();

    if (account.isEmpty() ||
        password.isEmpty()) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("提示"),
            QString::fromUtf8(
                "请输入账号和密码。"
            )
        );

        return;
    }

    QString role;
    QString name;
    QString errorMessage;

    bool success =
        DBManager::instance().login(
            account,
            password,
            role,
            name,
            errorMessage
        );

    if (!success) {
        QMessageBox::warning(
            this,
            QString::fromUtf8("登录失败"),
            errorMessage
        );

        ui->passwordEdit->clear();
        ui->passwordEdit->setFocus();

        return;
    }

    MainWindow *mainWindow =
        new MainWindow(
            account,
            name,
            role
        );

    mainWindow->setAttribute(
        Qt::WA_DeleteOnClose
    );

    connect(
        mainWindow,
        SIGNAL(logoutRequested()),
        this,
        SLOT(show())
    );

    connect(
        mainWindow,
        SIGNAL(logoutRequested()),
        mainWindow,
        SLOT(close())
    );

    ui->passwordEdit->clear();

    mainWindow->show();
    hide();
}

void LoginWindow::openRegisterDialog()
{
    RegisterDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        ui->accountEdit->setText(
            dialog.registeredAccount()
        );

        ui->passwordEdit->clear();
        ui->passwordEdit->setFocus();
    }
}
