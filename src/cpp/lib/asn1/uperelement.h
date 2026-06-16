/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UPERELEMENT_H
#define KITINERARY_UPERELEMENT_H

#include "uperdecoder.h"
#include "internal/instance_counter.h"

#ifndef KITINERARY_HAS_OPTIONAL_POLYFILL
#include <optional>
#endif

namespace KItinerary {

// start of an ASN.1 SEQUENCE definition
#define UPER_GADGET \
    Q_GADGET \
    static constexpr detail::num<0> _uper_optional_counter(detail::num<0>) { return {}; } \
    static constexpr bool _uper_ExtensionMarker = false;

// same as UPER_GADGET, for SEQUENCES with an extension marker
#define UPER_EXTENDABLE_GADGET \
    Q_GADGET \
    static constexpr detail::num<0> _uper_optional_counter(detail::num<0>) { return {}; } \
    static constexpr bool _uper_ExtensionMarker = true;

// ASN.1 ENUMERATED definitions, with or without extension marker
// Count parameter provides the number of enum values (all UPER enums are sequential from 0)
#define UPER_ENUM(Name, Count) \
    Q_ENUM_NS(Name) \
    inline bool uperHasExtensionMarker(Name) { return false; } \
    inline int uperEnumKeyCount(Name) { return Count; }
#define UPER_EXTENABLE_ENUM(Name, Count) \
    Q_ENUM_NS(Name) \
    inline bool uperHasExtensionMarker(Name) { return true; } \
    inline int uperEnumKeyCount(Name) { return Count; }

#define UPER_ELEMENT(Type, Name) \
public: \
    Type Name = {}; /* NOLINT misc-non-private-member-variables-in-classes */ \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT) \
    inline bool Name ## IsSet() const { return true; } \
    std::optional<Type> Name ## Value() const { return Name; }

#define UPER_ELEMENT_OPTIONAL(Type, Name) \
public: \
    Type Name = {}; /* NOLINT misc-non-private-member-variables-in-classes */ \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT) \
    Q_PROPERTY(bool Name ## IsSet READ Name ## IsSet) \
private: \
    static constexpr int _uper_ ## Name ## OptionalIndex = decltype(_uper_optional_counter(detail::num<>()))::value; \
    static constexpr auto _uper_optional_counter(detail::num<decltype(_uper_optional_counter(detail::num<>()))::value + 1> n) \
        -> decltype(n) { return {}; } \
public: \
    inline bool Name ## IsSet() const { return m_optionals[m_optionals.size() - _uper_ ## Name ## OptionalIndex - 1]; } \
    std::optional<Type> Name ## Value() const { return Name ## IsSet() ? std::optional<Type>(Name) : std::optional<Type>(); }

#define UPER_ELEMENT_DEFAULT(Type, Name, DefaultValue) \
public: \
    Type Name = DefaultValue; /* NOLINT misc-non-private-member-variables-in-classes */ \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT) \
private: \
    static constexpr int _uper_ ## Name ## OptionalIndex = decltype(_uper_optional_counter(detail::num<>()))::value; \
    static constexpr auto _uper_optional_counter(detail::num<decltype(_uper_optional_counter(detail::num<>()))::value + 1> n) \
        -> decltype(n) { return {}; } \
    inline bool Name ## IsSet() const { return m_optionals[m_optionals.size() - _uper_ ## Name ## OptionalIndex - 1]; } \
    std::optional<Type> Name ## Value() const { return Name; }

#if !defined(KITINERARY_NO_STD_VARIANT)
#define UPER_ELEMENT_CHOICE(Name, ...) \
public: \
    std::variant<__VA_ARGS__> Name; /* NOLINT misc-non-private-member-variables-in-classes */ \
    Q_PROPERTY(QVariant Name READ Name ## Variant CONSTANT) \
    inline bool Name ## IsSet() const { return true; } \
    std::optional<std::variant<__VA_ARGS__>> Name ## Value() const { return Name; } \
private: \
    inline QVariant Name ## Variant() const { return QVariant::fromStdVariant(Name); }
#else
// C++11/14 fallback: UPER_ELEMENT_CHOICE stores result in QVariant since std::variant is unavailable
#define UPER_ELEMENT_CHOICE(Name, ...) \
public: \
    QVariant Name; /* NOLINT misc-non-private-member-variables-in-classes */ \
    Q_PROPERTY(QVariant Name MEMBER Name CONSTANT) \
    inline bool Name ## IsSet() const { return !Name.isNull(); }
#endif

}

#define UPER_GADGET_FINALIZE \
public: \
    void decode(UPERDecoder &decoder); \
private: \
    static constexpr auto _uper_OptionalCount = decltype(_uper_optional_counter(detail::num<>()))::value; \
    std::bitset<_uper_OptionalCount> m_optionals; \
    inline void decodeSequence(UPERDecoder &decoder) { \
        if (_uper_ExtensionMarker) { \
            if (decoder.readBoolean()) { \
                decoder.setError("SEQUENCE with extension marker set not implemented."); \
                return; \
            } \
        } \
        m_optionals = decoder.readBitset<_uper_OptionalCount>(); \
    }

#endif // KITINERARY_UPERELEMENT_H
