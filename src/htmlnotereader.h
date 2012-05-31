#ifndef HTMLNOTEREADER_H
#define HTMLNOTEREADER_H

#include "abstractnotereader.h"
#include <QTextFrame>
#include <QDateTime>
#include <QUuid>
#include <QFile>
#include <QTextDocument>

/**
  * a class reading formatted html text and plain text
  * reading requires a QTextFrame
  *
  */

class HtmlNoteReader : public AbstractNoteReader
{
public:
     HtmlNoteReader(const QString &filePath, QTextDocument* doc = 0);


    void setDocument(QTextDocument * document)       {   document_ = document;}
    QTextDocument * document() const              { return document_;}

    // get the uuid that has been extracted during read()
    QUuid uuid() const                         { return uuid_;}

    const QString& title() const            { return title_;}

     // get last change date
    const QDateTime& lastChange() const         { return lastChange_;}

    // get last metadata change date
    const QDateTime& lastMetadataChange() const { return lastMetadataChange_;}

    // get create date
    const QDateTime& createDate() const         { return createDate_;}

//     // reads a uuid from a file, if uuid could not be found, a null uuid is returned
    static QUuid uuid(QString filePath);

//    // searches all directorys under the given path recursively for a file with the given UUID
//    // returns the first file that contains the given uuid or an empty string if the uuid could not be found or if the given uuid is null
    static QString findUuid(const QUuid uuid, const QString & path);

    // reads a uuid from the html contents of string
    static QUuid uuidFromHtml(const QString &html);

    // returns the text of the content attribute in a <meta name="foo" content="bar" /> element for the given name
    static QString metaContent(const QString &html, const QString &name);

private:
    void read(const QString &filePath); // read the content's of a QIODevice and write the formatted text into a QTextDocument
    static QUuid parseUuid(QString idStr);
    void readContent(); // read contents of <note-content> tag
    QString title_;
    QTextDocument * document_;
    QUuid uuid_;
    QDateTime lastChange_;
    QDateTime lastMetadataChange_;
    QDateTime createDate_;
};

#endif // HTMLNOTEREADER_H
