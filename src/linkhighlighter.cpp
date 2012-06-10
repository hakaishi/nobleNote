#include "linkhighlighter.h"

LinkHighlighter::LinkHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent){
     HighlightingRule rule;

     keywordFormat.setForeground(Qt::blue);
     keywordFormat.setFontUnderline(true);

     QStringList keywordPatterns;
     keywordPatterns << "((https?|ftp)://\\S+)" << "((\\S+)(@)(\\S+)(\\.)(\\S+))";

     foreach (const QString &pattern, keywordPatterns) {
         rule.pattern = QRegExp(pattern);
         rule.format = keywordFormat;
         highlightingRules.append(rule);
     }
}

void LinkHighlighter::highlightBlock(const QString &text){
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
