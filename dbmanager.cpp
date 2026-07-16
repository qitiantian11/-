#include "dbmanager.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

DBManager &DBManager::instance()
{
    static DBManager manager;
    return manager;
}

DBManager::DBManager()
{
}

DBManager::~DBManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DBManager::openDatabase()
{
    const QString connectionName =
        "library_mysql_connection";

    if (QSqlDatabase::contains(connectionName)) {
        m_db =
            QSqlDatabase::database(
                connectionName
            );
    } else {
        m_db =
            QSqlDatabase::addDatabase(
                "QODBC",
                connectionName
            );
    }

    /*
     * 必须和32位ODBC系统DSN名称一致。
     */
    m_db.setDatabaseName(
        "LibraryMySQL"
    );

    /*
     * 改成实际数据库账号。
     */
    m_db.setUserName(
        "root"
    );

    /*
     * 改成实际数据库密码。
     */
    m_db.setPassword(
        "Library@123"
    );

    if (!m_db.open()) {
        qDebug()
            << QString::fromUtf8(
                   "MySQL数据库连接失败："
               )
            << m_db.lastError().text();

        return false;
    }

    /*
     * 只检查现有三张表。
     * 不执行CREATE或ALTER。
     */
    QString checkError;

    if (!checkRequiredTables(checkError)) {
        qDebug()
            << QString::fromUtf8(
                   "数据库结构检查失败："
               )
            << checkError;

        m_db.close();
        return false;
    }

    QString overdueError;

    if (!syncOverdueStatus(overdueError)) {
        qDebug()
            << QString::fromUtf8(
                   "更新逾期状态失败："
               )
            << overdueError;
    }

    qDebug()
        << QString::fromUtf8(
               "MySQL数据库连接成功。"
           );

    return true;
}

QSqlDatabase DBManager::database() const
{
    return m_db;
}

bool DBManager::checkRequiredTables(
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库尚未连接。"
            );

        return false;
    }

    const QString sqlList[] = {
        "SELECT "
        "user_id,account,password,name,student_no,"
        "role,status,created_at "
        "FROM users "
        "WHERE 1=0",

        "SELECT "
        "book_id,isbn,title,author,publisher,"
        "category,total_count,available_count "
        "FROM books "
        "WHERE 1=0",

        "SELECT "
        "record_id,user_id,book_id,borrow_date,"
        "due_date,return_date,status,fine "
        "FROM borrow_records "
        "WHERE 1=0"
    };

    const QString tableNames[] = {
        "users",
        "books",
        "borrow_records"
    };

    for (int i = 0; i < 3; ++i) {
        QSqlQuery query(m_db);

        if (!query.exec(sqlList[i])) {
            errorMessage =
                QString::fromUtf8(
                    "无法访问数据表 %1。\n"
                    "请确认该表及字段已经存在，"
                    "并且数据库账号具有SELECT权限。\n\n"
                    "数据库错误：%2"
                )
                .arg(tableNames[i])
                .arg(query.lastError().text());

            return false;
        }
    }

    return true;
}

QString DBManager::hashPassword(
    const QString &password
) const
{
    QByteArray result =
        QCryptographicHash::hash(
            password.toUtf8(),
            QCryptographicHash::Sha256
        );

    return QString(
        result.toHex()
    );
}

bool DBManager::isPasswordStrong(
    const QString &password
) const
{
    if (password.length() < 8 ||
        password.length() > 30) {

        return false;
    }

    bool hasLetter = false;
    bool hasDigit = false;
    bool hasSymbol = false;

    for (int i = 0;
         i < password.length();
         ++i) {

        const QChar ch =
            password.at(i);

        if (ch.isSpace()) {
            return false;
        }

        if (ch.isLetter()) {
            hasLetter = true;
        } else if (ch.isDigit()) {
            hasDigit = true;
        } else {
            hasSymbol = true;
        }
    }

    return hasLetter &&
           hasDigit &&
           hasSymbol;
}

QString DBManager::passwordRuleText() const
{
    return QString::fromUtf8(
        "密码必须满足以下要求：\n"
        "1. 长度为8至30位；\n"
        "2. 至少包含一个字母；\n"
        "3. 至少包含一个数字；\n"
        "4. 至少包含一个特殊符号；\n"
        "5. 不能包含空格。\n\n"
        "例如：Library@123"
    );
}

