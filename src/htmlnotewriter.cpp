#include "htmlnotewriter.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "datetime.h"
#include "xmlnotereader.h"
#include <QDir>


HtmlNoteWriter::HtmlNoteWriter(const QString &filePath)
{
   document_ = 0;
   filePath_ = filePath;
}

void HtmlNoteWriter::write()
{

    if(!document_)
    {
        qDebug("HtmlNoteWriter::write failed : no QTextDocument set");
        return;
    }


    document_->setMetaInformation(QTextDocument::DocumentTitle,title_);

    QString html = document_->toHtml();

    // remove { } braces
    QString uuidStr = uuid_.toString().remove(0,1);
    uuidStr.chop(1);

    // insert meta elements
    insertMetaElement(&html,"width",QString::number(size_.width()));
    insertMetaElement(&html,"height",QString::number(size_.width()));
    insertMetaElement(&html,"cursor-position",QString::number(cursorPosition_));
    insertMetaElement(&html,"create-date",DateTime::toISO8601(lastChange_.isNull()? QDateTime::currentDateTime():lastChange_));
    insertMetaElement(&html,"last-change-date",DateTime::toISO8601(lastChange_.isNull()? QDateTime::currentDateTime():lastChange_));
    insertMetaElement(&html,"uuid",uuidStr);



    QFile file(filePath_);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug(qPrintable(QString("HtmlNoteWriter::write failed : could not open ") + filePath_));
           return;
    }

    QTextStream out(&file);
    out << html;
    file.close();
}

void HtmlNoteWriter::insertMetaElement(QString *html, const QString &name, const QString &content)
{
    QString metaLine("<meta name=\"" + name + "\" content=\"" + content + "\"/>");
    int headIdx = html->indexOf("<head>");
    html->insert(headIdx + qstrlen("<head>"),metaLine);
}

/*static*/ void HtmlNoteWriter::writeXml2Html(const QString &xmlFilePath, const QString &outputPath)
{
    if(xmlFilePath.isEmpty() || outputPath.isEmpty())
        return;

    QTextDocument document;
    XmlNoteReader reader(xmlFilePath,&document);

    QString folder;
    QString tag = reader.tag();

    QRegExp illegal("[" +QRegExp::escape("\\^/?<>:*|\"")+ "]|^(com\\d|lpt\\d|con|nul|prn)$");
    if(!tag.isEmpty())
    {
        //  takes 2nd colon, remove before
        //int colonIdx = tag.indexOf(":",tag.indexOf(":")+1); // 2nd index of :, e.g. "system:notebook:tagname"
        //folder = tag.right(tag.length()-colonIdx);
        tag.remove("system:notebook:");
        tag.remove("system:template");
        tag.remove(illegal);//remove illegal chars in filenames
        if(tag[0] == '.') // would be invisible if allowed
            tag.remove(0,1);
        folder = tag;
    }
    if(folder.isEmpty())
        folder = tr("default");

    QString title = reader.title();
    title.remove(illegal);
    if(title.isEmpty())
        title = tr("untitled note");

    QString filePath;
    QDir().mkpath(outputPath + "/" + folder);
    filePath =  outputPath + "/"+ folder + "/" + title;

    // TODO move this in extra static method
    int counter = 0;
    QString origPath = filePath;
    while(QFile::exists(filePath))
    {
        ++counter;
        filePath = origPath +  QString(" (%1)").arg(counter);
    }

    HtmlNoteWriter writer(filePath);
    writer.setDocument(&document);
    writer.setTitle(title);
    writer.setLastChange(reader.lastChange());
    writer.setLastMetadataChange(reader.lastMetadataChange());
    writer.setCreateDate(reader.createDate());
    writer.setUuid(reader.uuid());
    writer.write();
}
