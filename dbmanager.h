#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DBManager
{
public:
    static DBManager &instance();

    /*
     * 打开数据库。
     *
     * 当前版本只连接并检查已有的数据表，
     * 不会创建表、修改表或自动创建管理员。
     */
    bool openDatabase();

    QSqlDatabase database() const;

    /*
     * 注册普通读者。
     */
    bool registerUser(
        const QString &account,
        const QString &password,
        const QString &name,
        const QString &studentNo,
        QString &errorMessage
    );

    /*
     * 登录。
     */
    bool login(
        const QString &account,
        const QString &password,
        QString &role,
        QString &name,
        QString &errorMessage
    );

    /*
     * 新增图书。
     */
    bool addBook(
        const QString &isbn,
        const QString &title,
        const QString &author,
        const QString &publisher,
        const QString &category,
        int totalCount,
        QString &errorMessage
    );

    /*
     * 修改图书。
     */
    bool updateBook(
        int bookId,
        const QString &isbn,
        const QString &title,
        const QString &author,
        const QString &publisher,
        const QString &category,
        int totalCount,
        QString &errorMessage
    );

    /*
     * 删除图书。
     */
    bool deleteBook(
        int bookId,
        QString &errorMessage
    );

    /*
     * 修改普通读者账号状态。
     */
    bool updateReaderStatus(
        int userId,
        const QString &status,
        QString &errorMessage
    );

    /*
     * 将普通读者密码重置为 Library@123。
     */
    bool resetReaderPassword(
        int userId,
        QString &errorMessage
    );

    /*
     * 删除普通读者账号。
     *
     * 删除时会：
     * 1. 恢复未归还图书库存；
     * 2. 删除该读者全部借阅记录；
     * 3. 删除读者账号。
     */
    bool deleteReaderAccount(
        int userId,
        QString &errorMessage
    );

    /*
     * 密码必须：
     * 1. 长度8至30位；
     * 2. 至少一个字母；
     * 3. 至少一个数字；
     * 4. 至少一个特殊符号；
     * 5. 不能包含空格。
     */
    bool isPasswordStrong(
        const QString &password
    ) const;

    QString passwordRuleText() const;

    /*
     * 借书。
     */
    bool borrowBook(
        const QString &readerAccount,
        int bookId,
        int borrowDays,
        QString &errorMessage
    );

    /*
     * 管理员还书。
     */
    bool returnBook(
        int recordId,
        QString &errorMessage,
        double &fine
    );

    /*
     * 普通读者归还自己的图书。
     */
    bool returnBookByReader(
        int recordId,
        const QString &readerAccount,
        QString &errorMessage,
        double &fine
    );

    /*
     * 更新逾期状态和罚款。
     */
    bool syncOverdueStatus(
        QString &errorMessage
    );

private:
    DBManager();
    ~DBManager();

    DBManager(const DBManager &);
    DBManager &operator=(const DBManager &);

    /*
     * 只检查已有表和字段。
     */
    bool checkRequiredTables(
        QString &errorMessage
    );

    QString hashPassword(
        const QString &password
    ) const;

    bool returnBookInternal(
        int recordId,
        const QString &readerAccount,
        bool checkReader,
        QString &errorMessage,
        double &fine
    );

private:
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
