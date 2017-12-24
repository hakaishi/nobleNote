TEMPLATE = app
TARGET = bin/noblenote
DEPENDPATH = . src
INCLUDEPATH = . src
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
RCC_DIR = build

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent

win32 {
#QMAKE_LFLAGS += -static-libgcc

# use these for windows builds release builds only because debugging 
#symbols wont be linked if only release is specified
#CONFIG -= debug_and_release
#CONFIG += release

RC_FILE += icon.rc
}

system(lrelease nobleNote.pro)

# BUILDTIME and BUILDDATE will be shown in the About dialog
win32 {
DEFINES += BUILDTIME=\\\"$$system('echo %time%')\\\"
DEFINES += BUILDDATE=\\\"$$system('echo %date%')\\\"
} else {
DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M.%s')\\\"
DEFINES += BUILDDATE=\\\"$$system(date '+%d/%m/%y')\\\"
}

QMAKE_DISTCLEAN = src/translations/*.qm

# Input
HEADERS = src/mainwindow.h src/note.h \
    src/welcome.h \
    src/filesystemmodel.h \
    src/preferences.h \
    src/findfilesystemmodel.h \
    src/findfilemodel.h \
    src/lineedit.h \
    src/textbrowser.h \
    src/xorcipher.h \
    src/textformattingtoolbar.h \
    src/highlighter.h src/textsearchtoolbar.h \
    src/xmlnotewriter.h \
    src/xmlnotereader.h \
    src/datetime.h \
    src/textdocument.h \
    src/notedescriptor.h \
    src/abstractnotereader.h \
    src/htmlnotereader.h \
    src/htmlnotewriter.h \
    src/fileiconprovider.h \
    src/backup.h \
    src/trash.h \
    src/progressreceiver.h \
    src/noteimporter.h
FORMS = src/ui/mainwindow.ui src/ui/welcome.ui src/ui/note.ui \
        src/ui/preferences.ui src/ui/trash.ui 
SOURCES = src/main.cpp src/mainwindow.cpp src/note.cpp \
    src/welcome.cpp\
    src/preferences.cpp \
    src/findfilemodel.cpp \
    src/findfilesystemmodel.cpp src/lineedit.cpp src/textbrowser.cpp \
    src/xorcipher.cpp \
    src/textformattingtoolbar.cpp \
    src/highlighter.cpp src/textsearchtoolbar.cpp \
    src/xmlnotewriter.cpp \
    src/xmlnotereader.cpp \
    src/textdocument.cpp \
    src/notedescriptor.cpp \
    src/htmlnotereader.cpp \
    src/htmlnotewriter.cpp \
    src/datetime.cpp \
    src/fileiconprovider.cpp \
    src/backup.cpp \
    src/trash.cpp \
    src/progressreceiver.cpp \
    src/noteimporter.cpp
RESOURCES += nobleNote.qrc
TRANSLATIONS = src/translations/noblenote_ast.ts\
               src/translations/noblenote_cs.ts\
               src/translations/noblenote_de.ts\
               src/translations/noblenote_de_DE.ts\
               src/translations/noblenote_es.ts\
               src/translations/noblenote_gl.ts\
               src/translations/noblenote_ms.ts\
               src/translations/noblenote_pl.ts\
               src/translations/noblenote_ru.ts\
               src/translations/noblenote_uk.ts

!win32{
# install
target.path = /usr/bin
icons.files = src/noblenote-icons/*
icons.path = /usr/share/pixmaps/noblenote-icons
translation.files = src/translations/*.qm
translation.path = /usr/share/noblenote/translations
autostart.files = autostart/noblenote.desktop
autostart.path = /usr/share/applications

INSTALLS = target icons translation autostart

deinstall.depends = uninstall FORCE
deinstall.commands = rm -R /usr/share/noblenote
QMAKE_EXTRA_TARGETS = deinstall
}

OTHER_FILES += \
    icon.rc \
    src/noblenote-icons/noblenote.ico
