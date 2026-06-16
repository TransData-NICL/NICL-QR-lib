/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_VARIANTVISITOR_H
#define KITINERARY_VARIANTVISITOR_H

#include <QVariant>

#include <type_traits>

namespace KItinerary {

/** Like std::visit, but for a fixed set of types inside a QVariant. */
template <typename Func>
class VariantVisitor
{
private:
    Func m_func;

public:
    explicit VariantVisitor(Func func)
        : m_func(func) {}

    template <typename T, typename ...Ts>
    auto inline visit(const QVariant &v) -> decltype(m_func(v.value<T>())) {
        typedef decltype(m_func(v.value<T>())) RetT;
        return visitImpl<RetT, T, Ts...>(v);
    }

private:
    template <typename RetT>
    inline typename std::enable_if<std::is_same<RetT, void>::value>::type
    visitImplDefault(const QVariant&) {
        return;
    }
    template <typename RetT>
    inline typename std::enable_if<!std::is_same<RetT, void>::value, RetT>::type
    visitImplDefault(const QVariant&) {
        return RetT{};
    }
    template <typename RetT, typename T, typename ...Ts>
    inline RetT visitImpl(const QVariant &v) {
        if (v.userType() == qMetaTypeId<T>()) {
            return m_func(v.value<T>());
        }
        return visitImplNext<RetT, Ts...>(v);
    }
    template <typename RetT>
    inline RetT visitImplNext(const QVariant &v) {
        return visitImplDefault<RetT>(v);
    }
    template <typename RetT, typename T, typename ...Ts>
    inline RetT visitImplNext(const QVariant &v) {
        return visitImpl<RetT, T, Ts...>(v);
    }
};

/** Helper factory function for VariantVisitor (needed for C++14 where CTAD is unavailable). */
template <typename Func>
inline VariantVisitor<Func> makeVariantVisitor(Func func)
{
    return VariantVisitor<Func>(func);
}

template <typename T, typename U, typename ...Us>
struct is_any_of : std::conditional<std::is_same<typename std::decay<T>::type, U>::value, std::true_type, is_any_of<T, Us...>>::type {};

template <typename T, typename U>
struct is_any_of<T, U> : std::is_same<typename std::decay<T>::type, U>{};

template <typename T, typename U, typename ...Us>
struct is_any_of_val {
    static const bool value = is_any_of<T, U, Us...>::value;
};

// C++17 compat alias
#if __cplusplus >= 201703L
template <typename T, typename U, typename ...Us>
inline constexpr bool is_any_of_v = is_any_of<T, U, Us...>::value;
#endif

}

#endif
