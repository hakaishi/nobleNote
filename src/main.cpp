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

#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
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
     if (!qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        return 0;
     }
       QLibraryInfo::path(QLibraryInfo::TranslationsPath);
     app.installTranslator(&qtTranslator);


     //NobleNote translations
     QTranslator translator;
    #ifdef Q_OS_WIN32
     translator.load(":" + QLocale::system().name());
    #else
     QString tmp = "/usr/share/noblenote/translations/noblenote_";
     if (!translator.load(tmp + QLocale::system().name())) {
        return 0;
     }
    #endif
     app.installTranslator(&translator);

     app.setQuitOnLastWindowClosed(false);

     QSettings::setDefaultFormat(QSettings::IniFormat);

     QFileInfo settingsFile = QFileInfo(QDir::toNativeSeparators(app.applicationDirPath() + "/" +
                              app.applicationName() + "/" + QFileInfo(QSettings().fileName()).fileName()));

     if(settingsFile.exists()) //check if there is a conf/ini file in a folder called nobleNote next to the executable (for portable version)
     {
          QDir settingsParentDir = settingsFile.dir();
          settingsParentDir.cdUp(); //cdUp because the current folder will be created by QSettings here after.
          QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settingsParentDir.absolutePath()); //use this file instead of system standard if this is the case
          QSettings().setValue("isPortable",true);
     }
     else
       QSettings().setValue("isPortable",false);

     if(!QSettings().isWritable()) // TODO QObject::tr does not work here because there is no Q_OBJECT macro in main
         QMessageBox::critical(0,"Settings not writable", QString("%1 settings not writable!").arg(app.applicationName()));
     if(!QFile(QSettings().value("import_path").toString()).exists())
         QSettings().setValue("import_path", QDir::homePath());
     bool rootPathIsSet = QSettings().value("root_path").isValid();
     bool rootPathExists = QFileInfo::exists(QSettings().value("root_path").toString());
     bool rootPathIsWritable = QFileInfo(QSettings().value("root_path").toString()).isWritable();
     if(!rootPathExists || !rootPathIsWritable)
     {
          QScopedPointer<Welcome> welcome(new Welcome);
          welcome->getInstance(rootPathIsSet, rootPathExists, rootPathIsWritable);
          if(welcome->exec() == QDialog::Rejected) // welcome writes the root path
            return 0; // leave main if the user rejects the welcome dialog, else go on
     }
     else if(!rootPathIsSet)
       QSettings().setValue("root_path", app.applicationDirPath() + "/" + app.applicationName());

     MainWindow window;
     if(QSettings().value("Hide_main_at_startup",false).toBool())
       window.showOpenNotes();
     else
       window.show();

     return app.exec();
}
