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
    explicit NoteDescriptor(QString filePath, TextDocument *document, QObject *parent = 0);
    
signals:
    
public slots:
    void stateChange();

private slots:
    void setStateChangeEnabled();

private:
    void save(); // save modified document to file
    void load(); // load a note file into the document

    QString filePath_;
    TextDocument * document_;
    QDateTime lastChange_;
    QDateTime createDate_;
    QDateTime lastMetadataChange_;
    QUuid uuid_;

    bool stateChangeEnabled;
};

#endif // NOTEDESCRIPTOR_H