bool DBManager::registerUser(
    const QString &account,
    const QString &password,
    const QString &name,
    const QString &studentNo,
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    const QString cleanAccount =
        account.trimmed();

    const QString cleanName =
        name.trimmed();

    const QString cleanStudentNo =
        studentNo.trimmed();

    if (cleanAccount.isEmpty() ||
        cleanName.isEmpty() ||
        cleanStudentNo.isEmpty() ||
        password.isEmpty()) {

        errorMessage =
            QString::fromUtf8(
                "注册信息不能为空。"
            );

        return false;
    }

    if (cleanAccount.length() < 4 ||
        cleanAccount.length() > 30) {

        errorMessage =
            QString::fromUtf8(
                "账号长度必须为4至30位。"
            );

        return false;
    }

    if (!isPasswordStrong(password)) {
        errorMessage =
            passwordRuleText();

        return false;
    }

    QSqlQuery query(m_db);

    query.prepare(
        "SELECT user_id "
        "FROM users "
        "WHERE account=? "
        "OR student_no=?"
    );

    query.addBindValue(
        cleanAccount
    );

    query.addBindValue(
        cleanStudentNo
    );

    if (!query.exec()) {
        errorMessage =
            QString::fromUtf8(
                "检查注册信息失败："
            )
            + query.lastError().text();

        return false;
    }

    if (query.next()) {
        errorMessage =
            QString::fromUtf8(
                "该账号或学号已经注册。"
            );

        return false;
    }

    query.prepare(
        "INSERT INTO users "
        "(account,password,name,student_no,role,status) "
        "VALUES(?,?,?,?,'reader','normal')"
    );

    query.addBindValue(
        cleanAccount
    );

    query.addBindValue(
        hashPassword(password)
    );

    query.addBindValue(
        cleanName
    );

    query.addBindValue(
        cleanStudentNo
    );

    if (!query.exec()) {
        errorMessage =
            QString::fromUtf8(
                "注册失败："
            )
            + query.lastError().text();

        return false;
    }

    return true;
}

bool DBManager::login(
    const QString &account,
    const QString &password,
    QString &role,
    QString &name,
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    QString syncError;
    syncOverdueStatus(syncError);

    QSqlQuery query(m_db);

    query.prepare(
        "SELECT name,role,status "
        "FROM users "
        "WHERE account=? "
        "AND password=?"
    );

    query.addBindValue(
        account.trimmed()
    );

    query.addBindValue(
        hashPassword(password)
    );

    if (!query.exec()) {
        errorMessage =
            QString::fromUtf8(
                "登录查询失败："
            )
            + query.lastError().text();

        return false;
    }

    if (!query.next()) {
        errorMessage =
            QString::fromUtf8(
                "账号或密码错误。"
            );

        return false;
    }

    name =
        query.value(0).toString();

    role =
        query.value(1).toString();

    const QString status =
        query.value(2).toString();

    if (status != "normal") {
        errorMessage =
            QString::fromUtf8(
                "该账号已经被禁用，请联系管理员。"
            );

        return false;
    }

    return true;
}

bool DBManager::addBook(
    const QString &isbn,
    const QString &title,
    const QString &author,
    const QString &publisher,
    const QString &category,
    int totalCount,
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    if (isbn.trimmed().isEmpty() ||
        title.trimmed().isEmpty() ||
        author.trimmed().isEmpty()) {

        errorMessage =
            QString::fromUtf8(
                "ISBN、书名和作者不能为空。"
            );

        return false;
    }

    if (totalCount <= 0) {
        errorMessage =
            QString::fromUtf8(
                "库存数量必须大于0。"
            );

        return false;
    }

    QSqlQuery query(m_db);

    query.prepare(
        "INSERT INTO books "
        "(isbn,title,author,publisher,category,"
        "total_count,available_count) "
        "VALUES(?,?,?,?,?,?,?)"
    );

    query.addBindValue(
        isbn.trimmed()
    );

    query.addBindValue(
        title.trimmed()
    );

    query.addBindValue(
        author.trimmed()
    );

    query.addBindValue(
        publisher.trimmed()
    );

    query.addBindValue(
        category.trimmed()
    );

    query.addBindValue(
        totalCount
    );

    query.addBindValue(
        totalCount
    );

    if (!query.exec()) {
        errorMessage =
            QString::fromUtf8(
                "新增图书失败："
            )
            + query.lastError().text();

        return false;
    }

    return true;
}

