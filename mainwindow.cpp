#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dbmanager.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QLayoutItem>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QStackedWidget>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow(
    const QString &account,
    const QString &name,
    const QString &role,
    QWidget *parent
)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_stackedWidget(0),
      m_overdueButton(0),
      m_account(account),
      m_name(name),
      m_role(role),

      m_homeCard1Label(0),
      m_homeCard2Label(0),
      m_homeCard3Label(0),
      m_homeCard4Label(0),
      m_recentTable(0),
      m_recentModel(0),

      m_bookSearchEdit(0),
      m_bookTable(0),
      m_bookModel(0),
      m_bookPageCard1(0),
      m_bookPageCard2(0),
      m_bookPageCard3(0),
      m_bookPageCard4(0),

      m_readerSearchEdit(0),
      m_readerTable(0),
      m_readerModel(0),
      m_readerPageCard1(0),
      m_readerPageCard2(0),
      m_readerPageCard3(0),
      m_readerPageCard4(0),

      m_borrowSearchEdit(0),
      m_borrowTable(0),
      m_borrowModel(0),
      m_borrowPageCard1(0),
      m_borrowPageCard2(0),
      m_borrowPageCard3(0),
      m_borrowPageCard4(0),

      m_myBorrowSearchEdit(0),
      m_myBorrowTable(0),
      m_myBorrowModel(0),
      m_myBorrowCard1(0),
      m_myBorrowCard2(0),
      m_myBorrowCard3(0),
      m_myBorrowCard4(0),

      m_profileAccountLabel(0),
      m_profileNameLabel(0),
      m_profileStudentNoLabel(0),
      m_profileStatusLabel(0),
      m_profileCreatedAtLabel(0),
      m_profileBorrowingLabel(0),
      m_profileReturnedLabel(0),
      m_profileOverdueLabel(0),
      m_profileFineLabel(0),

      m_overdueSearchEdit(0),
      m_overdueTable(0),
      m_overdueModel(0),
      m_overduePageCard1(0),
      m_overduePageCard2(0),
      m_overduePageCard3(0),
      m_overduePageCard4(0),

      m_bookKindsLabel(0),
      m_bookTotalLabel(0),
      m_availableLabel(0),
      m_readerCountLabel(0),
      m_borrowedLabel(0),
      m_overdueLabel(0),
      m_totalFineLabel(0)
{
    ui->setupUi(this);

    setWindowTitle(
        QString::fromUtf8("图书馆管理系统 - 主界面")
    );

    resize(1220, 760);

    ui->welcomeLabel->setText(
        QString::fromUtf8("欢迎，%1").arg(m_name)
    );

    ui->roleLabel->setText(
        QString::fromUtf8("账号：%1　当前角色：%2")
            .arg(m_account)
            .arg(roleText(m_role))
    );

    ui->logoutButton->setStyleSheet(
        "QPushButton {"
        "background:#EDF3F8;"
        "color:#355C7D;"
        "border:1px solid #CAD8E3;"
        "border-radius:7px;"
        "padding:6px 16px;"
        "font-size:14px;"
        "font-weight:bold;"
        "}"
        "QPushButton:hover {"
        "background:#DFEAF2;"
        "}"
    );

    clearOldContentArea();

    m_overdueButton =
        new QPushButton(
            QString::fromUtf8("逾期管理"),
            ui->sideBar
        );

    m_overdueButton->setCursor(
        Qt::PointingHandCursor
    );

    int statisticsIndex =
        ui->sideLayout->indexOf(
            ui->statisticsButton
        );

    if (statisticsIndex < 0) {
        statisticsIndex =
            ui->sideLayout->count();
    }

    ui->sideLayout->insertWidget(
        statisticsIndex,
        m_overdueButton
    );

    initStackedPages();

    connect(
        ui->homeButton,
        SIGNAL(clicked()),
        this,
        SLOT(showHomePage())
    );

    connect(
        ui->bookButton,
        SIGNAL(clicked()),
        this,
        SLOT(showBookPage())
    );

    connect(
        ui->readerButton,
        SIGNAL(clicked()),
        this,
        SLOT(showReaderPage())
    );

    connect(
        ui->borrowButton,
        SIGNAL(clicked()),
        this,
        SLOT(showBorrowPage())
    );

    connect(
        m_overdueButton,
        SIGNAL(clicked()),
        this,
        SLOT(showOverduePage())
    );

    connect(
        ui->statisticsButton,
        SIGNAL(clicked()),
        this,
        SLOT(showStatisticsPage())
    );

    connect(
        ui->logoutButton,
        SIGNAL(clicked()),
        this,
        SLOT(handleLogout())
    );

    if (m_role != "admin") {
        ui->bookButton->setText(
            QString::fromUtf8("馆藏查询")
        );

        ui->readerButton->setText(
            QString::fromUtf8("我的借阅")
        );

        ui->borrowButton->setText(
            QString::fromUtf8("个人中心")
        );

        m_overdueButton->hide();
        ui->statisticsButton->hide();
    }

    showHomePage();

    if (m_role != "admin") {
        QTimer::singleShot(
            500,
            this,
            SLOT(showReaderOverdueReminder())
        );
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::removeLayoutSpacers(
    QLayout *layout
)
{
    if (!layout) {
        return;
    }

    for (int i = layout->count() - 1;
         i >= 0;
         --i) {

        QLayoutItem *item =
            layout->itemAt(i);

        if (!item) {
            continue;
        }

        if (item->spacerItem()) {
            QLayoutItem *removed =
                layout->takeAt(i);

            delete removed;
            continue;
        }

        if (item->layout()) {
            removeLayoutSpacers(
                item->layout()
            );
        }
    }
}

void MainWindow::clearOldContentArea()
{
    if (!ui->rightLayout) {
        return;
    }

    if (ui->titleLabel) {
        ui->rightLayout->removeWidget(
            ui->titleLabel
        );

        ui->titleLabel->hide();
        ui->titleLabel->setMaximumSize(0, 0);

        ui->titleLabel->setSizePolicy(
            QSizePolicy::Ignored,
            QSizePolicy::Ignored
        );
    }

    if (ui->contentCard) {
        ui->rightLayout->removeWidget(
            ui->contentCard
        );

        ui->contentCard->hide();
        ui->contentCard->setMaximumSize(0, 0);

        ui->contentCard->setSizePolicy(
            QSizePolicy::Ignored,
            QSizePolicy::Ignored
        );
    }

    removeLayoutSpacers(
        ui->rightLayout
    );

    ui->rightLayout->setContentsMargins(
        0, 0, 0, 0
    );

    ui->rightLayout->setSpacing(0);
}

void MainWindow::initStackedPages()
{
    clearOldContentArea();

    QWidget *layoutParent =
        ui->rightLayout->parentWidget();

    m_stackedWidget =
        new QStackedWidget(layoutParent);

    m_stackedWidget->setContentsMargins(
        0, 0, 0, 0
    );

    m_stackedWidget->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );

    m_stackedWidget->setStyleSheet(
        "QStackedWidget {"
        "background:#EEF3F8;"
        "border:none;"
        "}"
    );

    m_stackedWidget->addWidget(
        createHomePage()
    );

    m_stackedWidget->addWidget(
        createBookPage()
    );

    if (m_role == "admin") {
        m_stackedWidget->addWidget(
            createReaderPage()
        );

        m_stackedWidget->addWidget(
            createBorrowPage()
        );
    } else {
        m_stackedWidget->addWidget(
            createMyBorrowPage()
        );

        m_stackedWidget->addWidget(
            createReaderProfilePage()
        );
    }

    m_stackedWidget->addWidget(
        createOverduePage()
    );

    m_stackedWidget->addWidget(
        createStatisticsPage()
    );

    /*
     * rightLayout内如果只剩顶部栏，
     * 直接把页面插到最后即可。
     */
    ui->rightLayout->addWidget(
        m_stackedWidget,
        1
    );

    removeLayoutSpacers(
        ui->rightLayout
    );

    for (int i = 0;
         i < ui->rightLayout->count();
         ++i) {

        QLayoutItem *item =
            ui->rightLayout->itemAt(i);

        if (item &&
            item->widget() == m_stackedWidget) {

            ui->rightLayout->setStretch(
                i,
                1
            );
        } else {
            ui->rightLayout->setStretch(
                i,
                0
            );
        }
    }

    ui->rightLayout->invalidate();
    ui->rightLayout->activate();
}

QString MainWindow::roleText(
    const QString &role
) const
{
    return role == "admin"
        ? QString::fromUtf8("系统管理员")
        : QString::fromUtf8("普通读者");
}

QString MainWindow::formattedDateTime(
    const QVariant &value
) const
{
    QDateTime dateTime =
        value.toDateTime();

    if (dateTime.isValid()) {
        return dateTime.toString(
            "yyyy-MM-dd HH:mm:ss"
        );
    }

    QString text =
        value.toString();

    text.replace("T", " ");

    return text;
}

QWidget *MainWindow::createPageHeader(
    const QString &title,
    const QString &description,
    QWidget *parent
)
{
    QWidget *header =
        new QWidget(parent);

    header->setFixedHeight(58);

    QVBoxLayout *layout =
        new QVBoxLayout(header);

    layout->setContentsMargins(
        0, 0, 0, 0
    );

    layout->setSpacing(0);

    QLabel *titleLabel =
        new QLabel(title, header);

    titleLabel->setFixedHeight(35);

    titleLabel->setStyleSheet(
        "QLabel {"
        "color:#294B6D;"
        "background:transparent;"
        "border:none;"
        "font-size:24px;"
        "font-weight:bold;"
        "}"
    );

    QLabel *descriptionLabel =
        new QLabel(description, header);

    descriptionLabel->setFixedHeight(22);

    descriptionLabel->setStyleSheet(
        "QLabel {"
        "color:#7B8997;"
        "background:transparent;"
        "border:none;"
        "font-size:12px;"
        "}"
    );

    layout->addWidget(titleLabel);
    layout->addWidget(descriptionLabel);

    return header;
}

QFrame *MainWindow::createDataCard(
    const QString &title,
    QLabel **numberLabel,
    const QString &description,
    const QString &backgroundColor,
    const QString &accentColor,
    QWidget *parent
)
{
    QFrame *card =
        new QFrame(parent);

    card->setMinimumHeight(104);

    card->setStyleSheet(
        QString(
            "QFrame {"
            "background:%1;"
            "border:1px solid #D9E2EA;"
            "border-radius:11px;"
            "}"
        ).arg(backgroundColor)
    );

    QHBoxLayout *outer =
        new QHBoxLayout(card);

    outer->setContentsMargins(
        0, 0, 13, 0
    );

    outer->setSpacing(12);

    QFrame *bar =
        new QFrame(card);

    bar->setFixedWidth(5);

    bar->setStyleSheet(
        QString(
            "QFrame {"
            "background:%1;"
            "border:none;"
            "border-top-left-radius:10px;"
            "border-bottom-left-radius:10px;"
            "}"
        ).arg(accentColor)
    );

    outer->addWidget(bar);

    QVBoxLayout *content =
        new QVBoxLayout;

    content->setContentsMargins(
        0, 10, 0, 9
    );

    content->setSpacing(1);

    QLabel *titleLabel =
        new QLabel(title, card);

    titleLabel->setStyleSheet(
        "QLabel {"
        "color:#536373;"
        "background:transparent;"
        "border:none;"
        "font-size:13px;"
        "font-weight:bold;"
        "}"
    );

    *numberLabel =
        new QLabel("0", card);

    (*numberLabel)->setMinimumHeight(37);

    (*numberLabel)->setStyleSheet(
        QString(
            "QLabel {"
            "color:%1;"
            "background:transparent;"
            "border:none;"
            "font-size:27px;"
            "font-weight:bold;"
            "}"
        ).arg(accentColor)
    );

    QLabel *descriptionLabel =
        new QLabel(description, card);

    descriptionLabel->setStyleSheet(
        "QLabel {"
        "color:#8794A0;"
        "background:transparent;"
        "border:none;"
        "font-size:10px;"
        "}"
    );

    content->addWidget(titleLabel);
    content->addWidget(*numberLabel);
    content->addWidget(descriptionLabel);

    outer->addLayout(content, 1);

    return card;
}

QFrame *MainWindow::createSectionCard(
    QWidget *parent
)
{
    QFrame *card =
        new QFrame(parent);

    card->setStyleSheet(
        "QFrame {"
        "background:white;"
        "border:1px solid #DCE4EB;"
        "border-radius:11px;"
        "}"
    );

    return card;
}

QPushButton *MainWindow::createActionButton(
    const QString &text,
    QWidget *parent,
    const QString &type
)
{
    QString normal = "#4F7CAC";
    QString hover = "#416F9E";

    if (type == "success") {
        normal = "#3A9D78";
        hover = "#318767";
    } else if (type == "danger") {
        normal = "#D85A68";
        hover = "#C64D5A";
    } else if (type == "warning") {
        normal = "#E99A3E";
        hover = "#D9892F";
    } else if (type == "secondary") {
        normal = "#7B8997";
        hover = "#6A7885";
    }

    QPushButton *button =
        new QPushButton(text, parent);

    button->setMinimumHeight(36);

    button->setCursor(
        Qt::PointingHandCursor
    );

    button->setStyleSheet(
        QString(
            "QPushButton {"
            "background:%1;"
            "color:white;"
            "border:none;"
            "border-radius:7px;"
            "padding:6px 16px;"
            "font-size:13px;"
            "font-weight:bold;"
            "}"
            "QPushButton:hover {"
            "background:%2;"
            "}"
        )
        .arg(normal)
        .arg(hover)
    );

    return button;
}

void MainWindow::styleSearchEdit(
    QLineEdit *edit
)
{
    if (!edit) {
        return;
    }

    edit->setMinimumHeight(36);

    edit->setStyleSheet(
        "QLineEdit {"
        "background:#F8FAFC;"
        "border:1px solid #D4DEE7;"
        "border-radius:7px;"
        "padding:0 12px;"
        "font-size:13px;"
        "color:#3E5061;"
        "}"
        "QLineEdit:focus {"
        "background:white;"
        "border:1px solid #5A88B3;"
        "}"
    );
}

void MainWindow::setupTable(
    QTableView *table
)
{
    table->setSelectionBehavior(
        QAbstractItemView::SelectRows
    );

    table->setSelectionMode(
        QAbstractItemView::SingleSelection
    );

    table->setEditTriggers(
        QAbstractItemView::NoEditTriggers
    );

    table->setAlternatingRowColors(true);

    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(36);

    table->horizontalHeader()
        ->setStretchLastSection(true);

    table->horizontalHeader()
        ->setDefaultAlignment(Qt::AlignCenter);

    table->setStyleSheet(
        "QTableView {"
        "background:white;"
        "border:1px solid #D9E2EA;"
        "border-radius:7px;"
        "gridline-color:#E7EDF2;"
        "alternate-background-color:#F6F9FB;"
        "font-size:13px;"
        "color:#405363;"
        "}"
        "QHeaderView::section {"
        "background:#E8F0F6;"
        "color:#405A70;"
        "padding:7px;"
        "border:none;"
        "border-right:1px solid #D7E1E8;"
        "border-bottom:1px solid #D7E1E8;"
        "font-weight:bold;"
        "}"
        "QTableView::item:selected {"
        "background:#6E9BBC;"
        "color:white;"
        "}"
    );
}

/* ================= 首页 ================= */

QWidget *MainWindow::createHomePage()
{
    QWidget *page =
        new QWidget(m_stackedWidget);

    page->setStyleSheet(
        "background:#EEF3F8;"
    );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(page);

    mainLayout->setContentsMargins(
        22, 10, 22, 18
    );

    mainLayout->setSpacing(10);

    mainLayout->addWidget(
        createPageHeader(
            QString::fromUtf8("系统首页"),
            m_role == "admin"
                ? QString::fromUtf8(
                      "查看系统概况和最近借阅记录"
                  )
                : QString::fromUtf8(
                      "查看个人借阅概况和最近记录"
                  ),
            page
        )
    );

    QGridLayout *grid =
        new QGridLayout;

    grid->setSpacing(12);

    if (m_role == "admin") {
        grid->addWidget(
            createDataCard(
                QString::fromUtf8("图书总册数"),
                &m_homeCard1Label,
                QString::fromUtf8("全部馆藏图书"),
                "#EAF3FF",
                "#4F86C6",
                page
            ),
            0, 0
        );

        grid->addWidget(
            createDataCard(
                QString::fromUtf8("当前可借"),
                &m_homeCard2Label,
                QString::fromUtf8("当前可借册数"),
                "#E8F8F2",
                "#3A9D78",
                page
            ),
            0, 1
        );

        grid->addWidget(
            createDataCard(
                QString::fromUtf8("普通读者"),
                &m_homeCard3Label,
                QString::fromUtf8("已注册读者"),
                "#FFF4E6",
                "#E99A3E",
                page
            ),
            0, 2
        );

        grid->addWidget(
            createDataCard(
                QString::fromUtf8("逾期未还"),
                &m_homeCard4Label,
                QString::fromUtf8("需要及时处理"),
                "#FFEDEF",
                "#D85A68",
                page
            ),
            0, 3
        );
    } else {
        grid->addWidget(
            createDataCard(
                QString::fromUtf8("当前借阅"),
                &m_homeCard1Label,
                QString::fromUtf8("尚未归还图书"),
                "#EAF3FF",
                "#4F86C6",
                page
            ),
            0, 0
        );

        grid->addWidget(
            createDataCard(
                QString::fromUtf8("三天内到期"),
                &m_homeCard2Label,
                QString::fromUtf8("请留意归还时间"),
                "#FFF8DF",
                "#D09A24",
                page
            ),
            0, 1
        );

        grid->addWidget(
            createDataCard(
                QString::fromUtf8("已经逾期"),
                &m_homeCard3Label,
                QString::fromUtf8("逾期未归还"),
                "#FFEDEF",
                "#D85A68",
                page
            ),
            0, 2
        );

        grid->addWidget(
            createDataCard(
                QString::fromUtf8("累计罚款"),
                &m_homeCard4Label,
                QString::fromUtf8("逾期产生罚款"),
                "#F1EDFF",
                "#8066C9",
                page
            ),
            0, 3
        );
    }

    for (int i = 0; i < 4; ++i) {
        grid->setColumnStretch(i, 1);
    }

    mainLayout->addLayout(grid);

    QFrame *recentCard =
        createSectionCard(page);

    QVBoxLayout *recentLayout =
        new QVBoxLayout(recentCard);

    recentLayout->setContentsMargins(
        15, 12, 15, 15
    );

    QLabel *recentTitle =
        new QLabel(
            m_role == "admin"
                ? QString::fromUtf8("最近借阅记录")
                : QString::fromUtf8("我的最近借阅"),
            recentCard
        );

    recentTitle->setStyleSheet(
        "QLabel {"
        "color:#36566F;"
        "border:none;"
        "font-size:18px;"
        "font-weight:bold;"
        "}"
    );

    m_recentTable =
        new QTableView(recentCard);

    setupTable(m_recentTable);

    m_recentModel =
        new QSqlQueryModel(this);

    m_recentTable->setModel(
        m_recentModel
    );

    recentLayout->addWidget(recentTitle);
    recentLayout->addWidget(m_recentTable, 1);

    mainLayout->addWidget(recentCard, 1);

    refreshHomeData();

    return page;
}

void MainWindow::refreshHomeData()
{
    if (!m_homeCard1Label ||
        !m_recentModel) {
        return;
    }

    QString error;
    DBManager::instance()
        .syncOverdueStatus(error);

    QSqlQuery query(
        DBManager::instance().database()
    );

    if (m_role == "admin") {
        if (query.exec(
                "SELECT "
                "COALESCE(SUM(total_count),0),"
                "COALESCE(SUM(available_count),0) "
                "FROM books"
            ) &&
            query.next()) {

            m_homeCard1Label->setText(
                query.value(0).toString()
            );

            m_homeCard2Label->setText(
                query.value(1).toString()
            );
        }

        if (query.exec(
                "SELECT COUNT(*) FROM users "
                "WHERE role='reader'"
            ) &&
            query.next()) {

            m_homeCard3Label->setText(
                query.value(0).toString()
            );
        }

        if (query.exec(
                "SELECT COUNT(*) "
                "FROM borrow_records "
                "WHERE status='overdue'"
            ) &&
            query.next()) {

            m_homeCard4Label->setText(
                query.value(0).toString()
            );
        }

        m_recentModel->setQuery(
            "SELECT "
            "u.account,u.name,b.title,"
            "r.borrow_date,r.due_date,"
            "CASE "
            "WHEN r.status='returned' THEN '已归还' "
            "WHEN r.status='overdue' THEN '已逾期' "
            "ELSE '借阅中' END "
            "FROM borrow_records r "
            "JOIN users u ON r.user_id=u.user_id "
            "JOIN books b ON r.book_id=b.book_id "
            "ORDER BY r.record_id DESC "
            "LIMIT 8",
            DBManager::instance().database()
        );

        QString headers[] = {
            QString::fromUtf8("读者账号"),
            QString::fromUtf8("读者姓名"),
            QString::fromUtf8("书名"),
            QString::fromUtf8("借阅时间"),
            QString::fromUtf8("应还时间"),
            QString::fromUtf8("状态")
        };

        for (int i = 0; i < 6; ++i) {
            m_recentModel->setHeaderData(
                i, Qt::Horizontal, headers[i]
            );
        }
    } else {
        query.prepare(
            "SELECT "
            "COALESCE(SUM(CASE WHEN r.status IN "
            "('borrowed','overdue') THEN 1 ELSE 0 END),0),"
            "COALESCE(SUM(CASE WHEN r.status='borrowed' "
            "AND r.due_date BETWEEN NOW() "
            "AND DATE_ADD(NOW(),INTERVAL 3 DAY) "
            "THEN 1 ELSE 0 END),0),"
            "COALESCE(SUM(CASE WHEN r.status='overdue' "
            "THEN 1 ELSE 0 END),0),"
            "COALESCE(SUM(r.fine),0) "
            "FROM borrow_records r "
            "JOIN users u ON r.user_id=u.user_id "
            "WHERE u.account=?"
        );

        query.addBindValue(m_account);

        if (query.exec() &&
            query.next()) {

            m_homeCard1Label->setText(
                query.value(0).toString()
            );

            m_homeCard2Label->setText(
                query.value(1).toString()
            );

            m_homeCard3Label->setText(
                query.value(2).toString()
            );

            m_homeCard4Label->setText(
                QString::fromUtf8("￥%1")
                    .arg(
                        query.value(3).toDouble(),
                        0, 'f', 2
                    )
            );
        }

        QSqlQuery recentQuery(
            DBManager::instance().database()
        );

        recentQuery.prepare(
            "SELECT "
            "b.title,r.borrow_date,r.due_date,"
            "CASE "
            "WHEN r.status='returned' THEN '已归还' "
            "WHEN r.status='overdue' THEN '已逾期' "
            "ELSE '借阅中' END,"
            "r.fine "
            "FROM borrow_records r "
            "JOIN users u ON r.user_id=u.user_id "
            "JOIN books b ON r.book_id=b.book_id "
            "WHERE u.account=? "
            "ORDER BY r.record_id DESC "
            "LIMIT 8"
        );

        recentQuery.addBindValue(m_account);
        recentQuery.exec();

        m_recentModel->setQuery(recentQuery);

        QString headers[] = {
            QString::fromUtf8("书名"),
            QString::fromUtf8("借阅时间"),
            QString::fromUtf8("应还时间"),
            QString::fromUtf8("状态"),
            QString::fromUtf8("罚款/元")
        };

        for (int i = 0; i < 5; ++i) {
            m_recentModel->setHeaderData(
                i, Qt::Horizontal, headers[i]
            );
        }
    }

    m_recentTable->resizeColumnsToContents();
}

/* ================= 图书页面 ================= */

QWidget *MainWindow::createBookPage()
{
    QWidget *page =
        new QWidget(m_stackedWidget);

    page->setStyleSheet(
        "background:#EEF3F8;"
    );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(page);

    mainLayout->setContentsMargins(
        22, 10, 22, 18
    );

    mainLayout->setSpacing(10);

    mainLayout->addWidget(
        createPageHeader(
            m_role == "admin"
                ? QString::fromUtf8("图书管理")
                : QString::fromUtf8("馆藏查询"),
            m_role == "admin"
                ? QString::fromUtf8(
                      "管理图书资料和库存数量"
                  )
                : QString::fromUtf8(
                      "查询并选择图书进行借阅"
                  ),
            page
        )
    );

    QGridLayout *grid =
        new QGridLayout;

    grid->setSpacing(12);

    grid->addWidget(
        createDataCard(
            m_role == "admin"
                ? QString::fromUtf8("图书种类")
                : QString::fromUtf8("可借种类"),
            &m_bookPageCard1,
            QString::fromUtf8("馆藏图书数量"),
            "#EAF3FF",
            "#4F86C6",
            page
        ),
        0, 0
    );

    grid->addWidget(
        createDataCard(
            m_role == "admin"
                ? QString::fromUtf8("图书总册")
                : QString::fromUtf8("可借总册"),
            &m_bookPageCard2,
            QString::fromUtf8("图书库存数量"),
            "#E8F8F2",
            "#3A9D78",
            page
        ),
        0, 1
    );

    grid->addWidget(
        createDataCard(
            m_role == "admin"
                ? QString::fromUtf8("已经借出")
                : QString::fromUtf8("我的借阅"),
            &m_bookPageCard3,
            QString::fromUtf8("尚未归还数量"),
            "#FFF4E6",
            "#E99A3E",
            page
        ),
        0, 2
    );

    grid->addWidget(
        createDataCard(
            m_role == "admin"
                ? QString::fromUtf8("库存不足")
                : QString::fromUtf8("我的逾期"),
            &m_bookPageCard4,
            QString::fromUtf8("需要及时处理"),
            "#FFEDEF",
            "#D85A68",
            page
        ),
        0, 3
    );

    for (int i = 0; i < 4; ++i) {
        grid->setColumnStretch(i, 1);
    }

    mainLayout->addLayout(grid);

    QFrame *card =
        createSectionCard(page);

    QVBoxLayout *cardLayout =
        new QVBoxLayout(card);

    cardLayout->setContentsMargins(
        14, 12, 14, 14
    );

    cardLayout->setSpacing(9);

    QHBoxLayout *searchLayout =
        new QHBoxLayout;

    m_bookSearchEdit =
        new QLineEdit(card);

    m_bookSearchEdit->setPlaceholderText(
        QString::fromUtf8(
            "输入书名、作者、ISBN或分类查询"
        )
    );

    styleSearchEdit(m_bookSearchEdit);

    QPushButton *searchButton =
        createActionButton(
            QString::fromUtf8("查询"),
            card
        );

    QPushButton *refreshButton =
        createActionButton(
            QString::fromUtf8("刷新"),
            card,
            "secondary"
        );

    searchLayout->addWidget(
        m_bookSearchEdit, 1
    );

    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(refreshButton);

    cardLayout->addLayout(searchLayout);

    QHBoxLayout *actionLayout =
        new QHBoxLayout;

    actionLayout->addStretch();

    if (m_role == "admin") {
        QPushButton *addButton =
            createActionButton(
                QString::fromUtf8("新增图书"),
                card,
                "success"
            );

        QPushButton *editButton =
            createActionButton(
                QString::fromUtf8("修改图书"),
                card
            );

        QPushButton *deleteButton =
            createActionButton(
                QString::fromUtf8("删除图书"),
                card,
                "danger"
            );

        actionLayout->addWidget(addButton);
        actionLayout->addWidget(editButton);
        actionLayout->addWidget(deleteButton);

        connect(
            addButton,
            SIGNAL(clicked()),
            this,
            SLOT(addBook())
        );

        connect(
            editButton,
            SIGNAL(clicked()),
            this,
            SLOT(editBook())
        );

        connect(
            deleteButton,
            SIGNAL(clicked()),
            this,
            SLOT(deleteBook())
        );
    } else {
        QPushButton *borrowButton =
            createActionButton(
                QString::fromUtf8("借阅所选图书"),
                card,
                "success"
            );

        actionLayout->addWidget(borrowButton);

        connect(
            borrowButton,
            SIGNAL(clicked()),
            this,
            SLOT(readerBorrowSelectedBook())
        );
    }

    cardLayout->addLayout(actionLayout);

    m_bookTable =
        new QTableView(card);

    setupTable(m_bookTable);

    m_bookModel =
        new QSqlQueryModel(this);

    m_bookTable->setModel(m_bookModel);

    cardLayout->addWidget(m_bookTable, 1);
    mainLayout->addWidget(card, 1);

    connect(
        searchButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshBooks())
    );

    connect(
        refreshButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshBooks())
    );

    connect(
        m_bookSearchEdit,
        SIGNAL(returnPressed()),
        this,
        SLOT(refreshBooks())
    );

    refreshBooks();

    return page;
}

