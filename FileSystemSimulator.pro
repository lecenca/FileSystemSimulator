#-------------------------------------------------
#
# Project created by QtCreator 2017-08-26T15:59:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileSystemSimulator
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    fileoperator.cpp \
    disk.cpp \
    fileiter.cpp \
    openedlistitem.cpp \
    contentitem.cpp \
    createfiledialog.cpp \
    textdialog.cpp

HEADERS += \
        mainwindow.h \
    contentitem.h \
    fileoperator.h \
    disk.h \
    fileiter.h \
    openedlistitem.h \
    option.h \
    createfiledialog.h \
    textdialog.h

FORMS += \
        mainwindow.ui \
    createfiledialog.ui \
    textdialog.ui

RESOURCES += \
    rec.qrc

