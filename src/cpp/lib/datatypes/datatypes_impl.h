/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "internal/instance_counter.h"
#include "internal/parameter_type.h"
#include "internal/strict_equal.h"
#include "internal/strict_less.h"

namespace KItinerary {

///@cond internal
namespace detail {

// SFINAE helper to determine if we have a polymorphic or a simple value type
template <typename T>
struct base_type {
    template <typename U> static typename U::super_type test(typename U::super_type*);
    template <typename U> static T test(...);
    using type = decltype(test<T>(nullptr));
    static constexpr const bool is_valid = !std::is_same<type, T>::value;
};
}

#define KITINERARY_MAKE_CLASS_IMPL(Class) \
Q_GLOBAL_STATIC_WITH_ARGS(QExplicitlySharedDataPointer<Class ## Private>, s_ ## Class ## _shared_null, (new Class ## Private)) \
Class::Class() : Class(s_ ## Class ## _shared_null()->data()) {} \
Class::Class(const Class&) = default; \
Class::~Class() = default; \
Class& Class::operator=(const Class &other) { d = other.d; return *this; } \
QString Class::className() const { return QStringLiteral(#Class); } \
Class::operator QVariant() const { return QVariant::fromValue(*this); } \
const char* Class::typeName() { return #Class; } \
static_assert(sizeof(Class) == sizeof(void*), "dptr must be the only member!"); \
namespace detail { \
    static constexpr int property_counter(KItinerary::detail::num<0>, KItinerary::detail::tag<Class>) \
    { return 1; } \
    static constexpr bool property_equals(KItinerary::detail::num<0>, KItinerary::detail::tag<Class ## Private>, const Class ## Private *, const Class ## Private *) \
    { return true; } \
    static constexpr bool property_less(KItinerary::detail::num<0>, KItinerary::detail::tag<Class ## Private>, const Class ## Private *, const Class ## Private *) \
    { return true; } \
}
///@endcond

/** Macro to generate the value type and introspection implementation for a vocabulary type.
 *  This provides the implementation of KITINERARY_GADGET.
 */
#define KITINERARY_MAKE_CLASS(Class) \
KITINERARY_MAKE_CLASS_IMPL(Class) \
Class::Class(Class ## Private *dd) : d(dd) {}

/** Macro to generate the value type and introspection implementation for a derived vocabulary type.
 *  This provides the implementation of KITINERARY_GADGET.
 */
#define KITINERARY_MAKE_DERIVED_CLASS(Class, Base) \
KITINERARY_MAKE_CLASS_IMPL(Class) \
Class::Class(Class ## Private *dd) : Base(dd) {}

/** Macros to generate operator implementation details for a property.
 *  This is not needed when using KITINERARY_MAKE_PROPERTY, but only when
 *  implementing getters/setters manually.
 */
#define KITINERARY_MAKE_PROPERTY_OPERATOR(Class, Type, Name) \
namespace detail { \
    static constexpr int property_counter(KItinerary::detail::num<property_counter(KItinerary::detail::num<>(), KItinerary::detail::tag<Class>())> n, KItinerary::detail::tag<Class>) \
    { return decltype(n)::value + 1; } \
    static inline bool property_equals(KItinerary::detail::num<property_counter(KItinerary::detail::num<>(), KItinerary::detail::tag<Class>())> n, KItinerary::detail::tag<Class ## Private>, const Class ## Private *lhs, const Class ## Private *rhs) \
    { \
        if (KItinerary::detail::strict_equal<Type>(lhs->Name, rhs->Name)) { \
            return property_equals(n.prev(), KItinerary::detail::tag<Class ## Private>(), lhs, rhs); \
        } \
        return false; \
    } \
    static inline bool property_less(KItinerary::detail::num<property_counter(KItinerary::detail::num<>(), KItinerary::detail::tag<Class>())> n, KItinerary::detail::tag<Class ## Private>, const Class ## Private *lhs, const Class ## Private *rhs) \
    { \
        if (KItinerary::detail::strict_less<Type>(lhs->Name, rhs->Name)) { return true; } \
        if (KItinerary::detail::strict_equal<Type>(lhs->Name, rhs->Name)) { \
            return property_less(n.prev(), KItinerary::detail::tag<Class ## Private>(), lhs, rhs); \
        } \
        return false; \
    } \
}

/** Macro to generate the implementation of a vocabulary property type.
 *  This generates the definitions for the declaration the KITINERARY_PROPERTY macro
 *  produces, as well as implementation details needed for the automatic comparison
 *  operator.
 *  @see KITINERARY_MAKE_OPERATOR
 */
#define KITINERARY_MAKE_PROPERTY(Class, Type, Name, SetName) \
Type Class::Name() const { return static_cast<const Class ## Private*>(d.data())->Name; } \
void Class::SetName(KItinerary::detail::parameter_type<Type>::type value) { \
    if (KItinerary::detail::strict_equal<Type>(static_cast<Class ## Private*>(d.data())->Name, value)) { return; } \
    d.detach(); \
    static_cast<Class ## Private*>(d.data())->Name = value; \
} \
KITINERARY_MAKE_PROPERTY_OPERATOR(Class, Type, Name)

/** Generates the implementation of the comparison operator for vocabulary type @p Class.
 *  The generated operator==() implementation will check all properties for strict equality,
 *  as well as call operator==() of a base class if present.
 *  This relies on KITINERARY_MAKE_PROPERTY to generate supporting code and thus has to be
 *  called after all properties have been generated.
 */

// C++11-compatible dispatch helpers replacing if constexpr for recursive comparison
namespace detail {
template<typename T, bool has_base = base_type<T>::is_valid>
struct recursive_less_helper {
    static bool call(const T *, const T *) { return false; }
};

template<typename T>
struct recursive_less_helper<T, true> {
    static bool call(const T *lhs, const T *rhs) {
        typedef typename base_type<T>::type super_type;
        if (property_equals(num<>(), tag<T>(), lhs, rhs)) {
            if (property_less(num<>(), tag<super_type>(),
                    static_cast<const super_type*>(lhs),
                    static_cast<const super_type*>(rhs))) { return true; }
            return recursive_less_helper<super_type>::call(
                static_cast<const super_type*>(lhs),
                static_cast<const super_type*>(rhs));
        }
        return false;
    }
};

template<typename T, bool has_base = base_type<T>::is_valid>
struct recursive_equal_helper {
    static bool call(const T *, const T *) { return true; }
};

template<typename T>
struct recursive_equal_helper<T, true> {
    static bool call(const T *lhs, const T *rhs) {
        typedef typename base_type<T>::type super_type;
        if (!property_equals(num<>(), tag<super_type>(),
                static_cast<const super_type*>(lhs),
                static_cast<const super_type*>(rhs))) { return false; }
        return recursive_equal_helper<super_type>::call(
            static_cast<const super_type*>(lhs),
            static_cast<const super_type*>(rhs));
    }
};
} // namespace detail

#define KITINERARY_MAKE_OPERATOR(Class) \
bool Class::operator<(const Class &other) const { \
    static_assert(detail::property_counter(KItinerary::detail::num<0>(), KItinerary::detail::tag<Class>()) == 1, "silence unused function warnings"); \
    typedef Class ## Private this_type; \
    const auto lhs = static_cast<const this_type *>(d.data()); \
    const auto rhs = static_cast<const this_type*>(other.d.data()); \
    if (lhs == rhs) { \
        return false; \
    } \
    if (detail::property_less(KItinerary::detail::num<>(), KItinerary::detail::tag<this_type>(), lhs, rhs)) { \
        return true; \
    } \
    return detail::recursive_less_helper<this_type>::call(lhs, rhs); \
} \
bool Class::operator==(const Class &other) const \
{ \
    static_assert(detail::property_counter(KItinerary::detail::num<0>(), KItinerary::detail::tag<Class>()) == 1, "silence unused function warnings"); \
    typedef Class ## Private this_type; \
    const auto lhs = static_cast<const this_type *>(d.data()); \
    const auto rhs = static_cast<const this_type*>(other.d.data()); \
    if (lhs == rhs) { \
        return true; \
    } \
    if (!detail::property_equals(KItinerary::detail::num<>(), KItinerary::detail::tag<this_type>(), lhs, rhs)) { \
        return false; \
    } \
    return detail::recursive_equal_helper<this_type>::call(lhs, rhs); \
}

}
