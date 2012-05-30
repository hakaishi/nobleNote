#include "htmlnotewriter.h"
#include <QFile>
#include <QTextStream>


HtmlNoteWriter::HtmlNoteWriter(const QString &filePath, QTextDocument *doc) : file(filePath)
{
   document_ = doc;
   if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
   {
       qDebug("HtmlNoteWriter::HtmlNoteReader failed : could not open filepath");
          return;
   }
}

void HtmlNoteWriter::write()
{
    if(!file.isOpen())
    {
        qDebug("HtmlNoteWriter::write failed : file not open");
        return;
    }

    QString content = document_->toHtml();

    // remove { } braces
    QString uuidStr = uuid_.toString().remove(0,1);
    uuidStr.chop(1);

    QString metaElement("<meta name=\"uuid\" content=\"" + uuidStr + "\"/>");

    int headIdx = content.indexOf("<head>");
    content.insert(headIdx + qstrlen("<head>"), metaElement);

    QTextStream out(&file);
    out << content;
}