void MainWindow::refreshBooks()
{
    if (!m_bookModel || !m_bookTable) {
        return;
    }

    QString key = m_bookSearchEdit
        ? m_bookSearchEdit->text().trimmed()
        : QString();

    QSqlQuery query(DBManager::instance().database());

    QString sql =
        "SELECT "
        "ROW_NUMBER() OVER (ORDER BY book_id ASC) AS display_no,"
        "isbn,title,author,publisher,category,"
        "total_count,available_count,book_id "
        "FROM books ";

    if (!key.isEmpty()) {
        sql +=
            "WHERE isbn LIKE ? "
            "OR title LIKE ? "
            "OR author LIKE ? "
            "OR category LIKE ? ";
    }

    sql += "ORDER BY book_id ASC";
    query.prepare(sql);

    if (!key.isEmpty()) {
        const QString likeKey = "%" + key + "%";
        for (int i = 0; i < 4; ++i) {
            query.addBindValue(likeKey);
        }
    }

    if (!query.exec()) {
        QMessageBox::warning(
            this,
            QString::fromUtf8("查询失败"),
            query.lastError().text()
        );
        return;
    }

    m_bookModel->setQuery(query);

    QString headers[] = {
        QString::fromUtf8("编号"),
        QString::fromUtf8("ISBN"),
        QString::fromUtf8("书名"),
        QString::fromUtf8("作者"),
        QString::fromUtf8("出版社"),
        QString::fromUtf8("分类"),
        QString::fromUtf8("总库存"),
        QString::fromUtf8("可借数量"),
        QString::fromUtf8("真实ID")
    };

    for (int i = 0; i < 9; ++i) {
        m_bookModel->setHeaderData(i, Qt::Horizontal, headers[i]);
    }

    m_bookTable->setColumnHidden(8, true);
    m_bookTable->resizeColumnsToContents();

    QSqlQuery countQuery(DBManager::instance().database());

    if (m_role == "admin") {
        countQuery.exec(
            "SELECT COUNT(*),"
            "COALESCE(SUM(total_count),0),"
            "COALESCE(SUM(total_count-available_count),0),"
            "COALESCE(SUM(CASE WHEN available_count<=0 "
            "THEN 1 ELSE 0 END),0) "
            "FROM books"
        );
    } else {
        countQuery.prepare(
            "SELECT "
            "(SELECT COUNT(*) FROM books WHERE available_count>0),"
            "(SELECT COALESCE(SUM(available_count),0) FROM books),"
            "COALESCE(SUM(CASE WHEN r.status IN "
            "('borrowed','overdue') THEN 1 ELSE 0 END),0),"
            "COALESCE(SUM(CASE WHEN r.status='overdue' "
            "THEN 1 ELSE 0 END),0) "
            "FROM borrow_records r "
            "JOIN users u ON r.user_id=u.user_id "
            "WHERE u.account=?"
        );
        countQuery.addBindValue(m_account);
        countQuery.exec();
    }

    if (countQuery.next()) {
        m_bookPageCard1->setText(countQuery.value(0).toString());
        m_bookPageCard2->setText(countQuery.value(1).toString());
        m_bookPageCard3->setText(countQuery.value(2).toString());
        m_bookPageCard4->setText(countQuery.value(3).toString());
    }
}

