/* nobleNote, a note taking application
 * Copyright (C) 2012 Christian Metscher <hakaishi@web.de>,
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

#include "htmlnotewriter.h"
#include "datetime.h"
#include "xmlnotereader.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QSettings>

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
    insertMetaElement(&html,"create-date",DateTime::toISO8601(lastChange_.isNull()? QDateTime::currentDateTime():lastChange_));
    insertMetaElement(&html,"last-change-date",DateTime::toISO8601(lastChange_.isNull()? QDateTime::currentDateTime():lastChange_));
    insertMetaElement(&html,"uuid",uuidStr);



    QFile file(filePath_);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug(qPrintable(QString("HtmlNoteWriter::write failed : could not open ") + QDir::toNativeSeparators(filePath_)));
           return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8"); // set to UTF-8 for every platform, else ISO-8859-1 would be used on windows by default
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

    if(!QDir(QSettings().value("backup_dir_path").toString()).exists())
        QDir().mkpath(QSettings().value("backup_dir_path").toString());

    QString uuid = reader.uuid();
    uuid.chop(1); // }
    uuid = uuid.remove(0,1); // {

    QFile::copy(filePath, QSettings().value("backup_dir_path").toString() + "/" + uuid);
}
