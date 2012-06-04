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
    const QString& filePath() const { return filePath_; } // return the current filePath
    bool readOnly() const { return readOnly_; }
    
signals:
    void close(); // emitted if the user wants to close the note via a message box
    
public slots:
    void stateChange();

private slots:
    void unlockStateChange();

private:

    void save(const QString &filePath, QUuid uuid); // calls write with note and backup
    void write(const QString &filePath, QUuid uuid); // write note file to disc
    void load(const QString& filePath); // load a note file into the document


    QWidget * noteWidget_;
    QString filePath_;
    TextDocument * document_;
    QDateTime lastChange_;
    QDateTime createDate_;
    //QDateTime lastMetadataChange_;
    QUuid uuid_;
    QString title_;
    bool readOnly_;

    // thread locking mechanism for stateChange(), because if a MessageBox opens inside stateChange()
    // stateChange() can still be called from events from the GUI-Thread
    struct Lock
    {
        Lock(){count++;}
        ~Lock(){count = count > 0 ? count-1 : count;}
        static bool isLocked();
        static int count;
    };

    Lock * initialLock;
};



#endif // NOTEDESCRIPTOR_H