/* ================= 读者管理 ================= */

QWidget *MainWindow::createReaderPage()
{
    QWidget *page =
        new QWidget(m_stackedWidget);

    page->setStyleSheet(
        "background:#EEF3F8;"
    );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(page);

    mainLayout->setContentsMargins(
        22, 10, 22, 18
    );

    mainLayout->setSpacing(10);

    mainLayout->addWidget(
        createPageHeader(
            QString::fromUtf8("读者管理"),
            QString::fromUtf8(
                "查看读者资料并管理账号状态"
            ),
            page
        )
    );

    QGridLayout *grid =
        new QGridLayout;

    grid->setSpacing(12);

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("读者总数"),
            &m_readerPageCard1,
            QString::fromUtf8("全部普通读者"),
            "#EAF3FF",
            "#4F86C6",
            page
        ),
        0, 0
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("正常账号"),
            &m_readerPageCard2,
            QString::fromUtf8("允许正常登录"),
            "#E8F8F2",
            "#3A9D78",
            page
        ),
        0, 1
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("禁用账号"),
            &m_readerPageCard3,
            QString::fromUtf8("禁止登录账号"),
            "#FFF4E6",
            "#E99A3E",
            page
        ),
        0, 2
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("有逾期读者"),
            &m_readerPageCard4,
            QString::fromUtf8("存在逾期图书"),
            "#FFEDEF",
            "#D85A68",
            page
        ),
        0, 3
    );

    for (int i = 0; i < 4; ++i) {
        grid->setColumnStretch(i, 1);
    }

    mainLayout->addLayout(grid);

    QFrame *card =
        createSectionCard(page);

    QVBoxLayout *cardLayout =
        new QVBoxLayout(card);

    cardLayout->setContentsMargins(
        14, 12, 14, 14
    );

    cardLayout->setSpacing(9);

    QHBoxLayout *searchLayout =
        new QHBoxLayout;

    m_readerSearchEdit =
        new QLineEdit(card);

    m_readerSearchEdit->setPlaceholderText(
        QString::fromUtf8(
            "输入账号、姓名或学号查询"
        )
    );

    styleSearchEdit(m_readerSearchEdit);

    QPushButton *searchButton =
        createActionButton(
            QString::fromUtf8("查询"),
            card
        );

    QPushButton *refreshButton =
        createActionButton(
            QString::fromUtf8("刷新"),
            card,
            "secondary"
        );

    searchLayout->addWidget(
        m_readerSearchEdit, 1
    );

    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(refreshButton);

    cardLayout->addLayout(searchLayout);

    QHBoxLayout *actions =
        new QHBoxLayout;

    actions->addStretch();

    QPushButton *disableButton =
        createActionButton(
            QString::fromUtf8("禁用账号"),
            card,
            "danger"
        );

    QPushButton *enableButton =
        createActionButton(
            QString::fromUtf8("恢复账号"),
            card,
            "success"
        );

    QPushButton *resetButton =
        createActionButton(
            QString::fromUtf8("重置密码"),
            card,
            "warning"
        );

    QPushButton *deleteAccountButton =
        createActionButton(
            QString::fromUtf8("删除账号"),
            card,
            "danger"
        );

    actions->addWidget(disableButton);
    actions->addWidget(enableButton);
    actions->addWidget(resetButton);
    actions->addWidget(deleteAccountButton);

    cardLayout->addLayout(actions);

    m_readerTable =
        new QTableView(card);

    setupTable(m_readerTable);

    m_readerModel =
        new QSqlQueryModel(this);

    m_readerTable->setModel(m_readerModel);

    cardLayout->addWidget(m_readerTable, 1);
    mainLayout->addWidget(card, 1);

    connect(
        searchButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshReaders())
    );

    connect(
        refreshButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshReaders())
    );

    connect(
        disableButton,
        SIGNAL(clicked()),
        this,
        SLOT(disableReader())
    );

    connect(
        enableButton,
        SIGNAL(clicked()),
        this,
        SLOT(enableReader())
    );

    connect(
        resetButton,
        SIGNAL(clicked()),
        this,
        SLOT(resetReaderPassword())
    );

    connect(
        deleteAccountButton,
        SIGNAL(clicked()),
        this,
        SLOT(deleteReaderAccount())
    );

    refreshReaders();

    return page;
}

