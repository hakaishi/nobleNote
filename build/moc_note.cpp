/****************************************************************************
** Meta object code from reading C++ file 'note.h'
**
** Created: Tue May 8 22:42:30 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/note.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'note.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Note[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       6,    5,    5,    5, 0x0a,
      17,    5,    5,    5, 0x0a,
      28,    5,    5,    5, 0x0a,
      73,    5,   57,    5, 0x0a,
     109,  102,    5,    5, 0x0a,
     155,    5,    5,    5, 0x0a,
     166,    5,    5,    5, 0x0a,
     179,    5,    5,    5, 0x0a,
     196,    5,    5,    5, 0x0a,
     210,    5,    5,    5, 0x0a,
     223,  102,    5,    5, 0x0a,
     268,  266,    5,    5, 0x0a,
     288,  266,    5,    5, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Note[] = {
    "Note\0\0saveNote()\0dontSave()\0"
    "setupTextFormattingOptions()\0"
    "QTextCharFormat\0getFormatOnWordOrSelection()\0"
    "format\0mergeFormatOnWordOrSelection(QTextCharFormat)\0"
    "boldText()\0italicText()\0underlinedText()\0"
    "coloredText()\0markedText()\0"
    "getFontAndPointSizeOfText(QTextCharFormat)\0"
    "f\0fontOfText(QString)\0pointSizeOfText(QString)\0"
};

void Note::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Note *_t = static_cast<Note *>(_o);
        switch (_id) {
        case 0: _t->saveNote(); break;
        case 1: _t->dontSave(); break;
        case 2: _t->setupTextFormattingOptions(); break;
        case 3: { QTextCharFormat _r = _t->getFormatOnWordOrSelection();
            if (_a[0]) *reinterpret_cast< QTextCharFormat*>(_a[0]) = _r; }  break;
        case 4: _t->mergeFormatOnWordOrSelection((*reinterpret_cast< const QTextCharFormat(*)>(_a[1]))); break;
        case 5: _t->boldText(); break;
        case 6: _t->italicText(); break;
        case 7: _t->underlinedText(); break;
        case 8: _t->coloredText(); break;
        case 9: _t->markedText(); break;
        case 10: _t->getFontAndPointSizeOfText((*reinterpret_cast< const QTextCharFormat(*)>(_a[1]))); break;
        case 11: _t->fontOfText((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->pointSizeOfText((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Note::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Note::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_Note,
      qt_meta_data_Note, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Note::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Note::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Note::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Note))
        return static_cast<void*>(const_cast< Note*>(this));
    if (!strcmp(_clname, "Ui::Note"))
        return static_cast< Ui::Note*>(const_cast< Note*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int Note::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
