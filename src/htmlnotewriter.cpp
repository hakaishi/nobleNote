#include "htmlnotewriter.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "datetime.h"


HtmlNoteWriter::HtmlNoteWriter(const QString &filePath, QTextDocument *doc)
{
   document_ = doc;
   filePath_ = filePath;
}

void HtmlNoteWriter::write()
{

    if(!document_)
        return;

    document_->setMetaInformation(QTextDocument::DocumentTitle,title_);

    QString content = document_->toHtml();

    // remove { } braces
    QString uuidStr = uuid_.toString().remove(0,1);
    uuidStr.chop(1);

    QString metaUuid("<meta name=\"uuid\" content=\"" + uuidStr + "\"/>");

    QString metaLastChange("<meta name=\"last-change-date\" content=\"" +DateTime::toISO8601(lastChange_.isNull()? QDateTime::currentDateTime():lastChange_) + "\"/>");
    QString metaCreateDate("<meta name=\"create-date\" content=\"" +DateTime::toISO8601(createDate_.isNull()? QDateTime::currentDateTime():createDate_) + "\"/>");

    // insert meta elements
    int headIdx = content.indexOf("<head>");
    content.insert(headIdx + qstrlen("<head>"), metaCreateDate);
    content.insert(headIdx + qstrlen("<head>"), metaLastChange);
    content.insert(headIdx + qstrlen("<head>"), metaUuid);

    QFile file(filePath_);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug("HtmlNoteWriter::write failed : could not open filepath");
           return;
    }

    QTextStream out(&file);
    out << content;
    file.close();
}