void MainWindow::refreshReaders()
{
    if (!m_readerModel || !m_readerTable) {
        return;
    }

    QString key = m_readerSearchEdit
        ? m_readerSearchEdit->text().trimmed()
        : QString();

    QSqlQuery query(DBManager::instance().database());

    QString sql =
        "SELECT "
        "ROW_NUMBER() OVER (ORDER BY user_id ASC) AS display_no,"
        "account,name,student_no,"
        "CASE status WHEN 'normal' THEN '正常' ELSE '已禁用' END,"
        "created_at,user_id "
        "FROM users "
        "WHERE role='reader' ";

    if (!key.isEmpty()) {
        sql +=
            "AND (account LIKE ? "
            "OR name LIKE ? "
            "OR student_no LIKE ?) ";
    }

    sql += "ORDER BY user_id ASC";
    query.prepare(sql);

    if (!key.isEmpty()) {
        const QString likeKey = "%" + key + "%";
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
    }

    if (!query.exec()) {
        QMessageBox::warning(
            this,
            QString::fromUtf8("查询失败"),
            query.lastError().text()
        );
        return;
    }

    m_readerModel->setQuery(query);

    QString headers[] = {
        QString::fromUtf8("编号"),
        QString::fromUtf8("账号"),
        QString::fromUtf8("姓名"),
        QString::fromUtf8("学号"),
        QString::fromUtf8("状态"),
        QString::fromUtf8("注册时间"),
        QString::fromUtf8("真实ID")
    };

    for (int i = 0; i < 7; ++i) {
        m_readerModel->setHeaderData(i, Qt::Horizontal, headers[i]);
    }

    m_readerTable->setColumnHidden(6, true);
    m_readerTable->resizeColumnsToContents();

    QString syncError;
    DBManager::instance().syncOverdueStatus(syncError);

    QSqlQuery countQuery(DBManager::instance().database());
    countQuery.exec(
        "SELECT COUNT(*),"
        "COALESCE(SUM(CASE WHEN status='normal' THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE WHEN status='disabled' THEN 1 ELSE 0 END),0) "
        "FROM users WHERE role='reader'"
    );

    if (countQuery.next()) {
        m_readerPageCard1->setText(countQuery.value(0).toString());
        m_readerPageCard2->setText(countQuery.value(1).toString());
        m_readerPageCard3->setText(countQuery.value(2).toString());
    }

    countQuery.exec(
        "SELECT COUNT(DISTINCT user_id) "
        "FROM borrow_records WHERE status='overdue'"
    );

    if (countQuery.next()) {
        m_readerPageCard4->setText(countQuery.value(0).toString());
    }
}

/* ================= 借阅管理 ================= */

QWidget *MainWindow::createBorrowPage()
{
    QWidget *page =
        new QWidget(m_stackedWidget);

    page->setStyleSheet(
        "background:#EEF3F8;"
    );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(page);

    mainLayout->setContentsMargins(
        22, 10, 22, 18
    );

    mainLayout->setSpacing(10);

    mainLayout->addWidget(
        createPageHeader(
            QString::fromUtf8("借阅管理"),
            QString::fromUtf8(
                "办理借书和还书并查看借阅记录"
            ),
            page
        )
    );

    QGridLayout *grid =
        new QGridLayout;

    grid->setSpacing(12);

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("借阅中"),
            &m_borrowPageCard1,
            QString::fromUtf8("正常未到期"),
            "#EAF3FF",
            "#4F86C6",
            page
        ),
        0, 0
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("已归还"),
            &m_borrowPageCard2,
            QString::fromUtf8("历史归还记录"),
            "#E8F8F2",
            "#3A9D78",
            page
        ),
        0, 1
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("已经逾期"),
            &m_borrowPageCard3,
            QString::fromUtf8("超过应还时间"),
            "#FFEDEF",
            "#D85A68",
            page
        ),
        0, 2
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("今日借阅"),
            &m_borrowPageCard4,
            QString::fromUtf8("今日新增记录"),
            "#FFF4E6",
            "#E99A3E",
            page
        ),
        0, 3
    );

    for (int i = 0; i < 4; ++i) {
        grid->setColumnStretch(i, 1);
    }

    mainLayout->addLayout(grid);

    QFrame *card =
        createSectionCard(page);

    QVBoxLayout *cardLayout =
        new QVBoxLayout(card);

    cardLayout->setContentsMargins(
        14, 12, 14, 14
    );

    QHBoxLayout *searchLayout =
        new QHBoxLayout;

    m_borrowSearchEdit =
        new QLineEdit(card);

    m_borrowSearchEdit->setPlaceholderText(
        QString::fromUtf8(
            "输入账号、姓名、书名或ISBN查询"
        )
    );

    styleSearchEdit(m_borrowSearchEdit);

    QPushButton *searchButton =
        createActionButton(
            QString::fromUtf8("查询"),
            card
        );

    QPushButton *refreshButton =
        createActionButton(
            QString::fromUtf8("刷新"),
            card,
            "secondary"
        );

    searchLayout->addWidget(
        m_borrowSearchEdit, 1
    );

    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(refreshButton);

    cardLayout->addLayout(searchLayout);

    QHBoxLayout *actions =
        new QHBoxLayout;

    actions->addStretch();

    QPushButton *borrowButton =
        createActionButton(
            QString::fromUtf8("办理借书"),
            card,
            "success"
        );

    QPushButton *returnButton =
        createActionButton(
            QString::fromUtf8("办理还书"),
            card,
            "warning"
        );

    actions->addWidget(borrowButton);
    actions->addWidget(returnButton);

    cardLayout->addLayout(actions);

    m_borrowTable =
        new QTableView(card);

    setupTable(m_borrowTable);

    m_borrowModel =
        new QSqlQueryModel(this);

    m_borrowTable->setModel(
        m_borrowModel
    );

    cardLayout->addWidget(m_borrowTable, 1);
    mainLayout->addWidget(card, 1);

    connect(
        searchButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshBorrowRecords())
    );

    connect(
        refreshButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshBorrowRecords())
    );

    connect(
        borrowButton,
        SIGNAL(clicked()),
        this,
        SLOT(borrowBook())
    );

    connect(
        returnButton,
        SIGNAL(clicked()),
        this,
        SLOT(returnBook())
    );

    refreshBorrowRecords();

    return page;
}

void MainWindow::refreshBorrowRecords()
{
    if (!m_borrowModel || !m_borrowTable) {
        return;
    }

    QString syncError;
    DBManager::instance().syncOverdueStatus(syncError);

    QString key;
    if (m_borrowSearchEdit) {
        key = m_borrowSearchEdit->text().trimmed();
    }

    QSqlQuery query(DBManager::instance().database());

    QString sql =
        "SELECT "
        "ROW_NUMBER() OVER(ORDER BY r.record_id ASC) AS display_no,"
        "u.account,u.name,b.isbn,b.title,"
        "r.borrow_date,r.due_date,r.return_date,"
        "DATEDIFF(r.due_date,r.borrow_date),"
        "CASE "
        "WHEN r.status='returned' THEN '已归还' "
        "WHEN r.status='overdue' THEN '已逾期' "
        "ELSE '借阅中' END,"
        "CASE WHEN r.status='overdue' "
        "THEN GREATEST(DATEDIFF(NOW(),r.due_date),0) "
        "ELSE 0 END,"
        "r.fine,r.record_id "
        "FROM borrow_records r "
        "JOIN users u ON r.user_id=u.user_id "
        "JOIN books b ON r.book_id=b.book_id "
        "WHERE 1=1 ";

    if (!key.isEmpty()) {
        sql +=
            "AND (u.account LIKE ? "
            "OR u.name LIKE ? "
            "OR b.isbn LIKE ? "
            "OR b.title LIKE ?) ";
    }

    sql += "ORDER BY r.record_id ASC";
    query.prepare(sql);

    if (!key.isEmpty()) {
        const QString likeKey = "%" + key + "%";
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
    }

    if (!query.exec()) {
        QMessageBox::warning(this, QString::fromUtf8("查询失败"), query.lastError().text());
        return;
    }

    m_borrowModel->setQuery(query);

    const QString headers[] = {
        QString::fromUtf8("记录号"),
        QString::fromUtf8("读者账号"),
        QString::fromUtf8("读者姓名"),
        QString::fromUtf8("ISBN"),
        QString::fromUtf8("书名"),
        QString::fromUtf8("借阅时间"),
        QString::fromUtf8("应还时间"),
        QString::fromUtf8("归还时间"),
        QString::fromUtf8("借阅天数"),
        QString::fromUtf8("状态"),
        QString::fromUtf8("逾期天数"),
        QString::fromUtf8("罚款/元"),
        QString::fromUtf8("真实记录ID")
    };

    for (int i = 0; i < 13; ++i) {
        m_borrowModel->setHeaderData(i, Qt::Horizontal, headers[i]);
    }

    m_borrowTable->setColumnHidden(12, true);
    m_borrowTable->resizeColumnsToContents();

    QSqlQuery countQuery(DBManager::instance().database());
    countQuery.exec(
        "SELECT "
        "COALESCE(SUM(CASE WHEN status='borrowed' THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE WHEN status='returned' THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE WHEN status='overdue' THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE WHEN DATE(borrow_date)=CURDATE() THEN 1 ELSE 0 END),0) "
        "FROM borrow_records"
    );

    if (countQuery.next()) {
        if (m_borrowPageCard1) m_borrowPageCard1->setText(countQuery.value(0).toString());
        if (m_borrowPageCard2) m_borrowPageCard2->setText(countQuery.value(1).toString());
        if (m_borrowPageCard3) m_borrowPageCard3->setText(countQuery.value(2).toString());
        if (m_borrowPageCard4) m_borrowPageCard4->setText(countQuery.value(3).toString());
    }
}

/* ================= 我的借阅 ================= */

