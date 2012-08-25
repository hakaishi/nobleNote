#include "fileiconprovider.h"

FileIconProvider::FileIconProvider()
{
}

QIcon FileIconProvider::icon(QFileIconProvider::IconType type) const
{
    qDebug("called");
    if(type == QFileIconProvider::Folder)
        return QIcon(":folder");
    else if(type == QFileIconProvider::File)
        return QIcon(":file");
    else
        return QFileIconProvider::icon(type);


}

QIcon FileIconProvider::icon(const QFileInfo &info) const
{
    if(info.isDir())
        return QIcon(":folder");
    else if(info.isFile())
        return QIcon(":file");
    else
        return QFileIconProvider::icon(info);
}

