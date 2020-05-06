/* nobleNote, a note taking application
 * Copyright (C) 2020 Christian Metscher <hakaishi@web.de>,
                      Fabian Deuchler <Taiko000@gmail.com>

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * nobleNote is licensed under the MIT, see `http://copyfree.org/licenses/mit/license.txt'.
 */

#include "htmlnotereader.h"
#include <QTextStream>
#include <QXmlStreamReader>
#include <QFileInfo>
#include <QDirIterator>
#include <QTextDocument>


HtmlNoteReader::HtmlNoteReader(const QString &filePath)
{
   filePath_ = filePath;

}

void HtmlNoteReader::read()
{
    read(filePath_);
}

QString HtmlNoteReader::titleFromHtml(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        return QString();
    }
    QString content;
    QTextStream in(&file);
    content = in.readAll();
    file.close();

    QTextDocument  document;
    document.setHtml(content);
    QString title =  document.metaInformation(QTextDocument::DocumentTitle);
    return title;
}

void HtmlNoteReader::read(const QString& filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //qDebug(qPrintable(QString("HtmlNoteReader::read failed : could not open filepath ") + QDir::toNativeSeparators(filePath)));
           return;
    }


    QString html;

    QTextStream in(&file);
    in.setCodec("UTF-8");
    html = in.readAll();
    file.close();

    if(!html.isEmpty() && !Qt::mightBeRichText(html))
       html = Qt::convertFromPlainText(html); // this qt method creates unwanted paragraphs if empty Strings are converted

    //if(html.isEmpty())
    //    return;

    uuid_ = uuidFromHtml(html);
    lastChange_ = QDateTime::fromString(metaContent(html,"last-change-date"),Qt::ISODate);
    createDate_ = QDateTime::fromString(metaContent(html,"create-date"),Qt::ISODate);


//    // fallback dates
//    if(lastChange_.isNull())
//        lastChange_ = info.lastModified();


       // read <title>
       int titleStartIndex = html.indexOf("<title>");
       int titleEndIndex = html.indexOf("</title>");

       if(titleStartIndex != -1 && titleEndIndex != -1 && titleStartIndex < titleEndIndex)
       {
            int start = titleStartIndex + QLatin1String("<title>").size();
           title_ = html.mid(start,titleEndIndex - start);
       }

       html_ = html;
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
    return Qt::mightBeRichText(content) ? uuidFromHtml(content) : QUuid();
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
    if(html.isEmpty())
        return QString();

    QString content = html;
    QTime time; // avoid forever loop
    time.start();
    int metaIdx = 0;
    int endIdx = 0;
    // find the meta elements in the html files and read the uuid
    while(metaIdx != -1 && time.elapsed() < 1500)
    {
        metaIdx = content.indexOf("<meta",endIdx+1);

        if(metaIdx==-1)
            break;

        endIdx = content.indexOf(">",metaIdx+1);
#if QT_VERSION >= 0x040800 // Qt Version > 4.8
        QStringRef metaLine = content.midRef(metaIdx,endIdx-metaIdx+1); // e.g. <meta name="qrichtext" content="1" />
#else
        QString metaLine = content.mid(metaIdx,endIdx-metaIdx+1); // e.g. <meta name="qrichtext" content="1" />
#endif
        if(metaLine.contains(name))
        {
            int idx = metaLine.lastIndexOf('\"');
            int beforeIdx = metaLine.lastIndexOf('\"',idx-1);
            if(idx != -1 || beforeIdx != -1)
            {
                #if QT_VERSION >= 0x040800 // Qt Version > 4.8
                return metaLine.toString().mid(beforeIdx +1,(idx-beforeIdx+1) -2); // +1 and -1 to take the content between the " "
                #else
                return metaLine.mid(beforeIdx +1,(idx-beforeIdx+1) -2); // +1 and -1 to take the content between the " "
                #endif
            }
        }
    }
    return QString();
}