bool DBManager::updateBook(
    int bookId,
    const QString &isbn,
    const QString &title,
    const QString &author,
    const QString &publisher,
    const QString &category,
    int totalCount,
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    if (totalCount < 0) {
        errorMessage =
            QString::fromUtf8(
                "总库存不能小于0。"
            );

        return false;
    }

    QSqlQuery checkQuery(m_db);

    checkQuery.prepare(
        "SELECT total_count,available_count "
        "FROM books "
        "WHERE book_id=?"
    );

    checkQuery.addBindValue(
        bookId
    );

    if (!checkQuery.exec() ||
        !checkQuery.next()) {

        errorMessage =
            QString::fromUtf8(
                "未找到该图书："
            )
            + checkQuery.lastError().text();

        return false;
    }

    const int oldTotal =
        checkQuery.value(0).toInt();

    const int oldAvailable =
        checkQuery.value(1).toInt();

    const int borrowedCount =
        oldTotal - oldAvailable;

    if (totalCount < borrowedCount) {
        errorMessage =
            QString::fromUtf8(
                "当前已经借出%1册，"
                "总库存不能小于已借出数量。"
            ).arg(borrowedCount);

        return false;
    }

    const int newAvailable =
        totalCount - borrowedCount;

    QSqlQuery query(m_db);

    query.prepare(
        "UPDATE books SET "
        "isbn=?,"
        "title=?,"
        "author=?,"
        "publisher=?,"
        "category=?,"
        "total_count=?,"
        "available_count=? "
        "WHERE book_id=?"
    );

    query.addBindValue(
        isbn.trimmed()
    );

    query.addBindValue(
        title.trimmed()
    );

    query.addBindValue(
        author.trimmed()
    );

    query.addBindValue(
        publisher.trimmed()
    );

    query.addBindValue(
        category.trimmed()
    );

    query.addBindValue(
        totalCount
    );

    query.addBindValue(
        newAvailable
    );

    query.addBindValue(
        bookId
    );

    if (!query.exec()) {
        errorMessage =
            QString::fromUtf8(
                "修改图书失败："
            )
            + query.lastError().text();

        return false;
    }

    return true;
}

bool DBManager::deleteBook(
    int bookId,
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    QSqlQuery checkQuery(m_db);

    checkQuery.prepare(
        "SELECT COUNT(*) "
        "FROM borrow_records "
        "WHERE book_id=?"
    );

    checkQuery.addBindValue(
        bookId
    );

    if (!checkQuery.exec() ||
        !checkQuery.next()) {

        errorMessage =
            QString::fromUtf8(
                "检查借阅记录失败："
            )
            + checkQuery.lastError().text();

        return false;
    }

    if (checkQuery.value(0).toInt() > 0) {
        errorMessage =
            QString::fromUtf8(
                "该图书存在借阅记录，不能删除。"
            );

        return false;
    }

    QSqlQuery query(m_db);

    query.prepare(
        "DELETE FROM books "
        "WHERE book_id=?"
    );

    query.addBindValue(
        bookId
    );

    if (!query.exec()) {
        errorMessage =
            QString::fromUtf8(
                "删除图书失败："
            )
            + query.lastError().text();

        return false;
    }

    if (query.numRowsAffected() <= 0) {
        errorMessage =
            QString::fromUtf8(
                "未找到需要删除的图书。"
            );

        return false;
    }

    return true;
}

bool DBManager::updateReaderStatus(
    int userId,
    const QString &status,
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    if (status != "normal" &&
        status != "disabled") {

        errorMessage =
            QString::fromUtf8(
                "状态参数错误。"
            );

        return false;
    }

    QSqlQuery query(m_db);

    query.prepare(
        "UPDATE users SET status=? "
        "WHERE user_id=? "
        "AND role='reader'"
    );

    query.addBindValue(
        status
    );

    query.addBindValue(
        userId
    );

    if (!query.exec()) {
        errorMessage =
            QString::fromUtf8(
                "修改读者状态失败："
            )
            + query.lastError().text();

        return false;
    }

    if (query.numRowsAffected() <= 0) {
        errorMessage =
            QString::fromUtf8(
                "未找到对应的普通读者。"
            );

        return false;
    }

    return true;
}

