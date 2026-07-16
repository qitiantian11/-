#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = 0);
    ~RegisterDialog();

    QString registeredAccount() const;

private slots:
    void handleRegister();

private:
    Ui::RegisterDialog *ui;
    QString m_registeredAccount;
};

#endif // REGISTERDIALOG_H