QWidget *MainWindow::createMyBorrowPage()
{
    QWidget *page =
        new QWidget(m_stackedWidget);

    page->setStyleSheet(
        "background:#EEF3F8;"
    );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(page);

    mainLayout->setContentsMargins(
        22, 10, 22, 18
    );

    mainLayout->setSpacing(10);

    mainLayout->addWidget(
        createPageHeader(
            QString::fromUtf8("我的借阅"),
            QString::fromUtf8(
                "查看借阅状态并办理还书"
            ),
            page
        )
    );

    QGridLayout *grid =
        new QGridLayout;

    grid->setSpacing(12);

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("当前借阅"),
            &m_myBorrowCard1,
            QString::fromUtf8("尚未归还记录"),
            "#EAF3FF",
            "#4F86C6",
            page
        ),
        0, 0
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("三天内到期"),
            &m_myBorrowCard2,
            QString::fromUtf8("请及时归还"),
            "#FFF8DF",
            "#D09A24",
            page
        ),
        0, 1
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("已经逾期"),
            &m_myBorrowCard3,
            QString::fromUtf8("超过应还时间"),
            "#FFEDEF",
            "#D85A68",
            page
        ),
        0, 2
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("当前罚款"),
            &m_myBorrowCard4,
            QString::fromUtf8("累计逾期罚款"),
            "#F1EDFF",
            "#8066C9",
            page
        ),
        0, 3
    );

    for (int i = 0; i < 4; ++i) {
        grid->setColumnStretch(i, 1);
    }

    mainLayout->addLayout(grid);

    QFrame *card =
        createSectionCard(page);

    QVBoxLayout *cardLayout =
        new QVBoxLayout(card);

    cardLayout->setContentsMargins(
        14, 12, 14, 14
    );

    QHBoxLayout *searchLayout =
        new QHBoxLayout;

    m_myBorrowSearchEdit =
        new QLineEdit(card);

    m_myBorrowSearchEdit->setPlaceholderText(
        QString::fromUtf8(
            "输入书名、作者或ISBN查询"
        )
    );

    styleSearchEdit(m_myBorrowSearchEdit);

    QPushButton *searchButton =
        createActionButton(
            QString::fromUtf8("查询"),
            card
        );

    QPushButton *refreshButton =
        createActionButton(
            QString::fromUtf8("刷新"),
            card,
            "secondary"
        );

    searchLayout->addWidget(
        m_myBorrowSearchEdit, 1
    );

    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(refreshButton);

    cardLayout->addLayout(searchLayout);

    QHBoxLayout *actions =
        new QHBoxLayout;

    actions->addStretch();

    QPushButton *returnButton =
        createActionButton(
            QString::fromUtf8("归还所选图书"),
            card,
            "success"
        );

    actions->addWidget(returnButton);
    cardLayout->addLayout(actions);

    m_myBorrowTable =
        new QTableView(card);

    setupTable(m_myBorrowTable);

    m_myBorrowModel =
        new QSqlQueryModel(this);

    m_myBorrowTable->setModel(
        m_myBorrowModel
    );

    cardLayout->addWidget(
        m_myBorrowTable, 1
    );

    mainLayout->addWidget(card, 1);

    connect(
        searchButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshMyBorrowRecords())
    );

    connect(
        refreshButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshMyBorrowRecords())
    );

    connect(
        returnButton,
        SIGNAL(clicked()),
        this,
        SLOT(readerReturnSelectedBook())
    );

    refreshMyBorrowRecords();

    return page;
}

void MainWindow::refreshMyBorrowRecords()
{
    if (!m_myBorrowModel || !m_myBorrowTable) {
        return;
    }

    QString syncError;
    DBManager::instance().syncOverdueStatus(syncError);

    QString key;
    if (m_myBorrowSearchEdit) {
        key = m_myBorrowSearchEdit->text().trimmed();
    }

    QSqlQuery query(DBManager::instance().database());

    QString sql =
        "SELECT "
        "ROW_NUMBER() OVER(ORDER BY r.record_id ASC) AS display_no,"
        "b.isbn,b.title,b.author,"
        "r.borrow_date,r.due_date,r.return_date,"
        "DATEDIFF(r.due_date,r.borrow_date),"
        "CASE "
        "WHEN r.status='returned' THEN '已归还' "
        "WHEN r.status='overdue' THEN '已逾期' "
        "ELSE '借阅中' END,"
        "CASE WHEN r.status='overdue' "
        "THEN GREATEST(DATEDIFF(NOW(),r.due_date),0) "
        "ELSE 0 END,"
        "r.fine,r.record_id "
        "FROM borrow_records r "
        "JOIN users u ON r.user_id=u.user_id "
        "JOIN books b ON r.book_id=b.book_id "
        "WHERE u.account=? ";

    if (!key.isEmpty()) {
        sql +=
            "AND (b.isbn LIKE ? "
            "OR b.title LIKE ? "
            "OR b.author LIKE ?) ";
    }

    sql += "ORDER BY r.record_id ASC";
    query.prepare(sql);
    query.addBindValue(m_account);

    if (!key.isEmpty()) {
        const QString likeKey = "%" + key + "%";
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
    }

    if (!query.exec()) {
        QMessageBox::warning(this, QString::fromUtf8("查询失败"), query.lastError().text());
        return;
    }

    m_myBorrowModel->setQuery(query);

    const QString headers[] = {
        QString::fromUtf8("记录号"),
        QString::fromUtf8("ISBN"),
        QString::fromUtf8("书名"),
        QString::fromUtf8("作者"),
        QString::fromUtf8("借阅时间"),
        QString::fromUtf8("应还时间"),
        QString::fromUtf8("归还时间"),
        QString::fromUtf8("借阅天数"),
        QString::fromUtf8("状态"),
        QString::fromUtf8("逾期天数"),
        QString::fromUtf8("罚款/元"),
        QString::fromUtf8("真实记录ID")
    };

    for (int i = 0; i < 12; ++i) {
        m_myBorrowModel->setHeaderData(i, Qt::Horizontal, headers[i]);
    }

    m_myBorrowTable->setColumnHidden(11, true);
    m_myBorrowTable->resizeColumnsToContents();

    QSqlQuery countQuery(DBManager::instance().database());
    countQuery.prepare(
        "SELECT "
        "COALESCE(SUM(CASE WHEN r.status IN ('borrowed','overdue') THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE WHEN r.status='borrowed' "
        "AND r.due_date BETWEEN NOW() AND DATE_ADD(NOW(),INTERVAL 3 DAY) "
        "THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE WHEN r.status='overdue' THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(r.fine),0) "
        "FROM borrow_records r "
        "JOIN users u ON r.user_id=u.user_id "
        "WHERE u.account=?"
    );
    countQuery.addBindValue(m_account);

    if (countQuery.exec() && countQuery.next()) {
        if (m_myBorrowCard1) m_myBorrowCard1->setText(countQuery.value(0).toString());
        if (m_myBorrowCard2) m_myBorrowCard2->setText(countQuery.value(1).toString());
        if (m_myBorrowCard3) m_myBorrowCard3->setText(countQuery.value(2).toString());
        if (m_myBorrowCard4) {
            m_myBorrowCard4->setText(QString::fromUtf8("￥%1").arg(countQuery.value(3).toDouble(), 0, 'f', 2));
        }
    }
}

/* ================= 个人中心 ================= */

QWidget *MainWindow::createReaderProfilePage()
{
    QWidget *page =
        new QWidget(m_stackedWidget);

    page->setStyleSheet(
        "background:#EEF3F8;"
    );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(page);

    mainLayout->setContentsMargins(
        22, 10, 22, 18
    );

    mainLayout->setSpacing(10);

    mainLayout->addWidget(
        createPageHeader(
            QString::fromUtf8("个人中心"),
            QString::fromUtf8(
                "查看账号资料、借阅概况和借阅规则"
            ),
            page
        )
    );

    QHBoxLayout *bodyLayout =
        new QHBoxLayout;

    bodyLayout->setSpacing(14);

    QFrame *profileCard =
        createSectionCard(page);

    QVBoxLayout *profileLayout =
        new QVBoxLayout(profileCard);

    profileLayout->setContentsMargins(
        20, 18, 20, 18
    );

    profileLayout->setSpacing(10);

    QLabel *profileTitle =
        new QLabel(
            QString::fromUtf8("基本信息"),
            profileCard
        );

    profileTitle->setFixedHeight(31);

    profileTitle->setStyleSheet(
        "QLabel {"
        "color:#36566F;"
        "border:none;"
        "font-size:19px;"
        "font-weight:bold;"
        "}"
    );

    profileLayout->addWidget(profileTitle);

    QGridLayout *infoGrid =
        new QGridLayout;

    infoGrid->setHorizontalSpacing(16);
    infoGrid->setVerticalSpacing(10);
    infoGrid->setColumnMinimumWidth(0, 82);
    infoGrid->setColumnStretch(1, 1);

    QString names[] = {
        QString::fromUtf8("账号"),
        QString::fromUtf8("姓名"),
        QString::fromUtf8("学号"),
        QString::fromUtf8("账号状态"),
        QString::fromUtf8("注册时间")
    };

    m_profileAccountLabel =
        new QLabel("-", profileCard);

    m_profileNameLabel =
        new QLabel("-", profileCard);

    m_profileStudentNoLabel =
        new QLabel("-", profileCard);

    m_profileStatusLabel =
        new QLabel("-", profileCard);

    m_profileCreatedAtLabel =
        new QLabel("-", profileCard);

    QLabel *values[] = {
        m_profileAccountLabel,
        m_profileNameLabel,
        m_profileStudentNoLabel,
        m_profileStatusLabel,
        m_profileCreatedAtLabel
    };

    for (int i = 0; i < 5; ++i) {
        QLabel *nameLabel =
            new QLabel(names[i], profileCard);

        nameLabel->setFixedHeight(40);

        nameLabel->setStyleSheet(
            "QLabel {"
            "color:#667786;"
            "border:none;"
            "font-size:14px;"
            "}"
        );

        values[i]->setFixedHeight(40);
        values[i]->setContentsMargins(
            13, 0, 13, 0
        );

        values[i]->setStyleSheet(
            "QLabel {"
            "color:#33475B;"
            "background:#F6F9FB;"
            "border:1px solid #D4DEE7;"
            "border-radius:7px;"
            "font-size:14px;"
            "}"
        );

        infoGrid->addWidget(
            nameLabel, i, 0
        );

        infoGrid->addWidget(
            values[i], i, 1
        );
    }

    profileLayout->addLayout(infoGrid);
    profileLayout->addStretch();

    QPushButton *refreshButton =
        createActionButton(
            QString::fromUtf8("刷新个人信息"),
            profileCard,
            "secondary"
        );

    refreshButton->setMaximumWidth(155);

    profileLayout->addWidget(
        refreshButton,
        0,
        Qt::AlignRight
    );

    bodyLayout->addWidget(profileCard, 3);

    QFrame *summaryCard =
        createSectionCard(page);

    QVBoxLayout *summaryLayout =
        new QVBoxLayout(summaryCard);

    summaryLayout->setContentsMargins(
        18, 18, 18, 18
    );

    summaryLayout->setSpacing(10);

    QLabel *summaryTitle =
        new QLabel(
            QString::fromUtf8("借阅概况"),
            summaryCard
        );

    summaryTitle->setFixedHeight(31);

    summaryTitle->setStyleSheet(
        "QLabel {"
        "color:#36566F;"
        "border:none;"
        "font-size:19px;"
        "font-weight:bold;"
        "}"
    );

    summaryLayout->addWidget(summaryTitle);

    QGridLayout *summaryGrid =
        new QGridLayout;

    summaryGrid->setSpacing(10);

    summaryGrid->addWidget(
        createDataCard(
            QString::fromUtf8("当前借阅"),
            &m_profileBorrowingLabel,
            QString::fromUtf8("尚未归还"),
            "#EAF3FF",
            "#4F86C6",
            summaryCard
        ),
        0, 0
    );

    summaryGrid->addWidget(
        createDataCard(
            QString::fromUtf8("已经归还"),
            &m_profileReturnedLabel,
            QString::fromUtf8("历史归还"),
            "#E8F8F2",
            "#3A9D78",
            summaryCard
        ),
        0, 1
    );

    summaryGrid->addWidget(
        createDataCard(
            QString::fromUtf8("已经逾期"),
            &m_profileOverdueLabel,
            QString::fromUtf8("逾期未还"),
            "#FFEDEF",
            "#D85A68",
            summaryCard
        ),
        1, 0
    );

    summaryGrid->addWidget(
        createDataCard(
            QString::fromUtf8("累计罚款"),
            &m_profileFineLabel,
            QString::fromUtf8("逾期罚款"),
            "#F1EDFF",
            "#8066C9",
            summaryCard
        ),
        1, 1
    );

    summaryGrid->setColumnStretch(0, 1);
    summaryGrid->setColumnStretch(1, 1);

    summaryLayout->addLayout(
        summaryGrid,
        1
    );

    bodyLayout->addWidget(summaryCard, 2);
    mainLayout->addLayout(bodyLayout, 1);

    QFrame *ruleCard =
        new QFrame(page);

    ruleCard->setFixedHeight(59);

    ruleCard->setStyleSheet(
        "QFrame {"
        "background:#FFF8DF;"
        "border:1px solid #E8D89C;"
        "border-radius:10px;"
        "}"
    );

    QHBoxLayout *ruleLayout =
        new QHBoxLayout(ruleCard);

    ruleLayout->setContentsMargins(
        17, 9, 17, 9
    );

    QLabel *ruleTitle =
        new QLabel(
            QString::fromUtf8("借阅规则"),
            ruleCard
        );

    ruleTitle->setStyleSheet(
        "QLabel {"
        "color:#9A711C;"
        "border:none;"
        "font-size:16px;"
        "font-weight:bold;"
        "}"
    );

    QLabel *ruleText =
        new QLabel(
            QString::fromUtf8(
                "借阅期限1～60天　　"
                "逾期每天0.5元，最高20元　　"
                "存在逾期记录时不能继续借书"
            ),
            ruleCard
        );

    ruleText->setStyleSheet(
        "QLabel {"
        "color:#75642D;"
        "border:none;"
        "font-size:13px;"
        "}"
    );

    ruleLayout->addWidget(ruleTitle);
    ruleLayout->addWidget(ruleText, 1);

    mainLayout->addWidget(ruleCard);

    connect(
        refreshButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshReaderProfile())
    );

    refreshReaderProfile();

    return page;
}

