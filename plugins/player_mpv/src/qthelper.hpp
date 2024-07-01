#pragma once
#ifndef LIBMPV_QTHELPER_H_
#define LIBMPV_QTHELPER_H_

#include "../../qimgv/Common.h"
#include <mpv/client.h>
#include <cstring>
#include <QVariant>
#include <QString>
#include <QList>
#include <QHash>
#include <QSharedPointer>
#include <QMetaType>

namespace mpv::qt {

// Wrapper around mpv_handle. Does refcounting under the hood.
#if 1
class Handle
{
    struct container {
        explicit container(mpv_handle *h) : mpv(h) {}
        ~container() { mpv_terminate_destroy(mpv); }
        mpv_handle *mpv;
    };
    QSharedPointer<container> sptr;

  public:
    // Construct a new Handle from a raw mpv_handle with refcount 1. If the
    // last Handle goes out of scope, the mpv_handle will be destroyed with
    // mpv_terminate_destroy().
    // Never destroy the mpv_handle manually when using this wrapper. You
    // will create dangling pointers. Just let the wrapper take care of
    // destroying the mpv_handle.
    // Never create multiple wrappers from the same raw mpv_handle; copy the
    // wrapper instead (that's what it's for).
    ND static Handle FromRawHandle(mpv_handle *handle)
    {
        Handle h;
        h.sptr = QSharedPointer<container>(new container(handle));
        return h;
    }

    // Return the raw handle; for use with the libmpv C API.
    operator mpv_handle *() const
    {
        return sptr ? sptr->mpv : nullptr;
    }
};
#else
class Handle
{
    QSharedPointer<::mpv_handle> sptr;
  public:
    static Handle FromRawHandle(mpv_handle *handle)
    {
        Handle h;
        h.sptr = QSharedPointer<::mpv_handle>(handle, ::mpv_terminate_destroy);
        return h;
    }
    // Return the raw handle; for use with the libmpv C API.
    operator mpv_handle *() const
    {
        return sptr.get();
    }
};
#endif

inline QVariant node_to_variant(mpv_node const *node)
{
    switch (node->format) {
    case MPV_FORMAT_STRING:
        return {QString::fromUtf8(node->u.string)};
    case MPV_FORMAT_FLAG:
        return {static_cast<bool>(node->u.flag)};
    case MPV_FORMAT_INT64:
        return {static_cast<qlonglong>(node->u.int64)};
    case MPV_FORMAT_DOUBLE:
        return {node->u.double_};
    case MPV_FORMAT_NODE_ARRAY: {
        mpv_node_list *list = node->u.list;
        QVariantList   qlist;
        for (size_t n = 0; n < list->num; ++n)
            qlist.append(node_to_variant(&list->values[n]));
        return {qlist};
    }
    case MPV_FORMAT_NODE_MAP: {
        mpv_node_list *list = node->u.list;
        QVariantMap    qmap;
        for (size_t n = 0; n < list->num; ++n)
            qmap.insert(QString::fromUtf8(list->keys[n]), node_to_variant(&list->values[n]));
        return {qmap};
    }
    default: // MPV_FORMAT_NONE, unknown values (e.g. future extensions)
        return {};
    }
}

struct node_builder
{
    explicit node_builder(QVariant const &v)
    {
        set(&node_, v);
    }

    ~node_builder()
    {
        free_node(&node_);
    }

    DELETE_COPY_ROUTINES(node_builder);
    DEFAULT_MOVE_ROUTINES(node_builder);

    ND mpv_node       *node()       { return &node_; }
    ND mpv_node const *node() const { return &node_; }

  private:
    ND static mpv_node_list *create_list(mpv_node *dst, bool is_map, qsizetype num)
    {
        dst->format = is_map ? MPV_FORMAT_NODE_MAP : MPV_FORMAT_NODE_ARRAY;
        auto *list  = new mpv_node_list();
        dst->u.list = list;

        if (!list)
            goto err;
        list->values = new mpv_node[num]();
        if (!list->values)
            goto err;
        if (is_map) {
            list->keys = new char *[num]();
            if (!list->keys)
                goto err;
        }
        return list;

    err:
        free_node(dst);
        return nullptr;
    }

