#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>

int main (int argc, char *argv[]){

     QApplication app(argc, argv);
     app.setApplicationName("nobleNote");
     app.setOrganizationName("Hakaishi_and_Taiko");

     //Qt translations
     QTranslator qtTranslator;
     qtTranslator.load("qt_" + QLocale::system().name(),
       QLibraryInfo::location(QLibraryInfo::TranslationsPath));
     app.installTranslator(&qtTranslator);

     app.setQuitOnLastWindowClosed(false);

     NobleNote window;
     window.show();
     return app.exec();
}
