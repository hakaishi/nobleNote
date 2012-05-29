#ifndef NOTEDESCRIPTOR_H
#define NOTEDESCRIPTOR_H

#include <QObject>
#include "xmlnotereader.h"
#include "xmlnotewriter.h"
#include <QTextDocument>

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
    explicit NoteDescriptor(QString filePath, QTextDocument *document, QObject *parent = 0);
    
signals:
    
public slots:
    void stateChange();

private:

    QString filePath_;
    QTextDocument * document_;
    XmlNoteReader reader;
    XmlNoteWriter writer;
    QDateTime lastChange_;
    QUuid uuid_;
};

#endif // NOTEDESCRIPTOR_H
