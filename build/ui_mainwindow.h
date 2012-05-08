/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Tue May 8 22:42:25 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_NobleNote
{
public:
    QAction *actionOpen;
    QAction *actionNew;
    QAction *actionSave;
    QAction *actionQuit;
    QAction *action_Open;
    QAction *action_Import;
    QAction *action_Configure;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menu_Settings;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *NobleNote)
    {
        if (NobleNote->objectName().isEmpty())
            NobleNote->setObjectName(QString::fromUtf8("NobleNote"));
        NobleNote->resize(280, 331);
        actionOpen = new QAction(NobleNote);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionNew = new QAction(NobleNote);
        actionNew->setObjectName(QString::fromUtf8("actionNew"));
        actionSave = new QAction(NobleNote);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        actionQuit = new QAction(NobleNote);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        action_Open = new QAction(NobleNote);
        action_Open->setObjectName(QString::fromUtf8("action_Open"));
        action_Import = new QAction(NobleNote);
        action_Import->setObjectName(QString::fromUtf8("action_Import"));
        action_Configure = new QAction(NobleNote);
        action_Configure->setObjectName(QString::fromUtf8("action_Configure"));
        centralwidget = new QWidget(NobleNote);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        NobleNote->setCentralWidget(centralwidget);
        menubar = new QMenuBar(NobleNote);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 280, 25));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menu_Settings = new QMenu(menubar);
        menu_Settings->setObjectName(QString::fromUtf8("menu_Settings"));
        NobleNote->setMenuBar(menubar);
        statusbar = new QStatusBar(NobleNote);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        NobleNote->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menu_Settings->menuAction());
        menuFile->addAction(action_Import);
        menuFile->addAction(actionQuit);
        menu_Settings->addAction(action_Configure);

        retranslateUi(NobleNote);

        QMetaObject::connectSlotsByName(NobleNote);
    } // setupUi

    void retranslateUi(QMainWindow *NobleNote)
    {
        NobleNote->setWindowTitle(QApplication::translate("NobleNote", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionOpen->setText(QApplication::translate("NobleNote", "&Open", 0, QApplication::UnicodeUTF8));
        actionNew->setText(QApplication::translate("NobleNote", "&New", 0, QApplication::UnicodeUTF8));
        actionSave->setText(QApplication::translate("NobleNote", "&Save", 0, QApplication::UnicodeUTF8));
        actionQuit->setText(QApplication::translate("NobleNote", "&Quit", 0, QApplication::UnicodeUTF8));
        action_Open->setText(QApplication::translate("NobleNote", "&Open", 0, QApplication::UnicodeUTF8));
        action_Import->setText(QApplication::translate("NobleNote", "&Import", 0, QApplication::UnicodeUTF8));
        action_Configure->setText(QApplication::translate("NobleNote", "&Configure...", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("NobleNote", "&File", 0, QApplication::UnicodeUTF8));
        menu_Settings->setTitle(QApplication::translate("NobleNote", "&Settings", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class NobleNote: public Ui_NobleNote {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
