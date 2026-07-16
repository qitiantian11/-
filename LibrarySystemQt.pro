QT += core gui widgets sql

CONFIG += c++11

TEMPLATE = app
TARGET = LibrarySystemQt

SOURCES += \
    main.cpp \
    dbmanager.cpp \
    loginwindow.cpp \
    registerdialog.cpp \
    mainwindow.cpp

HEADERS += \
    dbmanager.h \
    loginwindow.h \
    registerdialog.h \
    mainwindow.h

FORMS += \
    loginwindow.ui \
    registerdialog.ui \
    mainwindow.ui
