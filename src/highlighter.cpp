#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent){
     HighlightingRule rule;

     linkFormat.setForeground(Qt::blue);
     linkFormat.setFontUnderline(true);

     QStringList keywordPatterns;
     keywordPatterns << "((https?|ftp)://\\S+)" << "((\\S+)(@)(\\S+)(\\.)(\\S+))";

     foreach (const QString &pattern, keywordPatterns) {
         rule.pattern = QRegExp(pattern);
         rule.format = linkFormat;
         highlightingRules.append(rule);
     }
}

void Highlighter::highlightBlock(const QString &text){
     HighlightingRule rule;
     keywordFormat.setBackground(Qt::yellow);
     if(!expression.isEmpty()){
       rule.pattern = QRegExp(expression, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
       rule.format = keywordFormat;
       highlightingRules.append(rule);
     }

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
