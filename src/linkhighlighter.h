#ifndef LINKHIGHLIGHTER_H
#define LINKHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

class LinkHighlighter : public QSyntaxHighlighter{
     Q_OBJECT

     public:
      LinkHighlighter(QTextDocument *parent = 0);
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

#endif //LINKHIGHLIGHTER_H
