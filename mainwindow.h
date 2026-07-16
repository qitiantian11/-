#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QVariant>

class QFrame;
class QLabel;
class QLayout;
class QLineEdit;
class QPushButton;
class QSqlQueryModel;
class QStackedWidget;
class QTableView;
class QWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        const QString &account,
        const QString &name,
        const QString &role,
        QWidget *parent = 0
    );

    ~MainWindow();

signals:
    void logoutRequested();

private slots:
    void handleLogout();

    void showHomePage();
    void showBookPage();
    void showReaderPage();
    void showBorrowPage();
    void showOverduePage();
    void showStatisticsPage();

    void refreshHomeData();

    void refreshBooks();
    void addBook();
    void editBook();
    void deleteBook();
    void readerBorrowSelectedBook();

    void refreshReaders();
    void disableReader();
    void enableReader();
    void resetReaderPassword();
    void deleteReaderAccount();

    void refreshBorrowRecords();
    void borrowBook();
    void returnBook();

    void refreshMyBorrowRecords();
    void readerReturnSelectedBook();

    void refreshReaderProfile();

    void refreshOverdueRecords();
    void remindSelectedReader();
    void showReaderOverdueReminder();

    void refreshStatistics();

private:
    void removeLayoutSpacers(QLayout *layout);
    void clearOldContentArea();
    void initStackedPages();

    QWidget *createHomePage();
    QWidget *createBookPage();
    QWidget *createReaderPage();
    QWidget *createBorrowPage();
    QWidget *createMyBorrowPage();
    QWidget *createReaderProfilePage();
    QWidget *createOverduePage();
    QWidget *createStatisticsPage();

    QWidget *createPageHeader(
        const QString &title,
        const QString &description,
        QWidget *parent
    );

    QFrame *createDataCard(
        const QString &title,
        QLabel **numberLabel,
        const QString &description,
        const QString &backgroundColor,
        const QString &accentColor,
        QWidget *parent
    );

    QFrame *createSectionCard(
        QWidget *parent
    );

    QPushButton *createActionButton(
        const QString &text,
        QWidget *parent,
        const QString &type = "primary"
    );

    void setupTable(
        QTableView *table
    );

    void styleSearchEdit(
        QLineEdit *edit
    );

    void setCurrentButton(
        const QString &buttonName
    );

    QString roleText(
        const QString &role
    ) const;

    QString formattedDateTime(
        const QVariant &value
    ) const;

    int selectedId(
        QTableView *table,
        int column,
        const QString &message
    ) const;

private:
    Ui::MainWindow *ui;

    QStackedWidget *m_stackedWidget;
    QPushButton *m_overdueButton;

    QString m_account;
    QString m_name;
    QString m_role;

    QLabel *m_homeCard1Label;
    QLabel *m_homeCard2Label;
    QLabel *m_homeCard3Label;
    QLabel *m_homeCard4Label;

    QTableView *m_recentTable;
    QSqlQueryModel *m_recentModel;

    QLineEdit *m_bookSearchEdit;
    QTableView *m_bookTable;
    QSqlQueryModel *m_bookModel;

    QLabel *m_bookPageCard1;
    QLabel *m_bookPageCard2;
    QLabel *m_bookPageCard3;
    QLabel *m_bookPageCard4;

    QLineEdit *m_readerSearchEdit;
    QTableView *m_readerTable;
    QSqlQueryModel *m_readerModel;

    QLabel *m_readerPageCard1;
    QLabel *m_readerPageCard2;
    QLabel *m_readerPageCard3;
    QLabel *m_readerPageCard4;

    QLineEdit *m_borrowSearchEdit;
    QTableView *m_borrowTable;
    QSqlQueryModel *m_borrowModel;

    QLabel *m_borrowPageCard1;
    QLabel *m_borrowPageCard2;
    QLabel *m_borrowPageCard3;
    QLabel *m_borrowPageCard4;

    QLineEdit *m_myBorrowSearchEdit;
    QTableView *m_myBorrowTable;
    QSqlQueryModel *m_myBorrowModel;

    QLabel *m_myBorrowCard1;
    QLabel *m_myBorrowCard2;
    QLabel *m_myBorrowCard3;
    QLabel *m_myBorrowCard4;

    QLabel *m_profileAccountLabel;
    QLabel *m_profileNameLabel;
    QLabel *m_profileStudentNoLabel;
    QLabel *m_profileStatusLabel;
    QLabel *m_profileCreatedAtLabel;

    QLabel *m_profileBorrowingLabel;
    QLabel *m_profileReturnedLabel;
    QLabel *m_profileOverdueLabel;
    QLabel *m_profileFineLabel;

    QLineEdit *m_overdueSearchEdit;
    QTableView *m_overdueTable;
    QSqlQueryModel *m_overdueModel;

    QLabel *m_overduePageCard1;
    QLabel *m_overduePageCard2;
    QLabel *m_overduePageCard3;
    QLabel *m_overduePageCard4;

    QLabel *m_bookKindsLabel;
    QLabel *m_bookTotalLabel;
    QLabel *m_availableLabel;
    QLabel *m_readerCountLabel;
    QLabel *m_borrowedLabel;
    QLabel *m_overdueLabel;
    QLabel *m_totalFineLabel;
};

#endif // MAINWINDOW_H
