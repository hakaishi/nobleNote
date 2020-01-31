#include "systemtraycreator.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QListIterator>
#include <QMenu>

SystemTrayCreator::SystemTrayCreator(QObject *parent) : QObject(parent)
{

}

void SystemTrayCreator::populateMenu(QMenu * menu)
{

    menu->clear();

    QString rootPath = QSettings().value("root_path").toString();


    QListIterator<QFileInfo> it(QDir(rootPath).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                                                             QDir::Name | QDir::IgnoreCase));
    while (it.hasNext()) {

        QFileInfo nextThing = it.next();

        QDir nextDir(nextThing.filePath());

        QMenu* notebookMenu = menu->addMenu(nextDir.dirName());

        QFileInfoList entries = nextDir.entryInfoList(QDir::Files,QDir::Name | QDir::IgnoreCase);

        QListIterator<QFileInfo> sit(entries);

        while (sit.hasNext()) {
            QFileInfo nextNote = sit.next();
            QAction* action = notebookMenu->addAction(nextNote.fileName());
            connect(action,&QAction::triggered,menu,[this,nextNote](){ this->noteClicked(nextNote.filePath());});
        }

    }
}
