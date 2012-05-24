#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>

class QTextDocument;

class Highlighter : public QSyntaxHighlighter{
     Q_OBJECT

     public:
      Highlighter(QTextDocument *parent = 0);
      QString expression;
      bool    caseSensitive;

     protected:
      void highlightBlock(const QString &text);

     private:
      struct HighlightingRule{
         QRegExp pattern;
         QTextCharFormat format;
      };
      QVector<HighlightingRule> highlightingRules;

      QTextCharFormat keywordFormat;
};

#endif //HIGHLIGHTER_H
