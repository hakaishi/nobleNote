#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent){ }

void Highlighter::highlightBlock(const QString &text){
     HighlightingRule rule;

     keywordFormat.setBackground(Qt::yellow);

     rule.pattern = QRegExp(expression);
     rule.format = keywordFormat;
     highlightingRules.append(rule);

     foreach(const HighlightingRule &rule, highlightingRules){
       QRegExp expression(rule.pattern);
       int index = expression.indexIn(text);
       while(index >= 0){
         int length = expression.matchedLength();
         setFormat(index, length, rule.format);
         index = expression.indexIn(text, index + length);
       }
     }
}
