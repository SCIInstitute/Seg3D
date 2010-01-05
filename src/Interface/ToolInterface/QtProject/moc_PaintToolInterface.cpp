/****************************************************************************
** Meta object code from reading C++ file 'PaintToolInterface.h'
**
** Created: Tue Jan 5 13:59:44 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "PaintToolInterface.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PaintToolInterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PaintToolInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   19,   19,   19, 0x05,
      39,   19,   19,   19, 0x05,
      56,   19,   19,   19, 0x05,
      79,   19,   19,   19, 0x05,
     106,   19,   19,   19, 0x05,
     133,   19,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
     160,   19,   19,   19, 0x0a,
     175,   19,   19,   19, 0x0a,
     201,   19,   19,   19, 0x0a,
     214,   19,   19,   19, 0x0a,
     238,   19,   19,   19, 0x0a,
     263,  261,   19,   19, 0x0a,
     296,   19,   19,   19, 0x0a,
     326,  261,   19,   19, 0x0a,
     359,   19,   19,   19, 0x0a,
     389,   19,   19,   19, 0x08,
     413,   19,   19,   19, 0x08,
     435,   19,   19,   19, 0x08,
     463,   19,   19,   19, 0x08,
     498,   19,   19,   19, 0x08,
     533,   19,   19,   19, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_PaintToolInterface[] = {
    "PaintToolInterface\0\0activeChanged(int)\0"
    "maskChanged(int)\0eraseModeChanged(bool)\0"
    "paintBrushSizeChanged(int)\0"
    "upperThresholdChanged(int)\0"
    "lowerThresholdChanged(int)\0setActive(int)\0"
    "addToActive(QStringList&)\0setMask(int)\0"
    "addToMask(QStringList&)\0setPaintBrushSize(int)\0"
    ",\0setLowerThreshold(double,double)\0"
    "setLowerThresholdStep(double)\0"
    "setUpperThreshold(double,double)\0"
    "setUpperThresholdStep(double)\0"
    "senseActiveChanged(int)\0senseMaskChanged(int)\0"
    "senseEraseModeChanged(bool)\0"
    "sensePaintBrushSizeChanged(double)\0"
    "senseUpperThresholdChanged(double)\0"
    "senselowerThresholdChanged(double)\0"
};

const QMetaObject PaintToolInterface::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PaintToolInterface,
      qt_meta_data_PaintToolInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PaintToolInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PaintToolInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PaintToolInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PaintToolInterface))
        return static_cast<void*>(const_cast< PaintToolInterface*>(this));
    return QWidget::qt_metacast(_clname);
}

int PaintToolInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: activeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: maskChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: eraseModeChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: paintBrushSizeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: upperThresholdChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: lowerThresholdChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: setActive((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: addToActive((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 8: setMask((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: addToMask((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 10: setPaintBrushSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: setLowerThreshold((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 12: setLowerThresholdStep((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 13: setUpperThreshold((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 14: setUpperThresholdStep((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 15: senseActiveChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: senseMaskChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: senseEraseModeChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 18: sensePaintBrushSizeChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 19: senseUpperThresholdChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 20: senselowerThresholdChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 21;
    }
    return _id;
}

// SIGNAL 0
void PaintToolInterface::activeChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PaintToolInterface::maskChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PaintToolInterface::eraseModeChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PaintToolInterface::paintBrushSizeChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PaintToolInterface::upperThresholdChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PaintToolInterface::lowerThresholdChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE
