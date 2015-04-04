/* nobleNote, a note taking application
 * Copyright (C) 2015 Christian Metscher <hakaishi@web.de>,
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

#include "mainwindow.h"
#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#else
#include <QApplication>
#include <QMessageBox>
#endif
#include <QTranslator>
#include <QLibraryInfo>
#include <QSettings>

#include "welcome.h"

int main (int argc, char *argv[]){
     QApplication app(argc, argv);
     app.setApplicationName("nobleNote");
     app.setOrganizationName("nobleNote");

     //Qt translations
     QTranslator qtTranslator;
     qtTranslator.load("qt_" + QLocale::system().name(),
       QLibraryInfo::location(QLibraryInfo::TranslationsPath));
     app.installTranslator(&qtTranslator);

     //NobleNote translations
     QTranslator translator;
    #ifdef Q_OS_WIN32
     translator.load(":" + QLocale::system().name());
    #else
     QString tmp = "/usr/share/noblenote/translations/noblenote_";
     translator.load(tmp + QLocale::system().name());
    #endif
     app.installTranslator(&translator);

     app.setQuitOnLastWindowClosed(false);

     QDir settingsFilePath = QDir(QCoreApplication::applicationDirPath()); //the settings always use organization as the folder, so we need to to change in the folder above. The folder must be named nobleNote!
     settingsFilePath.cdUp();
     QSettings settings; // uses standard path + Organization/Application.conf automatically
     if(QFile(QCoreApplication::applicationDirPath() + QDir::separator() + QFileInfo(settings.fileName()).fileName()).exists()) //check if there is a conf file next to the executable (for portable version)
     {
          QSettings::setPath(QSettings::defaultFormat(), QSettings::UserScope,settingsFilePath.path()); //use this file instead of system standard if this is the case
          QSettings().setValue("isPortable",true);
     }
     else
       QSettings().setValue("isPortable",false);

     if(!settings.isWritable()) // TODO QObject::tr does not work here because there is no Q_OBJECT macro in main
         QMessageBox::critical(0,"Settings not writable", QString("%1 settings not writable!").arg(app.applicationName()));
     if(!QFile(settings.value("import_path").toString()).exists())
         settings.setValue("import_path", QDir::homePath());
     bool rootPathIsSet = settings.value("root_path").isValid();
     bool rootPathExists = QFileInfo(settings.value("root_path").toString()).exists();
     bool rootPathIsWritable = QFileInfo(settings.value("root_path").toString()).isWritable();
     if(!QSettings().value("isPortable",false).toBool() && (!rootPathExists || !rootPathIsWritable))
     {
          QScopedPointer<Welcome> welcome(new Welcome);
          welcome->getInstance(rootPathIsSet, rootPathExists, rootPathIsWritable);
          if(welcome->exec() == QDialog::Rejected) // welcome writes the root path
            return 0; // leave main if the user rejects the welcome dialog, else go on
     }
     else if(!rootPathIsSet)
       QSettings().setValue("root_path", QCoreApplication::applicationDirPath());

     MainWindow window;
     window.show();
     return app.exec();
}
