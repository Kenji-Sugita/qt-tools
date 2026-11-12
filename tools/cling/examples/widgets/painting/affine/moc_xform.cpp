/****************************************************************************
** Meta object code from reading C++ file 'xform.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "xform.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'xform.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
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
struct qt_meta_tag_ZN9XFormViewE_t {};
} // unnamed namespace

template <> constexpr inline auto XFormView::qt_create_metaobjectdata<qt_meta_tag_ZN9XFormViewE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "XFormView",
        "rotationChanged",
        "",
        "rotation",
        "scaleChanged",
        "scale",
        "shearChanged",
        "shear",
        "setAnimation",
        "animate",
        "updateControlPoints",
        "QPolygonF",
        "changeRotation",
        "changeScale",
        "changeShear",
        "setText",
        "setPixmap",
        "QPixmap",
        "setType",
        "XFormType",
        "t",
        "setVectorType",
        "setPixmapType",
        "setTextType",
        "reset",
        "type",
        "animation",
        "text",
        "pixmap",
        "VectorType",
        "PixmapType",
        "TextType"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'rotationChanged'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'scaleChanged'
        QtMocHelpers::SignalData<void(int)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 5 },
        }}),
        // Signal 'shearChanged'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 },
        }}),
        // Slot 'setAnimation'
        QtMocHelpers::SlotData<void(bool)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 9 },
        }}),
        // Slot 'updateControlPoints'
        QtMocHelpers::SlotData<void(const QPolygonF &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 2 },
        }}),
        // Slot 'changeRotation'
        QtMocHelpers::SlotData<void(int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'changeScale'
        QtMocHelpers::SlotData<void(int)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 5 },
        }}),
        // Slot 'changeShear'
        QtMocHelpers::SlotData<void(int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 },
        }}),
        // Slot 'setText'
        QtMocHelpers::SlotData<void(const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 2 },
        }}),
        // Slot 'setPixmap'
        QtMocHelpers::SlotData<void(const QPixmap &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 17, 2 },
        }}),
        // Slot 'setType'
        QtMocHelpers::SlotData<void(enum XFormType)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 19, 20 },
        }}),
        // Slot 'setVectorType'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setPixmapType'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setTextType'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'reset'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'type'
        QtMocHelpers::PropertyData<enum XFormType>(25, 0x80000000 | 19, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet),
        // property 'animation'
        QtMocHelpers::PropertyData<bool>(26, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'shear'
        QtMocHelpers::PropertyData<qreal>(7, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'rotation'
        QtMocHelpers::PropertyData<qreal>(3, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'scale'
        QtMocHelpers::PropertyData<qreal>(5, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'text'
        QtMocHelpers::PropertyData<QString>(27, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'pixmap'
        QtMocHelpers::PropertyData<QPixmap>(28, 0x80000000 | 17, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'XFormType'
        QtMocHelpers::EnumData<enum XFormType>(19, 19, QMC::EnumFlags{}).add({
            {   29, XFormType::VectorType },
            {   30, XFormType::PixmapType },
            {   31, XFormType::TextType },
        }),
    };
    return QtMocHelpers::metaObjectData<XFormView, qt_meta_tag_ZN9XFormViewE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject XFormView::staticMetaObject = { {
    QMetaObject::SuperData::link<ArthurFrame::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9XFormViewE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9XFormViewE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9XFormViewE_t>.metaTypes,
    nullptr
} };

void XFormView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<XFormView *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->rotationChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->scaleChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->shearChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->setAnimation((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->updateControlPoints((*reinterpret_cast<std::add_pointer_t<QPolygonF>>(_a[1]))); break;
        case 5: _t->changeRotation((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->changeScale((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->changeShear((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->setText((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->setPixmap((*reinterpret_cast<std::add_pointer_t<QPixmap>>(_a[1]))); break;
        case 10: _t->setType((*reinterpret_cast<std::add_pointer_t<enum XFormType>>(_a[1]))); break;
        case 11: _t->setVectorType(); break;
        case 12: _t->setPixmapType(); break;
        case 13: _t->setTextType(); break;
        case 14: _t->reset(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (XFormView::*)(int )>(_a, &XFormView::rotationChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (XFormView::*)(int )>(_a, &XFormView::scaleChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (XFormView::*)(int )>(_a, &XFormView::shearChanged, 2))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<enum XFormType*>(_v) = _t->type(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->animation(); break;
        case 2: *reinterpret_cast<qreal*>(_v) = _t->shear(); break;
        case 3: *reinterpret_cast<qreal*>(_v) = _t->rotation(); break;
        case 4: *reinterpret_cast<qreal*>(_v) = _t->scale(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->text(); break;
        case 6: *reinterpret_cast<QPixmap*>(_v) = _t->pixmap(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setType(*reinterpret_cast<enum XFormType*>(_v)); break;
        case 1: _t->setAnimation(*reinterpret_cast<bool*>(_v)); break;
        case 2: _t->setShear(*reinterpret_cast<qreal*>(_v)); break;
        case 3: _t->setRotation(*reinterpret_cast<qreal*>(_v)); break;
        case 4: _t->setScale(*reinterpret_cast<qreal*>(_v)); break;
        case 5: _t->setText(*reinterpret_cast<QString*>(_v)); break;
        case 6: _t->setPixmap(*reinterpret_cast<QPixmap*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *XFormView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *XFormView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9XFormViewE_t>.strings))
        return static_cast<void*>(this);
    return ArthurFrame::qt_metacast(_clname);
}

int XFormView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ArthurFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void XFormView::rotationChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void XFormView::scaleChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void XFormView::shearChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
namespace {
struct qt_meta_tag_ZN11XFormWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto XFormWidget::qt_create_metaobjectdata<qt_meta_tag_ZN11XFormWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "XFormWidget"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<XFormWidget, qt_meta_tag_ZN11XFormWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject XFormWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11XFormWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11XFormWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11XFormWidgetE_t>.metaTypes,
    nullptr
} };

void XFormWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<XFormWidget *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *XFormWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *XFormWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11XFormWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int XFormWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
