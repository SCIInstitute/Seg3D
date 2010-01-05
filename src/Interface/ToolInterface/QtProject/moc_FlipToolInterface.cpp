/****************************************************************************
** Meta object code from reading C++ file 'FlipToolInterface.h'
**
** Created: Tue Jan 5 13:59:46 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FlipToolInterface.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FlipToolInterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FlipToolInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x05,
      38,   18,   18,   18, 0x05,
      50,   18,   18,   18, 0x05,
      64,   18,   18,   18, 0x05,
      79,   18,   18,   18, 0x05,
     100,   18,   18,   18, 0x05,
     122,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
     144,   18,   18,   18, 0x0a,
     159,   18,   18,   18, 0x0a,
     185,   18,   18,   18, 0x08,
     209,   18,   18,   18, 0x08,
     226,   18,   18,   18, 0x08,
     245,   18,   18,   18, 0x08,
     265,   18,   18,   18, 0x08,
     291,   18,   18,   18, 0x08,
     317,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FlipToolInterface[] = {
    "FlipToolInterface\0\0activeChanged(int)\0"
    "flipAxial()\0flipCoronal()\0flipSagittal()\0"
    "rotateAxialCoronal()\0rotateAxialSagittal()\0"
    "rotateSagittalAxial()\0setActive(int)\0"
    "addToActive(QStringList&)\0"
    "senseActiveChanged(int)\0senseFlipAxial()\0"
    "senseFlipCoronal()\0senseFlipSagittal()\0"
    "senseRotateAxialCoronal()\0"
    "senseRotateAxialSagital()\0"
    "senseRotateSagittalAxial()\0"
};

const QMetaObject FlipToolInterface::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_FlipToolInterface,
      qt_meta_data_FlipToolInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FlipToolInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FlipToolInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FlipToolInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FlipToolInterface))
        return static_cast<void*>(const_cast< FlipToolInterface*>(this));
    return QWidget::qt_metacast(_clname);
}

int FlipToolInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: activeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: flipAxial(); break;
        case 2: flipCoronal(); break;
        case 3: flipSagittal(); break;
        case 4: rotateAxialCoronal(); break;
        case 5: rotateAxialSagittal(); break;
        case 6: rotateSagittalAxial(); break;
        case 7: setActive((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: addToActive((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 9: senseActiveChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: senseFlipAxial(); break;
        case 11: senseFlipCoronal(); break;
        case 12: senseFlipSagittal(); break;
        case 13: senseRotateAxialCoronal(); break;
        case 14: senseRotateAxialSagital(); break;
        case 15: senseRotateSagittalAxial(); break;
        default: ;
        }
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void FlipToolInterface::activeChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FlipToolInterface::flipAxial()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void FlipToolInterface::flipCoronal()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void FlipToolInterface::flipSagittal()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void FlipToolInterface::rotateAxialCoronal()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void FlipToolInterface::rotateAxialSagittal()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void FlipToolInterface::rotateSagittalAxial()
{
    QMetaObject::activate(this, &staticMetaObject, 6, 0);
}
QT_END_MOC_NAMESPACE