bool DBManager::resetReaderPassword(
    int userId,
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    QSqlQuery query(m_db);

    query.prepare(
        "UPDATE users SET password=? "
        "WHERE user_id=? "
        "AND role='reader'"
    );

    query.addBindValue(
        hashPassword("Library@123")
    );

    query.addBindValue(
        userId
    );

    if (!query.exec()) {
        errorMessage =
            QString::fromUtf8(
                "重置密码失败："
            )
            + query.lastError().text();

        return false;
    }

    if (query.numRowsAffected() <= 0) {
        errorMessage =
            QString::fromUtf8(
                "未找到对应的普通读者。"
            );

        return false;
    }

    return true;
}

bool DBManager::deleteReaderAccount(
    int userId,
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    if (!m_db.transaction()) {
        errorMessage =
            QString::fromUtf8(
                "无法启动删除事务："
            )
            + m_db.lastError().text();

        return false;
    }

    /*
     * 确认目标是普通读者，防止删除管理员。
     */
    QSqlQuery userQuery(m_db);

    userQuery.prepare(
        "SELECT account,name "
        "FROM users "
        "WHERE user_id=? "
        "AND role='reader' "
        "FOR UPDATE"
    );

    userQuery.addBindValue(
        userId
    );

    if (!userQuery.exec()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "查询读者账号失败："
            )
            + userQuery.lastError().text();

        return false;
    }

    if (!userQuery.next()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "没有找到该普通读者，"
                "管理员账号不能通过此功能删除。"
            );

        return false;
    }

    /*
     * 统计尚未归还的图书。
     */
    QSqlQuery borrowedQuery(m_db);

    borrowedQuery.prepare(
        "SELECT book_id,COUNT(*) "
        "FROM borrow_records "
        "WHERE user_id=? "
        "AND status IN ('borrowed','overdue') "
        "GROUP BY book_id"
    );

    borrowedQuery.addBindValue(
        userId
    );

    if (!borrowedQuery.exec()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "查询未归还图书失败："
            )
            + borrowedQuery.lastError().text();

        return false;
    }

    /*
     * 删除账号前恢复库存。
     */
    while (borrowedQuery.next()) {
        const int bookId =
            borrowedQuery.value(0).toInt();

        const int restoreCount =
            borrowedQuery.value(1).toInt();

        QSqlQuery updateBookQuery(m_db);

        updateBookQuery.prepare(
            "UPDATE books SET "
            "available_count=LEAST("
            "available_count+?,"
            "total_count"
            ") "
            "WHERE book_id=?"
        );

        updateBookQuery.addBindValue(
            restoreCount
        );

        updateBookQuery.addBindValue(
            bookId
        );

        if (!updateBookQuery.exec()) {
            m_db.rollback();

            errorMessage =
                QString::fromUtf8(
                    "恢复图书库存失败："
                )
                + updateBookQuery
                      .lastError()
                      .text();

            return false;
        }
    }

    /*
     * 先删除借阅记录，避免外键限制。
     */
    QSqlQuery recordQuery(m_db);

    recordQuery.prepare(
        "DELETE FROM borrow_records "
        "WHERE user_id=?"
    );

    recordQuery.addBindValue(
        userId
    );

    if (!recordQuery.exec()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "删除读者借阅记录失败："
            )
            + recordQuery.lastError().text();

        return false;
    }

    /*
     * 删除普通读者账号。
     */
    QSqlQuery deleteQuery(m_db);

    deleteQuery.prepare(
        "DELETE FROM users "
        "WHERE user_id=? "
        "AND role='reader'"
    );

    deleteQuery.addBindValue(
        userId
    );

    if (!deleteQuery.exec()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "删除读者账号失败："
            )
            + deleteQuery.lastError().text();

        return false;
    }

    if (deleteQuery.numRowsAffected() <= 0) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "没有找到需要删除的普通读者账号。"
            );

        return false;
    }

    if (!m_db.commit()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "提交删除事务失败："
            )
            + m_db.lastError().text();

        return false;
    }

    return true;
}