    ND static char *dup_qstring(QString const &s)
    {
        auto b = s.toUtf8();
        auto r = new char[b.size() + 1];
        if (r)
            memcpy(r, b.data(), b.size() + 1);
        return r;
    }

    ND static bool test_type(QVariant const &v, QMetaType::Type t)
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        // The Qt docs say: "Although this function is declared as returning
        // "QVariant::Type(obsolete), the return value should be interpreted
        // as QMetaType::Type."
        // So a cast really seems to be needed to avoid warnings (urgh).
        return static_cast<int>(v.type()) == static_cast<int>(t);
#else
        return v.typeId() == t;
#endif
    }

    static void set(mpv_node *dst, QVariant const &src)
    {
        if (test_type(src, QMetaType::QString)) {
            dst->format   = MPV_FORMAT_STRING;
            dst->u.string = dup_qstring(src.toString());
            if (!dst->u.string)
                goto fail;
        } else if (test_type(src, QMetaType::Bool)) {
            dst->format = MPV_FORMAT_FLAG;
            dst->u.flag = src.toBool() ? 1 : 0;
        } else if (test_type(src, QMetaType::Int) || test_type(src, QMetaType::LongLong)
                   || test_type(src, QMetaType::UInt) || test_type(src, QMetaType::ULongLong))
        {
            dst->format  = MPV_FORMAT_INT64;
            dst->u.int64 = src.toLongLong();
        } else if (test_type(src, QMetaType::Double)) {
            dst->format    = MPV_FORMAT_DOUBLE;
            dst->u.double_ = src.toDouble();
        } else if (src.canConvert<QVariantList>()) {
            QVariantList   qlist = src.toList();
            mpv_node_list *list  = create_list(dst, false, qlist.size());
            if (!list)
                goto fail;
            list->num = qlist.size();
            for (int n = 0; n < qlist.size(); n++)
                set(&list->values[n], qlist[n]);
        } else if (src.canConvert<QVariantMap>()) {
            QVariantMap    qmap = src.toMap();
            mpv_node_list *list = create_list(dst, true, qmap.size());
            if (!list)
                goto fail;
            list->num = qmap.size();
            for (int n = 0; n < qmap.size(); n++) {
                list->keys[n] = dup_qstring(qmap.keys()[n]);
                if (!list->keys[n]) {
                    free_node(dst);
                    goto fail;
                }
                set(&list->values[n], qmap.values()[n]);
            }
        } else {
            goto fail;
        }
        return;
    fail:
        dst->format = MPV_FORMAT_NONE;
    }

    static void free_node(mpv_node *dst)
    {
        switch (dst->format) {
        case MPV_FORMAT_STRING:
            delete[] dst->u.string;
            break;
        case MPV_FORMAT_NODE_ARRAY:
        case MPV_FORMAT_NODE_MAP: {
            mpv_node_list *list = dst->u.list;
            if (list) {
                for (int n = 0; n < list->num; ++n) {
                    if (list->keys)
                        delete[] list->keys[n];
                    if (list->values)
                        free_node(&list->values[n]);
                }
                delete[] list->keys;
                delete[] list->values;
            }
            delete list;
            break;
        }
        default:
            break;
        }

        dst->format = MPV_FORMAT_NONE;
    }
    
    mpv_node node_;
};

/**
 * RAII wrapper that calls mpv_free_node_contents() on the pointer.
 */
struct node_autofree {
    std::unique_ptr<::mpv_node, decltype(&::mpv_free_node_contents)> ptr;

    explicit node_autofree(::mpv_node *a_ptr)
        : ptr(a_ptr, ::mpv_free_node_contents)
    {}
};

