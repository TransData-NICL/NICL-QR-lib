/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UPERDECODER_H
#define KITINERARY_UPERDECODER_H

#include "bitvectorview.h"

#include <QMetaEnum>

#if __cplusplus >= 201703L
#include <variant>
#endif

namespace KItinerary {

/** Decoder for data encoded according to X.691 ASN.1 Unaligned Packed Encoding Rules (UPER). */
class UPERDecoder
{
public:
    explicit UPERDecoder(BitVectorView data);
    ~UPERDecoder();

    using size_type = BitVectorView::size_type;

    size_type offset() const;
    void seek(size_type index);

    /** Read constrained whole number from the current position.
     *  @see X.691 §11.5
     */
    int64_t readConstrainedWholeNumber(int64_t minimum, int64_t maximum);

    /** Read unconstrained whole number.
     *  @see X.691 §11.8
     */
    int64_t readUnconstrainedWholeNumber();

    /** Read length determinant.
     * @see X.691 §11.9
     */
    size_type readLengthDeterminant();

    /** Read UTF-8 string. */
    QString readUtf8String();

    /** Read boolean value. */
    bool readBoolean();

    /** Read an unconstrained IA5String (7 bit ASCII).
     *  @see X.691 §30
     */
    QByteArray readIA5String();

    /** Read length-constrained IA5String (7 bit ASCII).
     *  @see X.691 §30
     */
    QByteArray readIA5String(size_type minLenght, size_type maxLength);

    /** Read unconstrained octet string (8 bit data).
     * @see X.691 §17
     */
    QByteArray readOctetString();

    /** Read an @tparam N sized bitmap. */
    template <std::size_t N>
    inline std::bitset<N> readBitset()
    {
        auto result = m_data.bitsetAt<N>(m_idx);
        m_idx += N;
        return result;
    }

    /** Read a sequence-of field with unrestricted size.
     *  @see X.691 §20
     */
    template <typename T>
    inline QList<T> readSequenceOf()
    {
        const auto size = readLengthDeterminant();
        QList<T> result;
        result.reserve(size);
        for (size_type i = 0; i < size; ++i) {
            T element;
            element.decode(*this);
            result.push_back(std::move(element));
        }
        return result;
    }
    QList<int> readSequenceOfConstrainedWholeNumber(int64_t minimum, int64_t maximum);
    QList<int> readSequenceOfUnconstrainedWholeNumber();
    QList<QByteArray> readSequenceOfIA5String();
    QList<QString> readSequenceOfUtf8String();

    /** Read an Object Identifier value.
     *  @see X.691 §24
     */
    QByteArray readObjectIdentifier();

    /** Read enumerated value.
     *  @see X.691 §14
     *  Overload taking an explicit key count, for use when QMetaEnum::fromType
     *  is unavailable (Qt < 5.8 where Q_ENUM_NS does not exist).
     */
    template <typename T>
    inline T readEnumerated(int keyCount)
    {
        const auto idx = readConstrainedWholeNumber(0, keyCount - 1);
        return static_cast<T>(idx);
    }
    template <typename T>
    inline T readEnumeratedWithExtensionMarker(int keyCount)
    {
        if (readBoolean()) {
            setError("CHOICE with extension marker set not implemented.");
            return {};
        }
        return readEnumerated<T>(keyCount);
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    /** Read enumerated value using QMetaEnum (requires Q_ENUM_NS registration).
     *  @see X.691 §14
     */
    template <typename T>
    inline T readEnumerated()
    {
        const auto me = QMetaEnum::fromType<T>();
        const auto idx = readConstrainedWholeNumber(0, me.keyCount() - 1);
        return static_cast<T>(me.value(idx));
    }
    template <typename T>
    inline T readEnumeratedWithExtensionMarker()
    {
        if (readBoolean()) {
            setError("CHOICE with extension marker set not implemented.");
            return {};
        }
        return readEnumerated<T>();
    }
#endif

    /** Read a choice value.
     *  @tparam T a std::variant with the alternatives covered by the choice value.
     *  @see X.691 §23
     */
#if !defined(KITINERARY_NO_STD_VARIANT)
    template <typename T>
    T readChoiceWithExtensionMarker()
    {
        if (readBoolean()) {
            setError("CHOICE with extension marker set not implemented.");
            return {};
        }
        constexpr auto count = std::variant_size_v<T>;
        const auto choiceIdx = readConstrainedWholeNumber(0, count - 1);
        if (choiceIdx > (int)count) {
            setError("Invalid CHOICE index.");
            return {};
        }
        return readChoiceElement<T>(choiceIdx);
    }
#else
    /** C++14 fallback: read a choice value into QVariant.
     *  @tparam Types... the alternative types in the choice.
     */
    template <typename... Types>
    QVariant readChoiceWithExtensionMarker()
    {
        if (readBoolean()) {
            setError("CHOICE with extension marker set not implemented.");
            return {};
        }
        const int count = sizeof...(Types);
        const auto choiceIdx = readConstrainedWholeNumber(0, count - 1);
        if (choiceIdx > count) {
            setError("Invalid CHOICE index.");
            return {};
        }
        return readChoiceElementQV<Types...>(choiceIdx, 0);
    }
#endif

    /** Reading at any point encountered an error.
     *  As uPER gives us no way to recover from that, everything
     *  read has to be considered invalid.
     */
    bool hasError() const;
    QByteArray errorMessage() const;

    /** Put the decoder into the error state. */
    void setError(const char *msg);

private:
    QByteArray readIA5StringData(size_type len);
    uint64_t readObjectIdentifierComponent(size_type &numBytes);

#if !defined(KITINERARY_NO_STD_VARIANT)
    template <typename T, std::size_t Index = 0>
    T readChoiceElement(int choiceIdx)
    {
        if (choiceIdx == Index) {
            std::variant_alternative_t<Index, T> value;
            value.decode(*this);
            return value;
        }
        if constexpr (Index + 1 < std::variant_size_v<T>) {
            return readChoiceElement<T, Index + 1>(choiceIdx);
        }
        Q_UNREACHABLE();
    }
#else
    // C++14 fallback: recursively decode choice element into QVariant
    // Base case: single type remaining
    template <typename T>
    QVariant readChoiceElementQV(int choiceIdx, int currentIdx)
    {
        if (choiceIdx == currentIdx) {
            T value;
            value.decode(*this);
            return QVariant::fromValue(value);
        }
        return {};
    }

    // Recursive case: two or more types
    template <typename T, typename T2, typename... Rest>
    QVariant readChoiceElementQV(int choiceIdx, int currentIdx)
    {
        if (choiceIdx == currentIdx) {
            T value;
            value.decode(*this);
            return QVariant::fromValue(value);
        }
        return readChoiceElementQV<T2, Rest...>(choiceIdx, currentIdx + 1);
    }
#endif

    BitVectorView m_data;
    size_type m_idx = {};
    QByteArray m_error;
};

}

#endif // KITINERARY_UPERDECODER_H
