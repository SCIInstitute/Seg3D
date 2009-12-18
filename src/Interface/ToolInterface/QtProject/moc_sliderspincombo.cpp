/****************************************************************************
** Meta object code from reading C++ file 'sliderspincombo.h'
**
** Created: Fri Dec 18 15:00:28 2009
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sliderspincombo.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sliderspincombo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SliderSpinCombo[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      39,   16,   16,   16, 0x0a,
      57,   55,   16,   16, 0x0a,
      82,   16,   16,   16, 0x0a,
     106,   16,   16,   16, 0x08,
     126,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SliderSpinCombo[] = {
    "SliderSpinCombo\0\0valueAdjusted(double)\0"
    "setStep(double)\0,\0setRanges(double,double)\0"
    "setCurrentValue(double)\0setSliderValue(int)\0"
    "setSpinnerValue(double)\0"
};

const QMetaObject SliderSpinCombo::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SliderSpinCombo,
      qt_meta_data_SliderSpinCombo, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SliderSpinCombo::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SliderSpinCombo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SliderSpinCombo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SliderSpinCombo))
        return static_cast<void*>(const_cast< SliderSpinCombo*>(this));
    return QWidget::qt_metacast(_clname);
}

int SliderSpinCombo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: valueAdjusted((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: setStep((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: setRanges((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 3: setCurrentValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: setSliderValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: setSpinnerValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void SliderSpinCombo::valueAdjusted(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