/**
 * Return the given property as mpv_node converted to QVariant, or QVariant()
 * on error.
 *
 * @deprecated use get_property() instead
 *
 * @param ctx mpv context
 * @param name the property name
 */
[[deprecated("Use `set_property()`")]]
inline QVariant get_property_variant(mpv_handle *ctx, QString const &name)
{
    mpv_node node;
    if (mpv_get_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, &node) < 0)
        return {};
    node_autofree f(&node);
    return node_to_variant(&node);
}

/**
 * Set the given property as mpv_node converted from the QVariant argument.
 *
 * @deprecated use set_property() instead
 */
[[deprecated("Use `set_property()`")]]
inline int set_property_variant(mpv_handle *ctx, QString const &name, QVariant const &v)
{
    node_builder node(v);
    return mpv_set_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, node.node());
}

/**
 * Set the given option as mpv_node converted from the QVariant argument.
 *
 * @deprecated use set_property() instead
 */
[[deprecated("Use `set_property()`")]]
inline int set_option_variant(mpv_handle *ctx, QString const &name, QVariant const &v)
{
    node_builder node(v);
    return mpv_set_option(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, node.node());
}

/**
 * mpv_command_node() equivalent. Returns QVariant() on error (and unfortunately, the same on success).
 *
 * @deprecated use command() instead
 */
[[deprecated("Use `command()`")]]
inline QVariant command_variant(mpv_handle *ctx, QVariant const &args)
{
    node_builder node(args);
    mpv_node     res;
    if (mpv_command_node(ctx, node.node(), &res) < 0)
        return {};
    node_autofree f(&res);
    return node_to_variant(&res);
}

/**
 * This is used to return error codes wrapped in QVariant for functions which
 * return QVariant.
 *
 * You can use get_error() or is_error() to extract the error status from a
 * QVariant value.
 */
struct ErrorReturn
{
    /**
     * enum mpv_error value (or a value outside of it if ABI was extended)
     */
    int error;

    ErrorReturn() : error(0)
    {}
    explicit ErrorReturn(int err) : error(err)
    {}
};

/**
 * Return the mpv error code packed into a QVariant, or 0 (success) if it's not
 * an error value.
 *
 * @return error code (<0) or success (>=0)
 */
inline int get_error(QVariant const &v)
{
    if (!v.canConvert<ErrorReturn>())
        return 0;
    return v.value<ErrorReturn>().error;
}

/**
 * Return whether the QVariant carries a mpv error code.
 */
inline bool is_error(QVariant const &v)
{
    return get_error(v) < 0;
}

/**
 * Return the given property as mpv_node converted to QVariant, or QVariant()
 * on error.
 *
 * @param ctx The MPV handle.
 * @param name the property name
 * @return the property value, or an ErrorReturn with the error code
 */
inline QVariant get_property(mpv_handle *ctx, QString const &name)
{
    mpv_node node;
    int      err = mpv_get_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, &node);
    if (err < 0)
        return QVariant::fromValue(ErrorReturn(err));
    node_autofree f(&node);
    return node_to_variant(&node);
}

/**
 * Set the given property as mpv_node converted from the QVariant argument.
 *
 * @return mpv error code (<0 on error, >= 0 on success)
 */
inline int set_property(mpv_handle *ctx, QString const &name, QVariant const &v)
{
    node_builder node(v);
    return mpv_set_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, node.node());
}

/**
 * mpv_command_node() equivalent.
 *
 * @param ctx The MPV handle.
 * @param args command arguments, with args[0] being the command name as string
 * @return the property value, or an ErrorReturn with the error code
 */
inline QVariant command(mpv_handle *ctx, QVariant const &args)
{
    node_builder node(args);
    mpv_node     res;
    int          err = mpv_command_node(ctx, node.node(), &res);
    if (err < 0)
        return QVariant::fromValue(ErrorReturn(err));
    node_autofree f(&res);
    return node_to_variant(&res);
}

} // namespace mpv::qt

Q_DECLARE_METATYPE(mpv::qt::ErrorReturn)

#endif
