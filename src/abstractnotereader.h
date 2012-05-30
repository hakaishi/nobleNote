#ifndef ABSTRACTNOTEREADER_H
#define ABSTRACTNOTEREADER_H

/**
 * @brief The AbstractNoteReader is an interface for note reader classes
 */

class QUuid;
class QDateTime;
class QString;
class QIODevice;

class AbstractNoteReader
{
public:
    AbstractNoteReader();

    virtual void read() = 0; // read the content's of a QIODevice and write the formatted text into a QTextFrame

    virtual QUuid uuid() const = 0; // get the uuid that has been extracted during read()

    virtual const QString& noteTitle() const     = 0;

     // get last change date
    virtual const QDateTime& lastChange() const          = 0;

    // get last metadata change date
    virtual const QDateTime& lastMetadataChange() const  = 0;

    // get create date
    virtual const QDateTime& createDate() const         = 0;

     // reads a uuid from a file, if uuid could not be found, a null uuid is returned
    //virtual static QUuid uuid(QIODevice * device) = 0;

    // searches all directorys under the given path recursively for a file with the given UUID
    // returns the first file that contains the given uuid or an empty string if the uuid could not be found or if the given uuid is null
    //virtual static QString findUuid(const QUuid uuid, const QString & path) = 0;
};

#endif // ABSTRACTNOTEREADER_H
