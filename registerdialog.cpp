#include "registerdialog.h"
#include "ui_registerdialog.h"

#include <QLineEdit>
#include <QMessageBox>

#include "dbmanager.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    setWindowTitle(
        QString::fromUtf8(
            "读者注册"
        )
    );

    ui->passwordEdit->setEchoMode(
        QLineEdit::Password
    );

    ui->confirmPasswordEdit->setEchoMode(
        QLineEdit::Password
    );

    ui->passwordEdit->setPlaceholderText(
        QString::fromUtf8(
            "8至30位，包含字母、数字和特殊符号"
        )
    );

    ui->confirmPasswordEdit->setPlaceholderText(
        QString::fromUtf8(
            "请再次输入密码"
        )
    );

    ui->passwordEdit->setToolTip(
        DBManager::instance()
            .passwordRuleText()
    );

    ui->confirmPasswordEdit->setToolTip(
        DBManager::instance()
            .passwordRuleText()
    );

    connect(
        ui->registerButton,
        SIGNAL(clicked()),
        this,
        SLOT(handleRegister())
    );

    connect(
        ui->cancelButton,
        SIGNAL(clicked()),
        this,
        SLOT(reject())
    );

    connect(
        ui->confirmPasswordEdit,
        SIGNAL(returnPressed()),
        this,
        SLOT(handleRegister())
    );
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

QString RegisterDialog::registeredAccount() const
{
    return m_registeredAccount;
}

void RegisterDialog::handleRegister()
{
    const QString account =
        ui->accountEdit
            ->text()
            .trimmed();

    const QString password =
        ui->passwordEdit
            ->text();

    const QString confirmPassword =
        ui->confirmPasswordEdit
            ->text();

    const QString name =
        ui->nameEdit
            ->text()
            .trimmed();

    const QString studentNo =
        ui->studentNoEdit
            ->text()
            .trimmed();

    if (account.isEmpty() ||
        password.isEmpty() ||
        confirmPassword.isEmpty() ||
        name.isEmpty() ||
        studentNo.isEmpty()) {

        QMessageBox::warning(
            this,
            QString::fromUtf8(
                "提示"
            ),
            QString::fromUtf8(
                "所有信息都必须填写。"
            )
        );

        return;
    }

    if (account.length() < 4 ||
        account.length() > 30) {

        QMessageBox::warning(
            this,
            QString::fromUtf8(
                "账号格式不正确"
            ),
            QString::fromUtf8(
                "账号长度必须为4至30位。"
            )
        );

        ui->accountEdit->setFocus();
        ui->accountEdit->selectAll();

        return;
    }

    if (!DBManager::instance()
             .isPasswordStrong(password)) {

        QMessageBox::warning(
            this,
            QString::fromUtf8(
                "密码格式不正确"
            ),
            DBManager::instance()
                .passwordRuleText()
        );

        ui->passwordEdit->setFocus();
        ui->passwordEdit->selectAll();

        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(
            this,
            QString::fromUtf8(
                "提示"
            ),
            QString::fromUtf8(
                "两次输入的密码不一致。"
            )
        );

        ui->confirmPasswordEdit->clear();
        ui->confirmPasswordEdit->setFocus();

        return;
    }

    QString errorMessage;

    const bool success =
        DBManager::instance()
            .registerUser(
                account,
                password,
                name,
                studentNo,
                errorMessage
            );

    if (!success) {
        QMessageBox::warning(
            this,
            QString::fromUtf8(
                "注册失败"
            ),
            errorMessage
        );

        return;
    }

    m_registeredAccount =
        account;

    QMessageBox::information(
        this,
        QString::fromUtf8(
            "注册成功"
        ),
        QString::fromUtf8(
            "账号注册成功，请返回登录。\n\n"
            "密码必须妥善保管。"
        )
    );

    accept();
}
