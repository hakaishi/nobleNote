#ifndef TEXTFORMATTINGTOOLBAR_H
#define TEXTFORMATTINGTOOLBAR_H

#include <QToolBar>
#include <QFontComboBox>
#include <QTextEdit>

/**
  * a toolbar with buttons to format text on a QTextEdit
  * you must call addToolBar on the MainWindow to add this toolbar
  *
  */

class TextFormattingToolbar : public QToolBar
{
    Q_OBJECT
public:
    explicit TextFormattingToolbar(QTextEdit* textEdit,QWidget *parent = 0);
    
public slots:

    void getFontAndPointSizeOfText(const QTextCharFormat &format);

private slots:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void boldText();
    void italicText();
    void underlinedText();
    void strikedOutText();
    void coloredText();
    void markedText();
    void insertHyperlink();
    void fontOfText(const QString &f);
    void pointSizeOfText(const QString &p);

private:
    QFontComboBox *fontComboBox;
    QComboBox *comboSize;
    QAction   *actionTextBold;
    QAction *actionTextItalic;
    QAction *actionTextUnderline;
    QAction *actionTextColor;
    QAction *actionTextBColor;
    QAction *actionTextStrikeOut;
    QAction *actionInsertHyperlink;
    QTextEdit * textEdit_;
    
};

#endif // TEXTFORMATTINGTOOLBAR_H