bool DBManager::borrowBook(
    const QString &readerAccount,
    int bookId,
    int borrowDays,
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    if (borrowDays < 1 ||
        borrowDays > 60) {

        errorMessage =
            QString::fromUtf8(
                "借阅天数必须为1至60天。"
            );

        return false;
    }

    QString syncError;
    syncOverdueStatus(syncError);

    if (!m_db.transaction()) {
        errorMessage =
            QString::fromUtf8(
                "无法启动借阅事务："
            )
            + m_db.lastError().text();

        return false;
    }

    QSqlQuery userQuery(m_db);

    userQuery.prepare(
        "SELECT user_id,status "
        "FROM users "
        "WHERE account=? "
        "AND role='reader'"
    );

    userQuery.addBindValue(
        readerAccount.trimmed()
    );

    if (!userQuery.exec() ||
        !userQuery.next()) {

        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "没有找到该普通读者账号。"
            );

        return false;
    }

    const int userId =
        userQuery.value(0).toInt();

    const QString userStatus =
        userQuery.value(1).toString();

    if (userStatus != "normal") {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "该读者账号已被禁用。"
            );

        return false;
    }

    QSqlQuery overdueQuery(m_db);

    overdueQuery.prepare(
        "SELECT COUNT(*) "
        "FROM borrow_records "
        "WHERE user_id=? "
        "AND status='overdue'"
    );

    overdueQuery.addBindValue(
        userId
    );

    if (!overdueQuery.exec() ||
        !overdueQuery.next()) {

        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "检查逾期记录失败："
            )
            + overdueQuery.lastError().text();

        return false;
    }

    if (overdueQuery.value(0).toInt() > 0) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "该读者存在逾期未归还图书，"
                "归还逾期图书后才能继续借书。"
            );

        return false;
    }

    QSqlQuery duplicateQuery(m_db);

    duplicateQuery.prepare(
        "SELECT COUNT(*) "
        "FROM borrow_records "
        "WHERE user_id=? "
        "AND book_id=? "
        "AND status IN ('borrowed','overdue')"
    );

    duplicateQuery.addBindValue(
        userId
    );

    duplicateQuery.addBindValue(
        bookId
    );

    if (!duplicateQuery.exec() ||
        !duplicateQuery.next()) {

        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "检查重复借阅失败："
            )
            + duplicateQuery.lastError().text();

        return false;
    }

    if (duplicateQuery.value(0).toInt() > 0) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "该读者已经借阅了这本书，"
                "不能重复借阅。"
            );

        return false;
    }

    QSqlQuery bookQuery(m_db);

    bookQuery.prepare(
        "SELECT available_count "
        "FROM books "
        "WHERE book_id=? "
        "FOR UPDATE"
    );

    bookQuery.addBindValue(
        bookId
    );

    if (!bookQuery.exec() ||
        !bookQuery.next()) {

        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "没有找到该图书。"
            );

        return false;
    }

    if (bookQuery.value(0).toInt() <= 0) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "该图书当前没有可借库存。"
            );

        return false;
    }

    QSqlQuery insertQuery(m_db);

    insertQuery.prepare(
        "INSERT INTO borrow_records "
        "(user_id,book_id,borrow_date,due_date,"
        "return_date,status,fine) "
        "VALUES(?,?,NOW(),"
        "DATE_ADD(NOW(),INTERVAL ? DAY),"
        "NULL,'borrowed',0)"
    );

    insertQuery.addBindValue(
        userId
    );

    insertQuery.addBindValue(
        bookId
    );

    insertQuery.addBindValue(
        borrowDays
    );

    if (!insertQuery.exec()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "新增借阅记录失败："
            )
            + insertQuery.lastError().text();

        return false;
    }

    QSqlQuery stockQuery(m_db);

    stockQuery.prepare(
        "UPDATE books SET "
        "available_count=available_count-1 "
        "WHERE book_id=? "
        "AND available_count>0"
    );

    stockQuery.addBindValue(
        bookId
    );

    if (!stockQuery.exec() ||
        stockQuery.numRowsAffected() <= 0) {

        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "扣减图书库存失败："
            )
            + stockQuery.lastError().text();

        return false;
    }

    if (!m_db.commit()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "提交借阅事务失败："
            )
            + m_db.lastError().text();

        return false;
    }

    return true;
}

bool DBManager::returnBook(
    int recordId,
    QString &errorMessage,
    double &fine
)
{
    return returnBookInternal(
        recordId,
        QString(),
        false,
        errorMessage,
        fine
    );
}

bool DBManager::returnBookByReader(
    int recordId,
    const QString &readerAccount,
    QString &errorMessage,
    double &fine
)
{
    return returnBookInternal(
        recordId,
        readerAccount,
        true,
        errorMessage,
        fine
    );
}

