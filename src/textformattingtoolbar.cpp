#include "textformattingtoolbar.h"
#include <QAction>
#include <QApplication>
#include <QColorDialog>

TextFormattingToolbar::TextFormattingToolbar(QTextEdit * textEdit, QWidget *parent) :
    QToolBar(parent), textEdit_(textEdit)
{
    this->setWindowTitle(tr("Format actions"));
    setObjectName(tr("Formattoolbar"));

    actionTextBold = new QAction(QIcon::fromTheme("format-text-bold",
      QIcon(":bold")), tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(boldText()));
    this->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon::fromTheme("format-text-italic",
      QIcon(":italic")), tr("&Italic"), this);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(italicText()));
    this->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon::fromTheme("format-text-underline",
      QIcon(":underlined")), tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(underlinedText()));
    this->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    actionTextStrikeOut = new QAction(QIcon::fromTheme("format-text-strikethrough",
      QIcon(":strikedout")), tr("&Strike Out"), this);
    actionTextStrikeOut->setShortcut(Qt::CTRL + Qt::Key_S);
    actionTextStrikeOut->setPriority(QAction::LowPriority);
    QFont strikeOut;
    strikeOut.setStrikeOut(true);
    actionTextStrikeOut->setFont(strikeOut);
    connect(actionTextStrikeOut, SIGNAL(triggered()), this, SLOT(strikedOutText()));
    this->addAction(actionTextStrikeOut);
    actionTextStrikeOut->setCheckable(true);

    QPixmap textPix(16, 16);
    textPix.fill(Qt::black);
    actionTextColor = new QAction(textPix, tr("&Text color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(coloredText()));
    this->addAction(actionTextColor);

    QPixmap bPix(16, 16);
    bPix.fill(Qt::white);
    actionTextBColor = new QAction(bPix, tr("&Background color..."), this);
    connect(actionTextBColor, SIGNAL(triggered()), this, SLOT(markedText()));
    this->addAction(actionTextBColor);

    fontComboBox = new QFontComboBox(this);
    this->addWidget(fontComboBox);
    connect(fontComboBox, SIGNAL(activated(QString)), this, SLOT(fontOfText(QString)));

    comboSize = new QComboBox(this);
    comboSize->setObjectName("comboSize");
    this->addWidget(comboSize);
    comboSize->setEditable(true);
    connect(comboSize, SIGNAL(activated(QString)), this, SLOT(pointSizeOfText(QString)));

    QFontDatabase db;
    foreach(int size, db.standardSizes())
      comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(
      QApplication::font().pointSize())));

    connect(textEdit_, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this,
      SLOT(getFontAndPointSizeOfText(QTextCharFormat)));
}

void TextFormattingToolbar::mergeFormatOnWordOrSelection(const QTextCharFormat &format){
     QTextCursor cursor = textEdit_->textCursor();
     if(!cursor.hasSelection() && !cursor.atBlockStart() && !cursor.atBlockEnd())
       cursor.select(QTextCursor::WordUnderCursor);
     cursor.mergeCharFormat(format);
     textEdit_->mergeCurrentCharFormat(format);
}

void TextFormattingToolbar::getFontAndPointSizeOfText(const QTextCharFormat &format){
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

void TextFormattingToolbar::boldText(){
     QTextCharFormat fmt;
     fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
     mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::italicText(){
     QTextCharFormat fmt;
     fmt.setFontItalic(actionTextItalic->isChecked());
     mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::underlinedText(){
     QTextCharFormat fmt;
     fmt.setFontUnderline(actionTextUnderline->isChecked());
     mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::strikedOutText(){
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(actionTextStrikeOut->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::coloredText(){
     QColor col = QColorDialog::getColor(textEdit_->textColor(), this);
     if(!col.isValid())
       return;
     QTextCharFormat fmt;
     fmt.setForeground(col);
     mergeFormatOnWordOrSelection(fmt);
     QPixmap pix(16, 16);
     pix.fill(col);
     actionTextColor->setIcon(pix);
}

void TextFormattingToolbar::markedText(){
     QColor col = QColorDialog::getColor(textEdit_->textBackgroundColor(), this);
     if(!col.isValid())
       return;
     QTextCharFormat fmt;
     fmt.setBackground(col);
     mergeFormatOnWordOrSelection(fmt);
     QPixmap pix(16, 16);
     pix.fill(col);
     actionTextColor->setIcon(pix);
}

void TextFormattingToolbar::fontOfText(const QString &f){
     QTextCharFormat fmt;
     fmt.setFontFamily(f);
     mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::pointSizeOfText(const QString &p){
     qreal pointSize = p.toFloat();
     if(p.toFloat() > 0){
       QTextCharFormat fmt;
       fmt.setFontPointSize(pointSize);
       mergeFormatOnWordOrSelection(fmt);
     }
}
