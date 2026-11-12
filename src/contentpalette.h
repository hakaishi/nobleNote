#ifndef CONTENTPALETTE_H
#define CONTENTPALETTE_H

#include <QApplication>
#include <QPalette>
#include <QWidget>

inline void updateContentPalette(QWidget *widget)
{
    const QPalette applicationPalette = QApplication::palette();
    const bool dark = applicationPalette.color(QPalette::Window).lightness()
            < applicationPalette.color(QPalette::WindowText).lightness();
    const QPalette currentPalette = widget->palette();
    QPalette contentPalette = currentPalette;
    contentPalette.setColor(QPalette::Base, dark ? Qt::black : Qt::white);
    contentPalette.setColor(QPalette::Text, dark ? Qt::white : Qt::black);
    if (contentPalette != currentPalette)
        widget->setPalette(contentPalette);
}

#endif
