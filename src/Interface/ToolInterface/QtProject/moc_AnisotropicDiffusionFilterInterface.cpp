/****************************************************************************
** Meta object code from reading C++ file 'AnisotropicDiffusionFilterInterface.h'
**
** Created: Mon Jan 18 16:35:28 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "AnisotropicDiffusionFilterInterface.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AnisotropicDiffusionFilterInterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AnisotropicDiffusionFilterInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      37,   36,   36,   36, 0x05,
      56,   36,   36,   36, 0x05,
      79,   36,   36,   36, 0x05,
      99,   36,   36,   36, 0x05,
     123,   36,   36,   36, 0x05,

 // slots: signature, parameters, type, tag, flags
     139,   36,   36,   36, 0x0a,
     154,   36,   36,   36, 0x0a,
     180,   36,   36,   36, 0x0a,
     201,  199,   36,   36, 0x0a,
     228,   36,   36,   36, 0x0a,
     244,  199,   36,   36, 0x0a,
     272,   36,   36,   36, 0x0a,
     292,  199,   36,   36, 0x0a,
     321,   36,   36,   36, 0x08,
     345,   36,   36,   36, 0x08,
     373,   36,   36,   36, 0x08,
     398,   36,   36,   36, 0x08,
     427,   36,   36,   36, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_AnisotropicDiffusionFilterInterface[] = {
    "AnisotropicDiffusionFilterInterface\0"
    "\0activeChanged(int)\0iterationsChanged(int)\0"
    "stepChanged(double)\0conductanceChanged(int)\0"
    "filterRun(bool)\0setActive(int)\0"
    "addToActive(QStringList&)\0setIterations(int)\0"
    ",\0setIterationRange(int,int)\0"
    "setStep(double)\0setStepRange(double,double)\0"
    "setConductance(int)\0setConductanceRange(int,int)\0"
    "senseActiveChanged(int)\0"
    "senseIterationsChanged(int)\0"
    "senseStepChanged(double)\0"
    "senseConductanceChanged(int)\0"
    "senseFilterRun(bool)\0"
};

const QMetaObject AnisotropicDiffusionFilterInterface::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_AnisotropicDiffusionFilterInterface,
      qt_meta_data_AnisotropicDiffusionFilterInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AnisotropicDiffusionFilterInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AnisotropicDiffusionFilterInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AnisotropicDiffusionFilterInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AnisotropicDiffusionFilterInterface))
        return static_cast<void*>(const_cast< AnisotropicDiffusionFilterInterface*>(this));
    return QWidget::qt_metacast(_clname);
}

int AnisotropicDiffusionFilterInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: activeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: iterationsChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: stepChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: conductanceChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: filterRun((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: setActive((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: addToActive((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 7: setIterations((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: setIterationRange((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: setStep((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 10: setStepRange((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 11: setConductance((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: setConductanceRange((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 13: senseActiveChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: senseIterationsChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: senseStepChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 16: senseConductanceChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: senseFilterRun((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void AnisotropicDiffusionFilterInterface::activeChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AnisotropicDiffusionFilterInterface::iterationsChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AnisotropicDiffusionFilterInterface::stepChanged(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void AnisotropicDiffusionFilterInterface::conductanceChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void AnisotropicDiffusionFilterInterface::filterRun(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
