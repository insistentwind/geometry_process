/****************************************************************************
** Meta object code from reading C++ file 'async_mesh_processor.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../viewer/include/async_mesh_processor.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'async_mesh_processor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN17MeshProcessWorkerE_t {};
} // unnamed namespace

template <> constexpr inline auto MeshProcessWorker::qt_create_metaobjectdata<qt_meta_tag_ZN17MeshProcessWorkerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MeshProcessWorker",
        "finished",
        "",
        "std::vector<QVector3D>",
        "vertices",
        "std::vector<uint>",
        "indices",
        "error",
        "errorMessage",
        "progressUpdated",
        "progress",
        "process"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'finished'
        QtMocHelpers::SignalData<void(const std::vector<QVector3D> &, const std::vector<unsigned int> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Signal 'error'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Signal 'progressUpdated'
        QtMocHelpers::SignalData<void(int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 10 },
        }}),
        // Slot 'process'
        QtMocHelpers::SlotData<void(const std::vector<QVector3D> &, const std::vector<unsigned int> &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 5, 6 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MeshProcessWorker, qt_meta_tag_ZN17MeshProcessWorkerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MeshProcessWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17MeshProcessWorkerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17MeshProcessWorkerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17MeshProcessWorkerE_t>.metaTypes,
    nullptr
} };

void MeshProcessWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MeshProcessWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->finished((*reinterpret_cast< std::add_pointer_t<std::vector<QVector3D>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<std::vector<uint>>>(_a[2]))); break;
        case 1: _t->error((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->progressUpdated((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->process((*reinterpret_cast< std::add_pointer_t<std::vector<QVector3D>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<std::vector<uint>>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MeshProcessWorker::*)(const std::vector<QVector3D> & , const std::vector<unsigned int> & )>(_a, &MeshProcessWorker::finished, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (MeshProcessWorker::*)(const QString & )>(_a, &MeshProcessWorker::error, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (MeshProcessWorker::*)(int )>(_a, &MeshProcessWorker::progressUpdated, 2))
            return;
    }
}

const QMetaObject *MeshProcessWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MeshProcessWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17MeshProcessWorkerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MeshProcessWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void MeshProcessWorker::finished(const std::vector<QVector3D> & _t1, const std::vector<unsigned int> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void MeshProcessWorker::error(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void MeshProcessWorker::progressUpdated(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
namespace {
struct qt_meta_tag_ZN18AsyncMeshProcessorE_t {};
} // unnamed namespace

template <> constexpr inline auto AsyncMeshProcessor::qt_create_metaobjectdata<qt_meta_tag_ZN18AsyncMeshProcessorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AsyncMeshProcessor",
        "processingFinished",
        "",
        "std::vector<QVector3D>",
        "vertices",
        "std::vector<uint>",
        "indices",
        "processingStarted",
        "processingError",
        "errorMessage",
        "progressUpdated",
        "progress",
        "onWorkerFinished",
        "onWorkerError",
        "onWorkerProgress"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'processingFinished'
        QtMocHelpers::SignalData<void(const std::vector<QVector3D> &, const std::vector<unsigned int> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Signal 'processingStarted'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'processingError'
        QtMocHelpers::SignalData<void(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Signal 'progressUpdated'
        QtMocHelpers::SignalData<void(int)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 11 },
        }}),
        // Slot 'onWorkerFinished'
        QtMocHelpers::SlotData<void(const std::vector<QVector3D> &, const std::vector<unsigned int> &)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Slot 'onWorkerError'
        QtMocHelpers::SlotData<void(const QString &)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Slot 'onWorkerProgress'
        QtMocHelpers::SlotData<void(int)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AsyncMeshProcessor, qt_meta_tag_ZN18AsyncMeshProcessorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AsyncMeshProcessor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18AsyncMeshProcessorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18AsyncMeshProcessorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18AsyncMeshProcessorE_t>.metaTypes,
    nullptr
} };

void AsyncMeshProcessor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AsyncMeshProcessor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->processingFinished((*reinterpret_cast< std::add_pointer_t<std::vector<QVector3D>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<std::vector<uint>>>(_a[2]))); break;
        case 1: _t->processingStarted(); break;
        case 2: _t->processingError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->progressUpdated((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->onWorkerFinished((*reinterpret_cast< std::add_pointer_t<std::vector<QVector3D>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<std::vector<uint>>>(_a[2]))); break;
        case 5: _t->onWorkerError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->onWorkerProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AsyncMeshProcessor::*)(const std::vector<QVector3D> & , const std::vector<unsigned int> & )>(_a, &AsyncMeshProcessor::processingFinished, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AsyncMeshProcessor::*)()>(_a, &AsyncMeshProcessor::processingStarted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AsyncMeshProcessor::*)(const QString & )>(_a, &AsyncMeshProcessor::processingError, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AsyncMeshProcessor::*)(int )>(_a, &AsyncMeshProcessor::progressUpdated, 3))
            return;
    }
}

const QMetaObject *AsyncMeshProcessor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AsyncMeshProcessor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18AsyncMeshProcessorE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AsyncMeshProcessor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void AsyncMeshProcessor::processingFinished(const std::vector<QVector3D> & _t1, const std::vector<unsigned int> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void AsyncMeshProcessor::processingStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AsyncMeshProcessor::processingError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AsyncMeshProcessor::progressUpdated(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