bool DBManager::returnBookInternal(
    int recordId,
    const QString &readerAccount,
    bool checkReader,
    QString &errorMessage,
    double &fine
)
{
    fine = 0.0;

    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    if (!m_db.transaction()) {
        errorMessage =
            QString::fromUtf8(
                "无法启动归还事务："
            )
            + m_db.lastError().text();

        return false;
    }

    QSqlQuery recordQuery(m_db);

    QString sql =
        "SELECT "
        "r.book_id,"
        "r.status,"
        "r.due_date "
        "FROM borrow_records r ";

    if (checkReader) {
        sql +=
            "JOIN users u "
            "ON r.user_id=u.user_id ";
    }

    sql +=
        "WHERE r.record_id=? ";

    if (checkReader) {
        sql +=
            "AND u.account=? "
            "AND u.role='reader' ";
    }

    sql +=
        "FOR UPDATE";

    recordQuery.prepare(sql);

    recordQuery.addBindValue(
        recordId
    );

    if (checkReader) {
        recordQuery.addBindValue(
            readerAccount.trimmed()
        );
    }

    if (!recordQuery.exec() ||
        !recordQuery.next()) {

        m_db.rollback();

        errorMessage =
            checkReader
                ? QString::fromUtf8(
                      "没有找到属于当前读者的借阅记录。"
                  )
                : QString::fromUtf8(
                      "没有找到该借阅记录。"
                  );

        return false;
    }

    const int bookId =
        recordQuery.value(0).toInt();

    const QString status =
        recordQuery.value(1).toString();

    const QDateTime dueDate =
        recordQuery.value(2).toDateTime();

    if (status == "returned") {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "该图书已经归还。"
            );

        return false;
    }

    int overdueDays = 0;

    const QDateTime now =
        QDateTime::currentDateTime();

    if (dueDate.isValid() &&
        now > dueDate) {

        const qint64 overdueSeconds =
            dueDate.secsTo(now);

        overdueDays =
            static_cast<int>(
                (overdueSeconds + 86399) /
                86400
            );
    }

    fine =
        overdueDays * 0.5;

    if (fine > 20.0) {
        fine = 20.0;
    }

    QSqlQuery updateRecordQuery(m_db);

    updateRecordQuery.prepare(
        "UPDATE borrow_records SET "
        "return_date=NOW(),"
        "status='returned',"
        "fine=? "
        "WHERE record_id=?"
    );

    updateRecordQuery.addBindValue(
        fine
    );

    updateRecordQuery.addBindValue(
        recordId
    );

    if (!updateRecordQuery.exec()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "更新借阅记录失败："
            )
            + updateRecordQuery.lastError().text();

        return false;
    }

    QSqlQuery updateBookQuery(m_db);

    updateBookQuery.prepare(
        "UPDATE books SET "
        "available_count="
        "LEAST(available_count+1,total_count) "
        "WHERE book_id=?"
    );

    updateBookQuery.addBindValue(
        bookId
    );

    if (!updateBookQuery.exec()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "恢复图书库存失败："
            )
            + updateBookQuery.lastError().text();

        return false;
    }

    if (!m_db.commit()) {
        m_db.rollback();

        errorMessage =
            QString::fromUtf8(
                "提交归还事务失败："
            )
            + m_db.lastError().text();

        return false;
    }

    return true;
}

bool DBManager::syncOverdueStatus(
    QString &errorMessage
)
{
    if (!m_db.isOpen()) {
        errorMessage =
            QString::fromUtf8(
                "数据库当前未连接。"
            );

        return false;
    }

    QSqlQuery overdueQuery(m_db);

    if (!overdueQuery.exec(
            "UPDATE borrow_records SET "
            "status='overdue' "
            "WHERE status='borrowed' "
            "AND return_date IS NULL "
            "AND due_date<NOW()"
        )) {

        errorMessage =
            QString::fromUtf8(
                "更新逾期状态失败："
            )
            + overdueQuery.lastError().text();

        return false;
    }

    QSqlQuery fineQuery(m_db);

    if (!fineQuery.exec(
            "UPDATE borrow_records SET "
            "fine=LEAST("
            "GREATEST(DATEDIFF(NOW(),due_date),0)*0.5,"
            "20"
            ") "
            "WHERE status='overdue' "
            "AND return_date IS NULL"
        )) {

        errorMessage =
            QString::fromUtf8(
                "更新逾期罚款失败："
            )
            + fineQuery.lastError().text();

        return false;
    }

    return true;
}
