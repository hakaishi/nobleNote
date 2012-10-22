#ifndef NOTEIMPORTER_H
#define NOTEIMPORTER_H

#include <QObject>
#include <QPointer>
#include <QFutureWatcher>
#include "htmlnotewriter.h"
#include "progressreceiver.h"
#include <QProgressDialog>
#include <QFileDialog>

class NoteImporter : public QObject
{
    Q_OBJECT
public:
    explicit NoteImporter(QObject *parent = 0);
    
public slots:
    void importDialog();

signals:
    
private slots:
    void importXmlNotes();

private:
    struct Xml2HtmlFunctor
    {
         ProgressReceiver *p;
         QString path;
         void operator()(const QString &file)
         {
              HtmlNoteWriter::writeXml2Html(file,path);
              p->postProgressEvent();
         }
    };

    Xml2HtmlFunctor xml2HtmlFunctor;

    QProgressDialog *dialog;
    ProgressReceiver *progressReceiver;
    QFutureWatcher<void> *futureWatcher;
    QStringList     importFiles;
    QPointer<QFileDialog> fileDialog;
    QWidget * parentWidget;
    
};

#endif // NOTEIMPORTER_H