void MainWindow::refreshReaderProfile()
{
    if (!m_profileAccountLabel) {
        return;
    }

    QString error;
    DBManager::instance()
        .syncOverdueStatus(error);

    QSqlQuery query(
        DBManager::instance().database()
    );

    query.prepare(
        "SELECT account,name,student_no,"
        "status,created_at "
        "FROM users "
        "WHERE account=? AND role='reader'"
    );

    query.addBindValue(m_account);

    if (!query.exec() ||
        !query.next()) {
        return;
    }

    QString studentNo =
        query.value(2).toString();

    if (studentNo.isEmpty()) {
        studentNo =
            QString::fromUtf8("未填写");
    }

    m_profileAccountLabel->setText(
        query.value(0).toString()
    );

    m_profileNameLabel->setText(
        query.value(1).toString()
    );

    m_profileStudentNoLabel->setText(
        studentNo
    );

    m_profileStatusLabel->setText(
        query.value(3).toString() == "normal"
            ? QString::fromUtf8("正常")
            : QString::fromUtf8("已禁用")
    );

    m_profileCreatedAtLabel->setText(
        formattedDateTime(query.value(4))
    );

    QSqlQuery countQuery(
        DBManager::instance().database()
    );

    countQuery.prepare(
        "SELECT "
        "COALESCE(SUM(CASE WHEN r.status IN "
        "('borrowed','overdue') "
        "THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE "
        "WHEN r.status='returned' "
        "THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE "
        "WHEN r.status='overdue' "
        "THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(r.fine),0) "
        "FROM borrow_records r "
        "JOIN users u ON r.user_id=u.user_id "
        "WHERE u.account=?"
    );

    countQuery.addBindValue(m_account);

    if (countQuery.exec() &&
        countQuery.next()) {

        m_profileBorrowingLabel->setText(
            countQuery.value(0).toString()
        );

        m_profileReturnedLabel->setText(
            countQuery.value(1).toString()
        );

        m_profileOverdueLabel->setText(
            countQuery.value(2).toString()
        );

        m_profileFineLabel->setText(
            QString::fromUtf8("￥%1")
                .arg(
                    countQuery.value(3).toDouble(),
                    0, 'f', 2
                )
        );
    }
}

/* ================= 逾期管理 ================= */

QWidget *MainWindow::createOverduePage()
{
    QWidget *page =
        new QWidget(m_stackedWidget);

    page->setStyleSheet(
        "background:#EEF3F8;"
    );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(page);

    mainLayout->setContentsMargins(
        22, 10, 22, 18
    );

    mainLayout->setSpacing(10);

    mainLayout->addWidget(
        createPageHeader(
            QString::fromUtf8("逾期管理"),
            QString::fromUtf8(
                "查看逾期记录和当前罚款"
            ),
            page
        )
    );

    QGridLayout *grid =
        new QGridLayout;

    grid->setSpacing(12);

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("逾期记录"),
            &m_overduePageCard1,
            QString::fromUtf8("全部逾期记录"),
            "#FFEDEF",
            "#D85A68",
            page
        ),
        0, 0
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("涉及读者"),
            &m_overduePageCard2,
            QString::fromUtf8("存在逾期读者"),
            "#FFF4E6",
            "#E99A3E",
            page
        ),
        0, 1
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("当前罚款"),
            &m_overduePageCard3,
            QString::fromUtf8("逾期罚款总计"),
            "#FFF8DF",
            "#D09A24",
            page
        ),
        0, 2
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("最长逾期"),
            &m_overduePageCard4,
            QString::fromUtf8("最大逾期天数"),
            "#F1EDFF",
            "#8066C9",
            page
        ),
        0, 3
    );

    for (int i = 0; i < 4; ++i) {
        grid->setColumnStretch(i, 1);
    }

    mainLayout->addLayout(grid);

    QFrame *card =
        createSectionCard(page);

    QVBoxLayout *cardLayout =
        new QVBoxLayout(card);

    cardLayout->setContentsMargins(
        14, 12, 14, 14
    );

    QHBoxLayout *searchLayout =
        new QHBoxLayout;

    m_overdueSearchEdit =
        new QLineEdit(card);

    m_overdueSearchEdit->setPlaceholderText(
        QString::fromUtf8(
            "输入账号、姓名、书名或ISBN查询"
        )
    );

    styleSearchEdit(m_overdueSearchEdit);

    QPushButton *searchButton =
        createActionButton(
            QString::fromUtf8("查询"),
            card
        );

    QPushButton *refreshButton =
        createActionButton(
            QString::fromUtf8("刷新"),
            card,
            "secondary"
        );

    searchLayout->addWidget(
        m_overdueSearchEdit, 1
    );

    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(refreshButton);

    cardLayout->addLayout(searchLayout);

    QHBoxLayout *actions =
        new QHBoxLayout;

    actions->addStretch();

    QPushButton *remindButton =
        createActionButton(
            QString::fromUtf8("生成逾期提醒"),
            card,
            "warning"
        );

    actions->addWidget(remindButton);
    cardLayout->addLayout(actions);

    m_overdueTable =
        new QTableView(card);

    setupTable(m_overdueTable);

    m_overdueModel =
        new QSqlQueryModel(this);

    m_overdueTable->setModel(
        m_overdueModel
    );

    cardLayout->addWidget(
        m_overdueTable, 1
    );

    mainLayout->addWidget(card, 1);

    connect(
        searchButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshOverdueRecords())
    );

    connect(
        refreshButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshOverdueRecords())
    );

    connect(
        remindButton,
        SIGNAL(clicked()),
        this,
        SLOT(remindSelectedReader())
    );

    refreshOverdueRecords();

    return page;
}

void MainWindow::refreshOverdueRecords()
{
    if (!m_overdueModel || !m_overdueTable) {
        return;
    }

    QString syncError;
    DBManager::instance().syncOverdueStatus(syncError);

    QString key;
    if (m_overdueSearchEdit) {
        key = m_overdueSearchEdit->text().trimmed();
    }

    QSqlQuery query(DBManager::instance().database());

    QString sql =
        "SELECT "
        "ROW_NUMBER() OVER(ORDER BY r.record_id ASC) AS display_no,"
        "u.account,u.name,u.student_no,b.isbn,b.title,"
        "r.borrow_date,r.due_date,"
        "GREATEST(DATEDIFF(NOW(),r.due_date),0),"
        "r.fine,r.record_id "
        "FROM borrow_records r "
        "JOIN users u ON r.user_id=u.user_id "
        "JOIN books b ON r.book_id=b.book_id "
        "WHERE r.status='overdue' ";

    if (!key.isEmpty()) {
        sql +=
            "AND (u.account LIKE ? "
            "OR u.name LIKE ? "
            "OR u.student_no LIKE ? "
            "OR b.isbn LIKE ? "
            "OR b.title LIKE ?) ";
    }

    sql += "ORDER BY r.record_id ASC";
    query.prepare(sql);

    if (!key.isEmpty()) {
        const QString likeKey = "%" + key + "%";
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
        query.addBindValue(likeKey);
    }

    if (!query.exec()) {
        QMessageBox::warning(this, QString::fromUtf8("查询失败"), query.lastError().text());
        return;
    }

    m_overdueModel->setQuery(query);

    const QString headers[] = {
        QString::fromUtf8("记录号"),
        QString::fromUtf8("读者账号"),
        QString::fromUtf8("读者姓名"),
        QString::fromUtf8("学号"),
        QString::fromUtf8("ISBN"),
        QString::fromUtf8("书名"),
        QString::fromUtf8("借阅时间"),
        QString::fromUtf8("应还时间"),
        QString::fromUtf8("逾期天数"),
        QString::fromUtf8("当前罚款/元"),
        QString::fromUtf8("真实记录ID")
    };

    for (int i = 0; i < 11; ++i) {
        m_overdueModel->setHeaderData(i, Qt::Horizontal, headers[i]);
    }

    m_overdueTable->setColumnHidden(10, true);
    m_overdueTable->resizeColumnsToContents();

    QSqlQuery countQuery(DBManager::instance().database());
    countQuery.exec(
        "SELECT COUNT(*),COUNT(DISTINCT user_id),"
        "COALESCE(SUM(fine),0),"
        "COALESCE(MAX(GREATEST(DATEDIFF(NOW(),due_date),0)),0) "
        "FROM borrow_records WHERE status='overdue'"
    );

    if (countQuery.next()) {
        if (m_overduePageCard1) m_overduePageCard1->setText(countQuery.value(0).toString());
        if (m_overduePageCard2) m_overduePageCard2->setText(countQuery.value(1).toString());
        if (m_overduePageCard3) {
            m_overduePageCard3->setText(QString::fromUtf8("￥%1").arg(countQuery.value(2).toDouble(), 0, 'f', 2));
        }
        if (m_overduePageCard4) {
            m_overduePageCard4->setText(QString::fromUtf8("%1天").arg(countQuery.value(3).toInt()));
        }
    }
}

/* ================= 统计页面 ================= */

QWidget *MainWindow::createStatisticsPage()
{
    QWidget *page =
        new QWidget(m_stackedWidget);

    page->setStyleSheet(
        "background:#EEF3F8;"
    );

    QVBoxLayout *mainLayout =
        new QVBoxLayout(page);

    mainLayout->setContentsMargins(
        22, 10, 22, 18
    );

    mainLayout->setSpacing(12);

    QHBoxLayout *headerLayout =
        new QHBoxLayout;

    headerLayout->addWidget(
        createPageHeader(
            QString::fromUtf8("统计查询"),
            QString::fromUtf8(
                "查看图书、读者、借阅、逾期和罚款数据"
            ),
            page
        ),
        1
    );

    QPushButton *refreshButton =
        createActionButton(
            QString::fromUtf8("刷新统计"),
            page,
            "secondary"
        );

    refreshButton->setMaximumWidth(125);

    headerLayout->addWidget(
        refreshButton,
        0,
        Qt::AlignVCenter
    );

    mainLayout->addLayout(headerLayout);

    QGridLayout *grid =
        new QGridLayout;

    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(14);

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("图书种类"),
            &m_bookKindsLabel,
            QString::fromUtf8("馆藏不同图书"),
            "#EAF3FF",
            "#4F86C6",
            page
        ),
        0, 0
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("图书总册数"),
            &m_bookTotalLabel,
            QString::fromUtf8("馆藏全部册数"),
            "#EEF0FF",
            "#6674D9",
            page
        ),
        0, 1
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("当前可借册数"),
            &m_availableLabel,
            QString::fromUtf8("目前可以借阅"),
            "#E8F8F2",
            "#3A9D78",
            page
        ),
        0, 2
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("普通读者数"),
            &m_readerCountLabel,
            QString::fromUtf8("已注册读者"),
            "#FFF4E6",
            "#E99A3E",
            page
        ),
        0, 3
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("未归还数量"),
            &m_borrowedLabel,
            QString::fromUtf8("借阅中和逾期"),
            "#F1EDFF",
            "#8066C9",
            page
        ),
        1, 0
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("逾期未还数量"),
            &m_overdueLabel,
            QString::fromUtf8("超过应还时间"),
            "#FFEDEF",
            "#D85A68",
            page
        ),
        1, 1
    );

    grid->addWidget(
        createDataCard(
            QString::fromUtf8("累计逾期罚款"),
            &m_totalFineLabel,
            QString::fromUtf8(
                "全部记录产生的罚款"
            ),
            "#FFF8DF",
            "#D09A24",
            page
        ),
        1, 2, 1, 2
    );

    for (int i = 0; i < 4; ++i) {
        grid->setColumnStretch(i, 1);
    }

    /*
     * 不给网格添加拉伸系数。
     * 统计卡片会紧跟标题显示。
     */
    mainLayout->addLayout(grid);

    /*
     * 所有多余空间放到卡片下面。
     */
    mainLayout->addStretch();

    connect(
        refreshButton,
        SIGNAL(clicked()),
        this,
        SLOT(refreshStatistics())
    );

    refreshStatistics();

    return page;
}

