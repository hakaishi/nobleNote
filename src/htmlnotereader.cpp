#include "htmlnotereader.h"
#include <QTextStream>
#include <QXmlStreamReader>
#include <QFileInfo>
#include <QDirIterator>


HtmlNoteReader::HtmlNoteReader(const QString &filePath, QTextDocument *doc)
{
   document_ = doc;
   read(filePath);
}

// TODO note title can be stored via QTextDocument.setMetaInformation
// TODO fallback file info also for XmlNoteReader
void HtmlNoteReader::read(const QString& filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug("HtmlNoteReader::HtmlNoteReader failed : could not open filepath");
           return;
    }
    QString content;

    QTextStream in(&file);
    content = in.readAll();
    if(content.isEmpty())
        return;

    if(document_)
    {
        document_->setHtml(content);
    }

    uuid_ = uuidFromHtml(content);
    lastChange_ = QDateTime::fromString(metaContent(content,"last-change-date"),Qt::ISODate);
    createDate_ = QDateTime::fromString(metaContent(content,"create-date"),Qt::ISODate);
    //lastMetadataChange_ = info.lastModified(); // not implemented

    // fallback dates
    QFileInfo info(filePath);
    if(lastChange_.isNull())
        lastChange_ = info.lastModified();

     if(createDate_.isNull())
        createDate_ = info.created();

    file.close(); // local object gets destroyed
}

/*QUuid*/ QUuid HtmlNoteReader::uuid(QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        return QUuid();
    }
    QString content;
    QTextStream in(&file);
    content = in.readAll();
    file.close();
    return uuidFromHtml(content);
}

/*static*/ QString HtmlNoteReader::findUuid(const QUuid uuid, const QString & path)
{
    if(uuid.isNull())
        return QString();

    QDirIterator it(path, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
              QString filePath = it.next();
                  if(uuid == HtmlNoteReader::uuid(filePath))
                      return filePath;
    }
    return QString();
}

/*static*/ QUuid HtmlNoteReader::uuidFromHtml(const QString& html)
{
    return QUuid(metaContent(html,"uuid").trimmed());
}

QString HtmlNoteReader::metaContent(const QString &html, const QString &name)
{
    QString content = html;

    int metaIdx = 0;
    int endIdx = 0;
    QTime time;
    time.start();
    // find the meta elements in the html files and read the uuid
    while(metaIdx != -1 || time.elapsed() > 500)
    {
        metaIdx = content.indexOf("<meta",endIdx+1);

        if(metaIdx==-1)
            break;

        endIdx = content.indexOf(">",metaIdx+1);
        QStringRef metaLine = content.midRef(metaIdx,endIdx-metaIdx+1); // e.g. <meta name="qrichtext" content="1" />
        if(metaLine.contains(name))
        {
            int idx = metaLine.lastIndexOf('\"');
            int beforeIdx = metaLine.lastIndexOf('\"',idx-1);
            QStringRef between = metaLine.toString().midRef(beforeIdx +1,(idx-beforeIdx+1) -2); // +1 and -1 to take the content between the " "
            return between.toString();
        }
    }
    return QString();
}





// does not work because QXmlStreamReader sometimes reports errors with html
    // TODO same device for both QTextStreama and QXmlStreamReader allowed?
//    QXmlStreamReader reader(file);

//    while(!reader.atEnd())
//    {

//        QXmlStreamReader::TokenType type = reader.readNext();
//        qDebug(reader.name().toAscii());
//        if(type == QXmlStreamReader::StartElement && reader.name() == "meta")
//        {
//                QXmlStreamAttributes attributes = reader.attributes();

//                if(!attributes.isEmpty())
//                {
//                    if((attributes.value("name") == "uuid" || attributes.value("name") == "id") && attributes.hasAttribute("content"))
//                        uuid_ = QUuid(attributes.value("content").toString());

//                }
//        }

//        if (reader.hasError())
//        {
//            qDebug("XmlNoteReader::read failed: Error reading xml content");
//            return;
//        }
//    }