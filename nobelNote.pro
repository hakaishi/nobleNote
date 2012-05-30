TEMPLATE = app
TARGET = bin/nobleNote
DEPENDPATH = . src
INCLUDEPATH = . src
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
RCC_DIR = build

# Input
HEADERS = src/mainwindow.h src/note.h \
    src/filesystemmodel.h \
    src/preferences.h \
    src/findfilesystemmodel.h \
    src/findfilemodel.h \
    src/lineedit.h \
    src/textedit.h \
    src/xorcipher.h \
    src/textformattingtoolbar.h \
    src/newnotename.h \
    src/highlighter.h src/textsearchtoolbar.h \
    src/xmlnotewriter.h \
    src/xmlnotereader.h \
    src/datetime.h \
    src/textdocument.h \
    src/notedescriptor.h \
    src/abstractnotereader.h \
    src/htmlnotereader.h \
    src/htmlnotewriter.h
FORMS = src/ui/note.ui src/ui/mainwindow.ui src/ui/preferences.ui \
        src/ui/newnotename.ui
SOURCES = src/main.cpp src/mainwindow.cpp src/note.cpp \
    src/preferences.cpp \
    src/findfilemodel.cpp \
    src/findfilesystemmodel.cpp src/lineedit.cpp src/textedit.cpp \
    src/xorcipher.cpp \
    src/textformattingtoolbar.cpp \
    src/newnotename.cpp \
    src/highlighter.cpp src/textsearchtoolbar.cpp \
    src/xmlnotewriter.cpp \
    src/xmlnotereader.cpp \
    src/textdocument.cpp \
    src/notedescriptor.cpp \
    src/abstractnotereader.cpp \
    src/htmlnotereader.cpp \
    src/htmlnotewriter.cpp \
    src/datetime.cpp
RESOURCES = nobleNote.qrc