void MainWindow::refreshStatistics()
{
    if (!m_bookKindsLabel) {
        return;
    }

    QString error;
    DBManager::instance()
        .syncOverdueStatus(error);

    QSqlQuery query(
        DBManager::instance().database()
    );

    if (query.exec(
            "SELECT COUNT(*),"
            "COALESCE(SUM(total_count),0),"
            "COALESCE(SUM(available_count),0) "
            "FROM books"
        ) &&
        query.next()) {

        m_bookKindsLabel->setText(
            query.value(0).toString()
        );

        m_bookTotalLabel->setText(
            query.value(1).toString()
        );

        m_availableLabel->setText(
            query.value(2).toString()
        );
    }

    if (query.exec(
            "SELECT COUNT(*) "
            "FROM users WHERE role='reader'"
        ) &&
        query.next()) {

        m_readerCountLabel->setText(
            query.value(0).toString()
        );
    }

    if (query.exec(
            "SELECT COUNT(*) "
            "FROM borrow_records "
            "WHERE status IN "
            "('borrowed','overdue')"
        ) &&
        query.next()) {

        m_borrowedLabel->setText(
            query.value(0).toString()
        );
    }

    if (query.exec(
            "SELECT COUNT(*) "
            "FROM borrow_records "
            "WHERE status='overdue'"
        ) &&
        query.next()) {

        m_overdueLabel->setText(
            query.value(0).toString()
        );
    }

    if (query.exec(
            "SELECT COALESCE(SUM(fine),0) "
            "FROM borrow_records"
        ) &&
        query.next()) {

        m_totalFineLabel->setText(
            QString::fromUtf8("￥%1")
                .arg(
                    query.value(0).toDouble(),
                    0, 'f', 2
                )
        );
    }
}

/* ================= 公共操作 ================= */

int MainWindow::selectedId(
    QTableView *table,
    int column,
    const QString &message
) const
{
    if (!table ||
        !table->currentIndex().isValid()) {

        QMessageBox::warning(
            const_cast<MainWindow *>(this),
            QString::fromUtf8("提示"),
            message
        );

        return -1;
    }

    int row =
        table->currentIndex().row();

    return table->model()
        ->index(row, column)
        .data()
        .toInt();
}

void MainWindow::addBook()
{
    bool ok = false;

    QString isbn =
        QInputDialog::getText(
            this,
            QString::fromUtf8("新增图书"),
            QString::fromUtf8("ISBN："),
            QLineEdit::Normal,
            QString(),
            &ok
        );

    if (!ok || isbn.trimmed().isEmpty()) {
        return;
    }

    QString title =
        QInputDialog::getText(
            this,
            QString::fromUtf8("新增图书"),
            QString::fromUtf8("书名："),
            QLineEdit::Normal,
            QString(),
            &ok
        );

    if (!ok || title.trimmed().isEmpty()) {
        return;
    }

    QString author =
        QInputDialog::getText(
            this,
            QString::fromUtf8("新增图书"),
            QString::fromUtf8("作者："),
            QLineEdit::Normal,
            QString(),
            &ok
        );

    if (!ok || author.trimmed().isEmpty()) {
        return;
    }

    QString publisher =
        QInputDialog::getText(
            this,
            QString::fromUtf8("新增图书"),
            QString::fromUtf8("出版社："),
            QLineEdit::Normal,
            QString(),
            &ok
        );

    if (!ok) {
        return;
    }

    QString category =
        QInputDialog::getText(
            this,
            QString::fromUtf8("新增图书"),
            QString::fromUtf8("分类："),
            QLineEdit::Normal,
            QString(),
            &ok
        );

    if (!ok) {
        return;
    }

    int count =
        QInputDialog::getInt(
            this,
            QString::fromUtf8("新增图书"),
            QString::fromUtf8("库存数量："),
            1, 1, 9999, 1, &ok
        );

    if (!ok) {
        return;
    }

    QString error;

    if (!DBManager::instance().addBook(
            isbn,
            title,
            author,
            publisher,
            category,
            count,
            error
        )) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("新增失败"),
            error
        );

        return;
    }

    refreshBooks();
    refreshHomeData();
}

void MainWindow::editBook()
{
    if (!m_bookTable ||
        !m_bookTable->currentIndex().isValid()) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("提示"),
            QString::fromUtf8(
                "请先选择一本图书。"
            )
        );

        return;
    }

    int row =
        m_bookTable->currentIndex().row();

    int bookId =
        m_bookModel
            ->index(row, 8)
            .data()
            .toInt();

    bool ok = false;

    QString isbn =
        QInputDialog::getText(
            this,
            QString::fromUtf8("修改图书"),
            QString::fromUtf8("ISBN："),
            QLineEdit::Normal,
            m_bookModel->index(row, 1)
                .data().toString(),
            &ok
        );

    if (!ok || isbn.trimmed().isEmpty()) {
        return;
    }

    QString title =
        QInputDialog::getText(
            this,
            QString::fromUtf8("修改图书"),
            QString::fromUtf8("书名："),
            QLineEdit::Normal,
            m_bookModel->index(row, 2)
                .data().toString(),
            &ok
        );

    if (!ok || title.trimmed().isEmpty()) {
        return;
    }

    QString author =
        QInputDialog::getText(
            this,
            QString::fromUtf8("修改图书"),
            QString::fromUtf8("作者："),
            QLineEdit::Normal,
            m_bookModel->index(row, 3)
                .data().toString(),
            &ok
        );

    if (!ok || author.trimmed().isEmpty()) {
        return;
    }

    QString publisher =
        QInputDialog::getText(
            this,
            QString::fromUtf8("修改图书"),
            QString::fromUtf8("出版社："),
            QLineEdit::Normal,
            m_bookModel->index(row, 4)
                .data().toString(),
            &ok
        );

    if (!ok) {
        return;
    }

    QString category =
        QInputDialog::getText(
            this,
            QString::fromUtf8("修改图书"),
            QString::fromUtf8("分类："),
            QLineEdit::Normal,
            m_bookModel->index(row, 5)
                .data().toString(),
            &ok
        );

    if (!ok) {
        return;
    }

    int count =
        QInputDialog::getInt(
            this,
            QString::fromUtf8("修改图书"),
            QString::fromUtf8("总库存："),
            m_bookModel->index(row, 6)
                .data().toInt(),
            0, 9999, 1, &ok
        );

    if (!ok) {
        return;
    }

    QString error;

    if (!DBManager::instance().updateBook(
            bookId,
            isbn,
            title,
            author,
            publisher,
            category,
            count,
            error
        )) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("修改失败"),
            error
        );

        return;
    }

    refreshBooks();
    refreshHomeData();
}

void MainWindow::deleteBook()
{
    int bookId =
        selectedId(
            m_bookTable,
            8,
            QString::fromUtf8(
                "请先选择一本图书。"
            )
        );

    if (bookId < 0) {
        return;
    }

    if (QMessageBox::question(
            this,
            QString::fromUtf8("确认删除"),
            QString::fromUtf8(
                "确定删除所选图书吗？"
            ),
            QMessageBox::Yes |
            QMessageBox::No,
            QMessageBox::No
        ) != QMessageBox::Yes) {

        return;
    }

    QString error;

    if (!DBManager::instance().deleteBook(
            bookId,
            error
        )) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("删除失败"),
            error
        );

        return;
    }

    refreshBooks();
    refreshHomeData();
}

void MainWindow::readerBorrowSelectedBook()
{
    int bookId =
        selectedId(
            m_bookTable,
            8,
            QString::fromUtf8(
                "请先选择一本图书。"
            )
        );

    if (bookId < 0) {
        return;
    }

    int row =
        m_bookTable->currentIndex().row();

    QString title =
        m_bookModel->index(row, 2)
            .data().toString();

    int available =
        m_bookModel->index(row, 7)
            .data().toInt();

    if (available <= 0) {
        QMessageBox::warning(
            this,
            QString::fromUtf8("无法借阅"),
            QString::fromUtf8(
                "该图书没有可借库存。"
            )
        );

        return;
    }

    bool ok = false;

    int days =
        QInputDialog::getInt(
            this,
            QString::fromUtf8("借阅天数"),
            QString::fromUtf8(
                "请输入借阅天数（1至60天）："
            ),
            30, 1, 60, 1, &ok
        );

    if (!ok) {
        return;
    }

    QString error;

    if (!DBManager::instance().borrowBook(
            m_account,
            bookId,
            days,
            error
        )) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("借阅失败"),
            error
        );

        return;
    }

    QMessageBox::information(
        this,
        QString::fromUtf8("借阅成功"),
        QString::fromUtf8(
            "《%1》借阅成功。\n借阅期限：%2天"
        )
        .arg(title)
        .arg(days)
    );

    refreshBooks();
    refreshMyBorrowRecords();
    refreshHomeData();
}

void MainWindow::borrowBook()
{
    bool ok = false;

    QString account =
        QInputDialog::getText(
            this,
            QString::fromUtf8("办理借书"),
            QString::fromUtf8("读者账号："),
            QLineEdit::Normal,
            QString(),
            &ok
        );

    if (!ok || account.trimmed().isEmpty()) {
        return;
    }

    int bookId =
        QInputDialog::getInt(
            this,
            QString::fromUtf8("办理借书"),
            QString::fromUtf8("图书编号："),
            1, 1, 999999, 1, &ok
        );

    if (!ok) {
        return;
    }

    int days =
        QInputDialog::getInt(
            this,
            QString::fromUtf8("办理借书"),
            QString::fromUtf8(
                "借阅天数（1至60天）："
            ),
            30, 1, 60, 1, &ok
        );

    if (!ok) {
        return;
    }

    QString error;

    if (!DBManager::instance().borrowBook(
            account,
            bookId,
            days,
            error
        )) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("借阅失败"),
            error
        );

        return;
    }

    QMessageBox::information(
        this,
        QString::fromUtf8("借阅成功"),
        QString::fromUtf8(
            "借阅办理成功。"
        )
    );

    refreshBorrowRecords();
    refreshBooks();
    refreshHomeData();
}

void MainWindow::returnBook()
{
    int recordId = selectedId(
        m_borrowTable,
        12,
        QString::fromUtf8("请先选择一条借阅记录。")
    );

    if (recordId < 0) {
        return;
    }

    if (!m_borrowTable || !m_borrowModel ||
        !m_borrowTable->currentIndex().isValid()) {
        return;
    }

    const int row = m_borrowTable->currentIndex().row();
    const QString status = m_borrowModel->index(row, 9).data().toString();

    if (status == QString::fromUtf8("已归还")) {
        QMessageBox::warning(
            this,
            QString::fromUtf8("提示"),
            QString::fromUtf8("该借阅记录已经归还，不能重复办理还书。")
        );
        return;
    }

    const QString readerAccount = m_borrowModel->index(row, 1).data().toString();
    const QString readerName = m_borrowModel->index(row, 2).data().toString();
    const QString bookTitle = m_borrowModel->index(row, 4).data().toString();

    QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        QString::fromUtf8("确认还书"),
        QString::fromUtf8(
            "确定办理还书吗？\n\n"
            "读者账号：%1\n"
            "读者姓名：%2\n"
            "归还图书：《%3》"
        ).arg(readerAccount).arg(readerName).arg(bookTitle),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (answer != QMessageBox::Yes) {
        return;
    }

    QString errorMessage;
    double fine = 0.0;

    if (!DBManager::instance().returnBook(recordId, errorMessage, fine)) {
        QMessageBox::warning(this, QString::fromUtf8("归还失败"), errorMessage);
        return;
    }

    QMessageBox::information(
        this,
        QString::fromUtf8("归还成功"),
        QString::fromUtf8(
            "图书归还成功。\n\n"
            "读者：%1\n"
            "图书：《%2》\n"
            "产生罚款：￥%3"
        ).arg(readerName).arg(bookTitle).arg(fine, 0, 'f', 2)
    );

    refreshBorrowRecords();
    refreshBooks();
    refreshOverdueRecords();
    refreshHomeData();
    refreshStatistics();
}

