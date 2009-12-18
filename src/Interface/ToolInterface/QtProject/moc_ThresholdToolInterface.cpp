/****************************************************************************
** Meta object code from reading C++ file 'ThresholdToolInterface.h'
**
** Created: Fri Dec 18 15:00:33 2009
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ThresholdToolInterface.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ThresholdToolInterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ThresholdToolInterface[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x05,
      43,   23,   23,   23, 0x05,
      66,   23,   23,   23, 0x05,
      79,   23,   23,   23, 0x05,
     106,   23,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
     133,   23,   23,   23, 0x0a,
     148,   23,   23,   23, 0x0a,
     176,  174,   23,   23, 0x0a,
     209,   23,   23,   23, 0x0a,
     239,  174,   23,   23, 0x0a,
     272,   23,   23,   23, 0x0a,
     302,   23,   23,   23, 0x0a,
     317,   23,   23,   23, 0x08,
     341,   23,   23,   23, 0x08,
     369,   23,   23,   23, 0x08,
     387,   23,   23,   23, 0x08,
     422,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ThresholdToolInterface[] = {
    "ThresholdToolInterface\0\0activeChanged(int)\0"
    "createThresholdLayer()\0clearSeeds()\0"
    "upperThresholdChanged(int)\0"
    "lowerThresholdChanged(int)\0setActive(int)\0"
    "addToActive(QStringList&)\0,\0"
    "setLowerThreshold(double,double)\0"
    "setLowerThresholdStep(double)\0"
    "setUpperThreshold(double,double)\0"
    "setUpperThresholdStep(double)\0"
    "setHistogram()\0senseActiveChanged(int)\0"
    "senseCreateThresholdLayer()\0"
    "senseClearSeeds()\0senseUpperThresholdChanged(double)\0"
    "senselowerThresholdChanged(double)\0"
};

const QMetaObject ThresholdToolInterface::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ThresholdToolInterface,
      qt_meta_data_ThresholdToolInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ThresholdToolInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ThresholdToolInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ThresholdToolInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ThresholdToolInterface))
        return static_cast<void*>(const_cast< ThresholdToolInterface*>(this));
    return QWidget::qt_metacast(_clname);
}

int ThresholdToolInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: activeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: createThresholdLayer(); break;
        case 2: clearSeeds(); break;
        case 3: upperThresholdChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: lowerThresholdChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: setActive((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: addToActive((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 7: setLowerThreshold((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 8: setLowerThresholdStep((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: setUpperThreshold((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 10: setUpperThresholdStep((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 11: setHistogram(); break;
        case 12: senseActiveChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: senseCreateThresholdLayer(); break;
        case 14: senseClearSeeds(); break;
        case 15: senseUpperThresholdChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 16: senselowerThresholdChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void ThresholdToolInterface::activeChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ThresholdToolInterface::createThresholdLayer()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void ThresholdToolInterface::clearSeeds()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void ThresholdToolInterface::upperThresholdChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ThresholdToolInterface::lowerThresholdChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
