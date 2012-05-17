#include "note.h"
#include "xmlnote.h"
#include <QFile>
#include <QPushButton>
#include <QDir>
#include <QToolBar>
#include <QColorDialog>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QTextBlock>
#include <QTextFragment>
#include <QMessageBox>
#include <QDebug>


Note::Note(QWidget *parent) : QMainWindow(parent){

     setupUi(this);
     setupTextFormattingOptions();

     jTimer = new QTimer(this);
     jTimer->setInterval(1000);
     jTimer->setSingleShot(true);

     timer = new QTimer(this);

     connect(textEdit, SIGNAL(textChanged()), jTimer, SLOT(start()));
     connect(jTimer, SIGNAL(timeout()), this, SLOT(save_or_not()));
     connect(timer, SIGNAL(timeout()), this, SLOT(save_or_not()));
     connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked(bool)),
       this, SLOT(resetAll()));
     connect(textEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this,
       SLOT(getFontAndPointSizeOfText(QTextCharFormat)));
}

Note::~Note(){ save_or_not();}

void Note::showEvent(QShowEvent* show_Note){
     load();
     QWidget::showEvent(show_Note);
}

void Note::load(){
     QFile note(notesPath);
     if(!note.open(QIODevice::ReadOnly))
       return;
     QTextStream nStream(&note);
     text = nStream.readAll();
     note.close();
     QFileInfo noteInfo(notesPath);
     noteModified = noteInfo.lastModified();

     setWindowTitle(noteInfo.fileName());
     textEdit->setHtml(text);

     QFile journal(journalsPath);
     if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     if(!journal.exists()){
       QTextStream jStream(&journal);
       jStream << text;
     }
     journal.close();
     QFileInfo journalInfo(journalsPath);
     journalModified = journalInfo.lastModified();
}

void Note::saveAll(){
     QFile note(notesPath);
     if(!note.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream nStream(&note);
     nStream << textEdit->toHtml();
     note.close();
     QFileInfo noteInfo(notesPath);
     noteModified = noteInfo.lastModified();

     QFile journal(journalsPath);
     if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream jStream(&journal);
     jStream << textEdit->toHtml();
     journal.close();
     QFileInfo journalInfo(journalsPath);
     journalModified = journalInfo.lastModified();

qDebug()<<"saved";

     // test xml output
//    QString xmlOutput;
//    QString noteName ="foo bar";
//    XmlNote writer(&xmlOutput);
//    writer.setFrame(textEdit->document()->rootFrame());
//    writer.write();
//     qDebug() << xmlOutput;
}

void Note::save_or_not(){
     QFile note(notesPath);

     if(!note.exists()){
       if(QMessageBox::warning(this,tr("Note doesn't exist"),
          tr("Do you want to save this Note as a new one?"),
          QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
         close();
       else
         saveAll();
     }

     QFileInfo noteInfo(notesPath);
     QFileInfo journalInfo(journalsPath);
     if((noteModified != noteInfo.lastModified()) ||
        (journalModified != journalInfo.lastModified())){
       load(); //try to reload file
       return;
     }

     if(!note.open(QIODevice::ReadOnly))
       return;
     QTextStream nStream(&note);
     QString content = nStream.readAll();
     note.close();
     if(content == textEdit->toHtml())
       return; //don't save if text didn't change
     else
       saveAll();
}

void Note::resetAll(){
     textEdit->setHtml(text);

     QFile note(notesPath);
     if(!note.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream stream(&note);
     stream << text;
     note.close();
     QFileInfo noteInfo(notesPath);
     noteModified = noteInfo.lastModified();

     QFile journal(journalsPath);
     if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream jStream(&journal);
     jStream << text;
     journal.close();
     QFileInfo journalInfo(journalsPath);
     journalModified = journalInfo.lastModified();
}

void Note::setupTextFormattingOptions(){
     QToolBar *tb = new QToolBar(this);
     tb->setWindowTitle(tr("Format Actions"));
     addToolBar(tb);

     actionTextBold = new QAction(QIcon::fromTheme("format-text-bold", QIcon(":bold")), tr("&Bold"), this);
     actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
     actionTextBold->setPriority(QAction::LowPriority);
     QFont bold;
     bold.setBold(true);
     actionTextBold->setFont(bold);
     connect(actionTextBold, SIGNAL(triggered()), this, SLOT(boldText()));
     tb->addAction(actionTextBold);
     actionTextBold->setCheckable(true);

     actionTextItalic = new QAction(QIcon::fromTheme("format-text-italic", QIcon(":italic")), tr("&Italic"), this);
     actionTextItalic->setPriority(QAction::LowPriority);
     actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
     QFont italic;
     italic.setItalic(true);
     actionTextItalic->setFont(italic);
     connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(italicText()));
     tb->addAction(actionTextItalic);
     actionTextItalic->setCheckable(true);

     actionTextUnderline = new QAction(QIcon::fromTheme("format-text-underline", QIcon(":underlined")), tr("&Underline"), this);
     actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
     actionTextUnderline->setPriority(QAction::LowPriority);
     QFont underline;
     underline.setUnderline(true);
     actionTextUnderline->setFont(underline);
     connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(underlinedText()));
     tb->addAction(actionTextUnderline);
     actionTextUnderline->setCheckable(true);

     actionTextStrikeOut = new QAction(QIcon::fromTheme("format-text-strikethrough", QIcon(":strikedout")), tr("&Strike Out"), this);
     actionTextStrikeOut->setShortcut(Qt::CTRL + Qt::Key_S);
     actionTextStrikeOut->setPriority(QAction::LowPriority);
     QFont strikeOut;
     strikeOut.setStrikeOut(true);
     actionTextStrikeOut->setFont(strikeOut);
     connect(actionTextStrikeOut, SIGNAL(triggered()), this, SLOT(strikedOutText()));
     tb->addAction(actionTextStrikeOut);
     actionTextStrikeOut->setCheckable(true);

     QPixmap textPix(16, 16);
     textPix.fill(Qt::black);
     actionTextColor = new QAction(textPix, tr("&Text color..."), this);
     connect(actionTextColor, SIGNAL(triggered()), this, SLOT(coloredText()));
     tb->addAction(actionTextColor);

     QPixmap bPix(16, 16);
     bPix.fill(Qt::white);
     actionTextBColor = new QAction(bPix, tr("&Background color..."), this);
     connect(actionTextBColor, SIGNAL(triggered()), this, SLOT(markedText()));
     tb->addAction(actionTextBColor);

     fontComboBox = new QFontComboBox(tb);
     tb->addWidget(fontComboBox);
     connect(fontComboBox, SIGNAL(activated(QString)), this, SLOT(fontOfText(QString)));

     comboSize = new QComboBox(tb);
     comboSize->setObjectName("comboSize");
     tb->addWidget(comboSize);
     comboSize->setEditable(true);
     connect(comboSize, SIGNAL(activated(QString)), this, SLOT(pointSizeOfText(QString)));

     QFontDatabase db;
     foreach(int size, db.standardSizes())
       comboSize->addItem(QString::number(size));
     comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font().pointSize())));
}

