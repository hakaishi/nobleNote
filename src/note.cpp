#include "note.h"
#include <QDebug>
#include <QFile>
#include <QPushButton>
#include <QDir>

Note::Note(QWidget *parent) : QDialog (parent){

     setupUi(this);

     timer = new QTimer(this);
     timer->setInterval(1000);
     timer->setSingleShot(true);

     connect(textEdit, SIGNAL(textChanged()), timer, SLOT(start()));
     connect(timer, SIGNAL(timeout()), this, SLOT(saveNote()));
     connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)), this, SLOT(saveNote()));
     connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)), this, SLOT(dontSave()));
     connect(toolB, SIGNAL(clicked()), this, SLOT(boldText()));
     connect(toolI, SIGNAL(clicked()), this, SLOT(italicText()));
     connect(toolU, SIGNAL(clicked()), this, SLOT(underlinedText()));
     connect(fontComboBox, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(fontOfText()));
     connect(fontSizeSpin, SIGNAL(valueChanged(int)), this, SLOT(pointSizeOfText()));
     connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(getFontAndPointSizeOfText()));
}

Note::~Note(){}

void Note::saveNote(){
       QFile file(notesPath);
       if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
         return;
       QTextStream stream(&file);
       stream << textEdit->toHtml();
       file.close();
       //TODO: create real notes (so far only journal)
}

void Note::dontSave(){
       QFile file(notesPath);
       if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
         return;
       QTextStream stream(&file);
       stream << text;
       file.close();
}

QTextCharFormat Note::getFormatOnWordOrSelection(){
     QTextCursor cursor = textEdit->textCursor();
     if (!cursor.hasSelection())
         cursor.select(QTextCursor::WordUnderCursor);
     return cursor.charFormat();
}

void Note::mergeFormatOnWordOrSelection(const QTextCharFormat &format){
     QTextCursor cursor = textEdit->textCursor();
     if (!cursor.hasSelection())
         cursor.select(QTextCursor::WordUnderCursor);
     cursor.mergeCharFormat(format);
     textEdit->mergeCurrentCharFormat(format);
}

void Note::boldText(){
     QTextCharFormat fmt;
     if(getFormatOnWordOrSelection().fontWeight() == 75)
       fmt.setFontWeight(QFont::Normal);
     else
       fmt.setFontWeight(QFont::Bold);
     mergeFormatOnWordOrSelection(fmt);
}

void Note::italicText(){
     QTextCharFormat fmt;
     if(getFormatOnWordOrSelection().fontItalic())
       fmt.setFontItalic(false);
     else
       fmt.setFontItalic(true);
     mergeFormatOnWordOrSelection(fmt);
}

void Note::underlinedText(){//TODO:for some reason (under-)line disappears while text is selected
     QTextCharFormat fmt;
     if(getFormatOnWordOrSelection().fontUnderline())
       fmt.setFontUnderline(false);
     else
       fmt.setFontUnderline(true);
     mergeFormatOnWordOrSelection(fmt);
}

void Note::getFontAndPointSizeOfText(){
     fontComboBox->setCurrentFont(getFormatOnWordOrSelection().font());
     fontSizeSpin->setValue(getFormatOnWordOrSelection().fontPointSize());
}

void Note::fontOfText(){
     QTextCharFormat fmt;
     fmt.setFont(fontComboBox->currentFont());
     mergeFormatOnWordOrSelection(fmt);
}

void Note::pointSizeOfText(){
     QTextCharFormat fmt;
     fmt.setFontPointSize(fontSizeSpin->value());
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
