#include "noteimporter.h"
#include <QFileDialog>
#include <QSettings>
#include <QWidget>
#include <QtConcurrentMap>

NoteImporter::NoteImporter(QObject *parent) :
    QObject(parent)
{
    parentWidget = this->parent()->isWidgetType() ? qobject_cast<QWidget*>(this->parent()) : 0;
}

void NoteImporter::importDialog()
{
     if(fileDialog)
       return;

     importFiles.clear(); //remove old files

     fileDialog = new QFileDialog(parentWidget, tr("Select one or more tomboy or gnote notes"),
                      QSettings().value("import_path").toString(), tr("Notes")+"(*.note)");
     fileDialog->setViewMode(QFileDialog::Detail);
     fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
     fileDialog->setFileMode(QFileDialog::ExistingFiles);
     fileDialog->setOption(QFileDialog::ReadOnly);
     fileDialog->show();
     QObject::connect(fileDialog, SIGNAL(accepted()), this, SLOT(importXmlNotes()));
     QObject::connect(fileDialog, SIGNAL(rejected()), fileDialog, SLOT(deleteLater()));
}

void NoteImporter::importXmlNotes()
{
     importFiles = fileDialog->selectedFiles();
     if(importFiles.isEmpty())
        return;

     QSettings().setValue("import_path",QFileInfo(importFiles.last()).absolutePath());

     dialog = new QProgressDialog(parentWidget);
     dialog->setLabelText(QString(tr("Importing notes...")));

     progressReceiver = new ProgressReceiver(parentWidget);
     xml2HtmlFunctor.path = QSettings().value("root_path").toString();
     xml2HtmlFunctor.p = progressReceiver;

     futureWatcher = new QFutureWatcher<void>(parentWidget);
     futureWatcher->setFuture(QtConcurrent::map(importFiles, xml2HtmlFunctor));

     QObject::connect(progressReceiver,SIGNAL(valueChanged(int)),dialog, SLOT(setValue(int)));
     QObject::connect(futureWatcher, SIGNAL(finished()), dialog, SLOT(reset()));
     QObject::connect(dialog, SIGNAL(canceled()), futureWatcher, SLOT(cancel()));

     QList<QObject*> objects;
     objects << futureWatcher << dialog << progressReceiver << fileDialog;

     foreach(QObject * o, objects)
     {
         connect(futureWatcher, SIGNAL(canceled()),o,SLOT(deleteLater()));
         connect(futureWatcher, SIGNAL(finished()),o,SLOT(deleteLater()));
     }

//     QObject::connect(futureWatcher, SIGNAL(canceled()), futureWatcher, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(finished()), futureWatcher, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(canceled()), dialog, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(finished()), dialog, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(canceled()), progressReceiver, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(finished()), progressReceiver, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(canceled()), fileDialog, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(finished()), fileDialog, SLOT(deleteLater()));

     dialog->show();
}

