#include "systemtraycreator.h"
#include <QSettings>
#include <QDirIterator>
#include <QFileInfo>
#include <QDir>
#include <QListIterator>
#include <QMenu>

SystemTrayCreator::SystemTrayCreator(QObject *parent) : QObject(parent)
{

}

QMenu *SystemTrayCreator::createMenu()
{
    QMenu * menu = new  QMenu();


    QString rootPath = QSettings().value("root_path").toString();


    QDirIterator it(rootPath, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    while (it.hasNext()) {

        QString nextThing = it.next();

        QDir nextDir(nextThing);

        QMenu* notebookMenu = menu->addMenu(nextDir.dirName());

        QFileInfoList entries = nextDir.entryInfoList(QDir::Files,QDir::Name);

        QListIterator<QFileInfo> sit(entries);

        while (sit.hasNext()) {
            QFileInfo nextNote = sit.next();
            QAction* action = notebookMenu->addAction(nextNote.fileName());
            connect(action,&QAction::triggered,menu,[this,nextNote](){ this->noteClicked(nextNote.filePath());});
        }

    }

    return menu;

}