void Note::mergeFormatOnWordOrSelection(const QTextCharFormat &format){
     QTextCursor cursor = textEdit->textCursor();
     if(!cursor.hasSelection() && !cursor.atBlockStart() && !cursor.atBlockEnd())
       cursor.select(QTextCursor::WordUnderCursor);
     cursor.mergeCharFormat(format);
     textEdit->mergeCurrentCharFormat(format);
}

void Note::getFontAndPointSizeOfText(const QTextCharFormat &format){
     QFont f = format.font();
     fontComboBox->setCurrentIndex(fontComboBox->findText(QFontInfo(f).family()));
     comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
     actionTextBold->setChecked(f.bold());
     actionTextItalic->setChecked(f.italic());
     actionTextUnderline->setChecked(f.underline());
     actionTextStrikeOut->setChecked(f.strikeOut());
     QPixmap textPix(16,16);
     textPix.fill(format.foreground().color());
     actionTextColor->setIcon(textPix);
     QPixmap bPix(16,16);
     bPix.fill(format.background().color());
     actionTextBColor->setIcon(bPix);
}

void Note::boldText(){
     QTextCharFormat fmt;
     fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
     mergeFormatOnWordOrSelection(fmt);
}

void Note::italicText(){
     QTextCharFormat fmt;
     fmt.setFontItalic(actionTextItalic->isChecked());
     mergeFormatOnWordOrSelection(fmt);
}

void Note::underlinedText(){
     QTextCharFormat fmt;
     fmt.setFontUnderline(actionTextUnderline->isChecked());
     mergeFormatOnWordOrSelection(fmt);
}

void Note::strikedOutText(){
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(actionTextStrikeOut->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void Note::coloredText(){
     QColor col = QColorDialog::getColor(textEdit->textColor(), this);
     if(!col.isValid())
       return;
     QTextCharFormat fmt;
     fmt.setForeground(col);
     mergeFormatOnWordOrSelection(fmt);
     QPixmap pix(16, 16);
     pix.fill(col);
     actionTextColor->setIcon(pix);
}

void Note::markedText(){
     QColor col = QColorDialog::getColor(textEdit->textBackgroundColor(), this);
     if(!col.isValid())
       return;
     QTextCharFormat fmt;
     fmt.setBackground(col);
     mergeFormatOnWordOrSelection(fmt);
     QPixmap pix(16, 16);
     pix.fill(col);
     actionTextColor->setIcon(pix);
}

void Note::fontOfText(const QString &f){
     QTextCharFormat fmt;
     fmt.setFontFamily(f);
     mergeFormatOnWordOrSelection(fmt);
}

void Note::pointSizeOfText(const QString &p){
     qreal pointSize = p.toFloat();
     if(p.toFloat() > 0){
       QTextCharFormat fmt;
       fmt.setFontPointSize(pointSize);
       mergeFormatOnWordOrSelection(fmt);
     }
}

void Note::keyPressEvent(QKeyEvent *k){
     if((k->modifiers() == Qt::ControlModifier) && (k->key() == Qt::Key_F)){

     }
}
