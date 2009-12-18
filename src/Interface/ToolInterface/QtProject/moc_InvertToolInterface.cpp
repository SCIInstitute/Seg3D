/****************************************************************************
** Meta object code from reading C++ file 'InvertToolInterface.h'
**
** Created: Fri Dec 18 15:00:31 2009
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "InvertToolInterface.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'InvertToolInterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_InvertToolInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,
      40,   20,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      53,   20,   20,   20, 0x0a,
      68,   20,   20,   20, 0x0a,
      94,   20,   20,   20, 0x08,
     118,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_InvertToolInterface[] = {
    "InvertToolInterface\0\0activeChanged(int)\0"
    "invert(bool)\0setActive(int)\0"
    "addToActive(QStringList&)\0"
    "senseActiveChanged(int)\0senseInverted()\0"
};

const QMetaObject InvertToolInterface::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_InvertToolInterface,
      qt_meta_data_InvertToolInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &InvertToolInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *InvertToolInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *InvertToolInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_InvertToolInterface))
        return static_cast<void*>(const_cast< InvertToolInterface*>(this));
    return QWidget::qt_metacast(_clname);
}

int InvertToolInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: activeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: invert((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: setActive((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: addToActive((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 4: senseActiveChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: senseInverted(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void InvertToolInterface::activeChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void InvertToolInterface::invert(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
