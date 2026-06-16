/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDateTime>

#include <cstdint>

/** @file vdvbasictypes.h
 *  Low-level data types used in VDV ticket structs.
 */

namespace KItinerary {

#pragma pack(push)
#pragma pack(1)

/** Two-digit BCD encoded number. */
template <int N>
struct VdvBcdNumber
{
    static_assert(N > 0 && N <= 4, "N must be between 1 and 4");
    uint8_t data[N];

    inline uint32_t value() const
    {
        uint32_t v = 0;
        for (int i = 0; i < N; ++i) {
            v *= 100;
            v += ((data[i] & 0xF0) >> 4) * 10 + (data[i] & 0x0F);
        }
        return v;
    }

    inline operator uint32_t() const { return value(); }
};

/** Date encoded as 8 BCD digits. */
struct VdvBcdDate
{
    VdvBcdNumber<2> bcdYear;
    VdvBcdNumber<1> bcdMonth;
    VdvBcdNumber<1> bcdDay;

    inline QDate value() const
    {
        return QDate(bcdYear, bcdMonth, bcdDay);
    }

    inline operator QDate() const { return value(); }
    inline bool operator==(const QDate &other) const { return value() == other; }
    inline bool operator!=(const QDate &other) const { return value() != other; }

    // dummy assignment operator for compatibility with the Q_PROPERTY system
    inline VdvBcdDate& operator=(const QDate&) { return *this; }
};

/** Big-endian numeric value. */
template <int N>
struct VdvNumber
{
    static_assert(N > 0 && N <= 4, "N must be between 1 and 4");
    uint8_t data[N];

    inline uint32_t value() const
    {
        uint32_t v = 0;
        for (int i = 0; i < N; ++i) {
            v <<= 8;
            v |= data[i];
        }
        return v;
    }

    inline operator uint32_t() const { return value(); }

    // dummy assignment operator for compatibility with the Q_PROPERTY system
    inline VdvNumber<N>& operator=(uint32_t) { return *this; }
};

/** Date/time representation encoded in 4 byte. */
struct VdvDateTimeCompact
{
    VdvNumber<4> data;

    inline QDateTime value() const
    {
        return QDateTime(
            {
                (int)((data & 0b11111110000000000000000000000000u) >> 25) + 1990,
                (int)(data & 0b00000001111000000000000000000000) >> 21,
                (int)(data & 0b00000000000111110000000000000000) >> 16
            }, {
                (int)(data & 0b00000000000000001111100000000000) >> 11,
                (int)(data & 0b00000000000000000000011111100000) >> 5,
                (int)(data & 0b00000000000000000000000000011111) * 2
            });
    }

    inline operator QDateTime() const { return value(); }
    inline bool operator==(const QDateTime &other) const { return value() == other; }
    inline bool operator!=(const QDateTime &other) const { return value() != other; }

    // dummy assignment operator for compatibility with the Q_PROPERTY system
    inline VdvDateTimeCompact& operator=(const QDateTime&) { return *this; }
};

#pragma pack(pop)

}

