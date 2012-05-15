#include "note.h"
#include <QFile>
#include <QPushButton>
#include <QDir>
#include <QToolBar>
#include <QColorDialog>
#include <QTextStream>
#include <QDebug>
Note::Note(QWidget *parent) : QMainWindow(parent){

     setupUi(this);
     setupTextFormattingOptions();

     jTimer = new QTimer(this);
     jTimer->setInterval(1000);
     jTimer->setSingleShot(true);

     timer = new QTimer(this);

     connect(textEdit, SIGNAL(textChanged()), jTimer, SLOT(start()));
     connect(jTimer, SIGNAL(timeout()), this, SLOT(saveText()));
     connect(timer, SIGNAL(timeout()), this, SLOT(saveText()));
     connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked(bool)),
       this, SLOT(resetAll()));
     connect(textEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this,
       SLOT(getFontAndPointSizeOfText(QTextCharFormat)));
}

Note::~Note(){ saveText(); closing(notesPath); }

void Note::showEvent(QShowEvent* show_Note){
     textEdit->setHtml(text);
     QWidget::showEvent(show_Note);
}

void Note::saveText(){
     QFile journal(journalsPath);
     if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream jStream(&journal);
     jStream << textEdit->toHtml();
     journal.close();

     QFile note(notesPath);
     if(!note.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream nStream(&note);
     nStream << textEdit->toHtml();
     note.close();
}

void Note::resetAll(){
     QFile file(notesPath);
     if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream stream(&file);
     stream << text;
     file.close();

     textEdit->setHtml(text);
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