void MainWindow::readerReturnSelectedBook()
{
    int recordId = selectedId(
        m_myBorrowTable,
        11,
        QString::fromUtf8("请先选择一条借阅记录。")
    );

    if (recordId < 0) {
        return;
    }

    if (!m_myBorrowTable || !m_myBorrowModel ||
        !m_myBorrowTable->currentIndex().isValid()) {
        return;
    }

    const int row = m_myBorrowTable->currentIndex().row();
    const QString status = m_myBorrowModel->index(row, 8).data().toString();

    if (status == QString::fromUtf8("已归还")) {
        QMessageBox::warning(
            this,
            QString::fromUtf8("提示"),
            QString::fromUtf8("该图书已经归还，不能重复办理还书。")
        );
        return;
    }

    const QString isbn = m_myBorrowModel->index(row, 1).data().toString();
    const QString title = m_myBorrowModel->index(row, 2).data().toString();
    const QString dueDate = formattedDateTime(m_myBorrowModel->index(row, 5).data());

    QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        QString::fromUtf8("确认还书"),
        QString::fromUtf8(
            "确定归还所选图书吗？\n\n"
            "ISBN：%1\n"
            "书名：《%2》\n"
            "应还时间：%3"
        ).arg(isbn).arg(title).arg(dueDate),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (answer != QMessageBox::Yes) {
        return;
    }

    QString errorMessage;
    double fine = 0.0;

    if (!DBManager::instance().returnBookByReader(recordId, m_account, errorMessage, fine)) {
        QMessageBox::warning(this, QString::fromUtf8("归还失败"), errorMessage);
        return;
    }

    QMessageBox::information(
        this,
        QString::fromUtf8("归还成功"),
        QString::fromUtf8(
            "图书《%1》归还成功。\n\n"
            "产生罚款：￥%2"
        ).arg(title).arg(fine, 0, 'f', 2)
    );

    refreshMyBorrowRecords();
    refreshBooks();
    refreshHomeData();
    refreshReaderProfile();
}

void MainWindow::disableReader()
{
    int userId =
        selectedId(
            m_readerTable,
            6,
            QString::fromUtf8(
                "请先选择读者。"
            )
        );

    if (userId < 0) {
        return;
    }

    QString error;

    if (!DBManager::instance()
            .updateReaderStatus(
                userId,
                "disabled",
                error
            )) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("操作失败"),
            error
        );

        return;
    }

    refreshReaders();
}

void MainWindow::enableReader()
{
    int userId =
        selectedId(
            m_readerTable,
            6,
            QString::fromUtf8(
                "请先选择读者。"
            )
        );

    if (userId < 0) {
        return;
    }

    QString error;

    if (!DBManager::instance()
            .updateReaderStatus(
                userId,
                "normal",
                error
            )) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("操作失败"),
            error
        );

        return;
    }

    refreshReaders();
}

void MainWindow::resetReaderPassword()
{
    int userId =
        selectedId(
            m_readerTable,
            6,
            QString::fromUtf8(
                "请先选择读者。"
            )
        );

    if (userId < 0) {
        return;
    }

    QString error;

    if (!DBManager::instance()
            .resetReaderPassword(
                userId,
                error
            )) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("操作失败"),
            error
        );

        return;
    }

    QMessageBox::information(
        this,
        QString::fromUtf8("重置成功"),
        QString::fromUtf8(
            "密码已重置为Library@123。"
        )
    );
}

void MainWindow::deleteReaderAccount()
{
    if (!m_readerTable
        || !m_readerTable->currentIndex().isValid()) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("提示"),
            QString::fromUtf8(
                "请先选择需要删除的读者账号。"
            )
        );

        return;
    }

    const int row =
        m_readerTable->currentIndex().row();

    const int userId =
        m_readerModel->index(row, 0)
            .data().toInt();

    const QString account =
        m_readerModel->index(row, 1)
            .data().toString();

    const QString name =
        m_readerModel->index(row, 2)
            .data().toString();

    const QMessageBox::StandardButton answer =
        QMessageBox::warning(
            this,
            QString::fromUtf8("确认删除账号"),
            QString::fromUtf8(
                "确定永久删除该读者账号吗？\n\n"
                "账号：%1\n"
                "姓名：%2\n\n"
                "删除后：\n"
                "1. 该账号无法继续登录；\n"
                "2. 该读者的全部借阅历史会被删除；\n"
                "3. 未归还图书会自动恢复库存。\n\n"
                "该操作无法撤销。"
            )
            .arg(account)
            .arg(name),
            QMessageBox::Yes
            | QMessageBox::No,
            QMessageBox::No
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    QString errorMessage;

    if (!DBManager::instance()
             .deleteReaderAccount(
                 userId,
                 errorMessage
             )) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("删除失败"),
            errorMessage
        );

        return;
    }

    QMessageBox::information(
        this,
        QString::fromUtf8("删除成功"),
        QString::fromUtf8(
            "读者账号“%1”已经删除。"
        ).arg(account)
    );

    refreshReaders();
    refreshHomeData();
    refreshBorrowRecords();
    refreshOverdueRecords();
    refreshStatistics();
}

void MainWindow::remindSelectedReader()
{
    if (!m_overdueTable ||
        !m_overdueTable->currentIndex().isValid()) {

        QMessageBox::warning(
            this,
            QString::fromUtf8("提示"),
            QString::fromUtf8(
                "请先选择一条逾期记录。"
            )
        );

        return;
    }

    int row =
        m_overdueTable->currentIndex().row();

    QString message =
        QString::fromUtf8(
            "逾期归还提醒\n\n"
            "读者：%1\n"
            "账号：%2\n"
            "图书：《%3》\n"
            "应还时间：%4\n"
            "逾期天数：%5天\n"
            "当前罚款：￥%6\n\n"
            "请尽快归还图书。"
        )
        .arg(
            m_overdueModel->index(row, 2)
                .data().toString()
        )
        .arg(
            m_overdueModel->index(row, 1)
                .data().toString()
        )
        .arg(
            m_overdueModel->index(row, 5)
                .data().toString()
        )
        .arg(
            formattedDateTime(
                m_overdueModel->index(row, 7)
                    .data()
            )
        )
        .arg(
            m_overdueModel->index(row, 8)
                .data().toInt()
        )
        .arg(
            m_overdueModel->index(row, 9)
                .data().toDouble(),
            0, 'f', 2
        );

    QMessageBox::information(
        this,
        QString::fromUtf8("逾期提醒"),
        message
    );
}

void MainWindow::showReaderOverdueReminder()
{
    QString error;

    DBManager::instance()
        .syncOverdueStatus(error);

    QSqlQuery query(
        DBManager::instance().database()
    );

    query.prepare(
        "SELECT b.title,r.due_date,"
        "GREATEST(DATEDIFF(NOW(),r.due_date),0),"
        "r.fine "
        "FROM borrow_records r "
        "JOIN users u ON r.user_id=u.user_id "
        "JOIN books b ON r.book_id=b.book_id "
        "WHERE u.account=? "
        "AND r.status='overdue'"
    );

    query.addBindValue(m_account);

    if (!query.exec()) {
        return;
    }

    QString text;
    int count = 0;

    while (query.next()) {
        ++count;

        text +=
            QString::fromUtf8(
                "《%1》\n"
                "应还时间：%2\n"
                "逾期：%3天\n"
                "罚款：￥%4\n\n"
            )
            .arg(query.value(0).toString())
            .arg(
                formattedDateTime(
                    query.value(1)
                )
            )
            .arg(query.value(2).toInt())
            .arg(
                query.value(3).toDouble(),
                0, 'f', 2
            );
    }

    if (count > 0) {
        QMessageBox::warning(
            this,
            QString::fromUtf8("逾期提醒"),
            QString::fromUtf8(
                "你有%1本图书已经逾期。\n\n"
            ).arg(count) + text
        );
    }
}

/* ================= 导航 ================= */

void MainWindow::setCurrentButton(
    const QString &buttonName
)
{
    const QString normal =
        "QPushButton {"
        "background:transparent;"
        "color:#EAF0F5;"
        "border:none;"
        "border-radius:7px;"
        "padding:10px 16px;"
        "text-align:left;"
        "font-size:14px;"
        "}"
        "QPushButton:hover {"
        "background:#46667E;"
        "}";

    const QString selected =
        "QPushButton {"
        "background:#5D86A2;"
        "color:white;"
        "border:none;"
        "border-radius:7px;"
        "padding:10px 16px;"
        "text-align:left;"
        "font-size:14px;"
        "font-weight:bold;"
        "}";

    ui->homeButton->setStyleSheet(normal);
    ui->bookButton->setStyleSheet(normal);
    ui->readerButton->setStyleSheet(normal);
    ui->borrowButton->setStyleSheet(normal);
    ui->statisticsButton->setStyleSheet(normal);

    if (m_overdueButton) {
        m_overdueButton->setStyleSheet(normal);
    }

    if (buttonName == "home") {
        ui->homeButton->setStyleSheet(selected);
    } else if (buttonName == "book") {
        ui->bookButton->setStyleSheet(selected);
    } else if (buttonName == "reader") {
        ui->readerButton->setStyleSheet(selected);
    } else if (buttonName == "borrow") {
        ui->borrowButton->setStyleSheet(selected);
    } else if (buttonName == "overdue") {
        m_overdueButton->setStyleSheet(selected);
    } else if (buttonName == "statistics") {
        ui->statisticsButton->setStyleSheet(selected);
    }
}

void MainWindow::showHomePage()
{
    refreshHomeData();
    m_stackedWidget->setCurrentIndex(0);
    setCurrentButton("home");
}

void MainWindow::showBookPage()
{
    refreshBooks();
    m_stackedWidget->setCurrentIndex(1);
    setCurrentButton("book");
}

void MainWindow::showReaderPage()
{
    if (m_role == "admin") {
        refreshReaders();
    } else {
        refreshMyBorrowRecords();
    }

    m_stackedWidget->setCurrentIndex(2);
    setCurrentButton("reader");
}

void MainWindow::showBorrowPage()
{
    if (m_role == "admin") {
        refreshBorrowRecords();
    } else {
        refreshReaderProfile();
    }

    m_stackedWidget->setCurrentIndex(3);
    setCurrentButton("borrow");
}

void MainWindow::showOverduePage()
{
    if (m_role != "admin") {
        return;
    }

    refreshOverdueRecords();
    m_stackedWidget->setCurrentIndex(4);
    setCurrentButton("overdue");
}

void MainWindow::showStatisticsPage()
{
    if (m_role != "admin") {
        return;
    }

    refreshStatistics();
    m_stackedWidget->setCurrentIndex(5);
    setCurrentButton("statistics");
}

void MainWindow::handleLogout()
{
    if (QMessageBox::question(
            this,
            QString::fromUtf8("退出登录"),
            QString::fromUtf8(
                "确定退出当前账号吗？"
            ),
            QMessageBox::Yes |
            QMessageBox::No,
            QMessageBox::No
        ) == QMessageBox::Yes) {

        emit logoutRequested();
    }
}
