#include "htmlnotereader.h"

HtmlNoteReader::HtmlNoteReader()
{
    document_ = NULL;
}

HtmlNoteReader::HtmlNoteReader(const QString &filePath) : file(filePath)
{
   document_ = NULL;
   if(!file.open(QIODevice::ReadOnly))
   {
       qDebug("XmlNoteReader::XmlNoteReader failed : could not open filepath");
          return;
   }
}

// TODO simplify API,
// TODO note title can be stored via QTextDocument.setMetaInformation
void HtmlNoteReader::read()
{
    if(document_)
        document_->setHtml(QString());
}
