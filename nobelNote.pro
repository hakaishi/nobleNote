TEMPLATE = app
TARGET = bin/nobleNote
DEPENDPATH = . src
INCLUDEPATH = . src
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
RCC_DIR = build

# Input
HEADERS = src/mainwindow.h src/note.h
FORMS = src/ui/note.ui src/ui/mainwindow.ui
SOURCES = src/main.cpp src/mainwindow.cpp src/note.cpp
