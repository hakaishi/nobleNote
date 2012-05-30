#include "htmlnotereader.h"
#include <QTextStream>
#include <QXmlStreamReader>
#include <QFileInfo>


HtmlNoteReader::HtmlNoteReader(const QString &filePath, QTextDocument *doc)
{
   document_ = doc;
   QFile file(filePath);
   if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
   {
       qDebug("HtmlNoteReader::HtmlNoteReader failed : could not open filepath");
          return;
   }

   QFileInfo info(filePath);
   lastChange_ = info.lastModified();
   lastMetadataChange_ = info.lastModified();
   createDate_ = info.created();

   read(&file);
   file.close(); // local object gets destroyed
}

// TODO note title can be stored via QTextDocument.setMetaInformation
void HtmlNoteReader::read(QIODevice * file)
{
    QString content;

    QTextStream in(file);
    content = in.readAll();
    if(content.isEmpty())
        return;

    if(document_)
    {
        document_->setHtml(content);
    }

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
        if(metaLine.contains("uuid"))
        {
            int idx = metaLine.lastIndexOf('\"');
            int beforeIdx = metaLine.lastIndexOf('\"',idx-1);
            QStringRef between = metaLine.toString().midRef(beforeIdx +1,(idx-beforeIdx+1) -2); // +1 and -1 to take the content between the " "
            uuid_ = QUuid(between.toString().trimmed());

            // simpler alternative
            // 32 + 4 is the lenght of an uuid
            //QStringRef uuidRef = metaLine.toString().midRef(idx-(32 + 4),32+4); // take 32+4 chars before the last "
            //qDebug(uuidRef.toAscii());
        }

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
}
