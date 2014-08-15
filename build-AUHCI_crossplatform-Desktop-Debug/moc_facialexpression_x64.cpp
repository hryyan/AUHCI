/****************************************************************************
** Meta object code from reading C++ file 'facialexpression_x64.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../AUHCI_crossplatform/facialexpression_x64.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'facialexpression_x64.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ProcessThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      37,   15,   14,   14, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_ProcessThread[] = {
    "ProcessThread\0\0,face,gabor,au_appear\0"
    "Write(QImage*,QImage*,QImage*,bool*)\0"
};

void ProcessThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ProcessThread *_t = static_cast<ProcessThread *>(_o);
        switch (_id) {
        case 0: _t->Write((*reinterpret_cast< QImage*(*)>(_a[1])),(*reinterpret_cast< QImage*(*)>(_a[2])),(*reinterpret_cast< QImage*(*)>(_a[3])),(*reinterpret_cast< bool*(*)>(_a[4]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ProcessThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ProcessThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_ProcessThread,
      qt_meta_data_ProcessThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ProcessThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ProcessThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ProcessThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ProcessThread))
        return static_cast<void*>(const_cast< ProcessThread*>(this));
    return QThread::qt_metacast(_clname);
}

int ProcessThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void ProcessThread::Write(QImage * _t1, QImage * _t2, QImage * _t3, bool * _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_FacialExpressionX64[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x0a,
      39,   20,   20,   20, 0x0a,
      52,   20,   20,   20, 0x0a,
      67,   20,   20,   20, 0x0a,
      83,   20,   20,   20, 0x0a,
     101,   20,   20,   20, 0x0a,
     111,   20,   20,   20, 0x0a,
     125,   20,   20,   20, 0x0a,
     140,   20,   20,   20, 0x0a,
     158,   20,   20,   20, 0x0a,
     182,  178,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FacialExpressionX64[] = {
    "FacialExpressionX64\0\0ProcessOneFrame()\0"
    "ProcessPic()\0ProcessVideo()\0ProcessCamera()\0"
    "ProcessSequence()\0StopAll()\0CaptureFace()\0"
    "CaptureGabor()\0ChangeDirectory()\0"
    "ChangeInterval(int)\0,,,\0"
    "Read(QImage*,QImage*,QImage*,bool*)\0"
};

void FacialExpressionX64::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FacialExpressionX64 *_t = static_cast<FacialExpressionX64 *>(_o);
        switch (_id) {
        case 0: _t->ProcessOneFrame(); break;
        case 1: _t->ProcessPic(); break;
        case 2: _t->ProcessVideo(); break;
        case 3: _t->ProcessCamera(); break;
        case 4: _t->ProcessSequence(); break;
        case 5: _t->StopAll(); break;
        case 6: _t->CaptureFace(); break;
        case 7: _t->CaptureGabor(); break;
        case 8: _t->ChangeDirectory(); break;
        case 9: _t->ChangeInterval((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->Read((*reinterpret_cast< QImage*(*)>(_a[1])),(*reinterpret_cast< QImage*(*)>(_a[2])),(*reinterpret_cast< QImage*(*)>(_a[3])),(*reinterpret_cast< bool*(*)>(_a[4]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FacialExpressionX64::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FacialExpressionX64::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_FacialExpressionX64,
      qt_meta_data_FacialExpressionX64, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FacialExpressionX64::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FacialExpressionX64::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FacialExpressionX64::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FacialExpressionX64))
        return static_cast<void*>(const_cast< FacialExpressionX64*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int FacialExpressionX64::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
