#include "note.h"
#include <QDebug>
#include <QFile>
#include <QPushButton>
#include <QDir>

Note::Note(QWidget *parent) : QDialog (parent){

     setupUi(this);

     //TODO:Make text formating possible

     timer = new QTimer(this);
     timer->setInterval(1000);
     timer->setSingleShot(true);

     connect(textEdit, SIGNAL(textChanged()), timer, SLOT(start()));
     connect(timer, SIGNAL(timeout()), this, SLOT(saveNote()));
     connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)), this, SLOT(saveNote()));
     connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)), this, SLOT(dontSave()));
     connect(toolB, SIGNAL(clicked(bool)), this, SLOT(boldText(bool)));
     connect(toolI, SIGNAL(clicked(bool)), this, SLOT(italicText(bool)));
     connect(toolU, SIGNAL(clicked(bool)), this, SLOT(underlinedText(bool)));
}

Note::~Note(){}

void Note::saveNote(){
       QFile file(notesPath);
       if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
         return;
       QTextStream stream(&file);
       stream << textEdit->toHtml();
       file.close();
       //TODO 3: create real notes (so far only journal)
}

void Note::dontSave(){
       QFile file(notesPath);
       if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
         return;
       QTextStream stream(&file);
       stream << text;
       file.close();
}

void Note::mergeFormatOnWordOrSelection(const QTextCharFormat &format){
     QTextCursor cursor = textEdit->textCursor();
     if (!cursor.hasSelection())
         cursor.select(QTextCursor::WordUnderCursor);
     cursor.mergeCharFormat(format);
     textEdit->mergeCurrentCharFormat(format);
}

void Note::boldText(bool isBold){ //TODO:for some reason variable is always false
     QTextCharFormat fmt;
     fmt.setFontWeight(isBold ? QFont::Bold : QFont::Normal);
     mergeFormatOnWordOrSelection(fmt);
}

void Note::italicText(bool isItalic){//TODO:for some reason variable is always false
     QTextCharFormat fmt;
     fmt.setFontItalic(isItalic);
     mergeFormatOnWordOrSelection(fmt);
}

void Note::underlinedText(bool isUnderlined){//TODO:for some reason variable is always false
     QTextCharFormat fmt;
     fmt.setFontUnderline(isUnderlined);
     mergeFormatOnWordOrSelection(fmt);
}

void Note::showEvent(QShowEvent* show_Note){
     textEdit->setHtml(text);
     QWidget::showEvent(show_Note);
}

void Note::closeEvent(QCloseEvent* close_Note){
     dontSave();
     QWidget::closeEvent(close_Note);
}
