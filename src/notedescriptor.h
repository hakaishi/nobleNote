#ifndef NOTEDESCRIPTOR_H
#define NOTEDESCRIPTOR_H

#include <QObject>
#include "xmlnotereader.h"
#include "xmlnotewriter.h"
#include <QTextDocument>
#include "textdocument.h"

/**
 * @brief The NoteDescriptor class provides an abstraction layer to the underlying note
 * file on the filesystem
 * and also manages automatic saving and reloading of note files
 *
 */

class NoteDescriptor : public QObject
{
    Q_OBJECT
public:
    explicit NoteDescriptor(QString filePath, TextDocument *document, QWidget *noteWidget = 0);
    
signals:
    void close(); // emitted if the user wants to close the note via a message box
    
public slots:
    void stateChange();

private slots:
    void setActivityIdle();

private:

    enum
    {
        Idle,
        CheckFilePath,
        CheckLastChange,
        ProcessEvents
    } Activity;

    void save(const QString &filePath, QUuid uuid); // save modified document to file
    void load(const QString& filePath); // load a note file into the document

    QWidget * noteWidget_;
    QString filePath_;
    TextDocument * document_;
    QDateTime lastChange_;
    QDateTime createDate_;
    QDateTime lastMetadataChange_;
    QUuid uuid_;
};

#endif // NOTEDESCRIPTOR_H
