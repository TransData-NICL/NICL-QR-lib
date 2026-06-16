/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBTICKET_H
#define KITINERARY_FCBTICKET_H

#include "fcbticket1.h"
#include "fcbticket2.h"
#include "fcbticket3.h"

#if __cplusplus >= 201703L
#include <variant>
#endif

#define FCB_VERSIONED(T) \
    Fcb::v13::T, Fcb::v2::T, Fcb::v3::T

namespace KItinerary {

class UPERDecoder;

namespace Fcb {

#if __cplusplus >= 201703L
using UicRailTicketData = std::variant<FCB_VERSIONED(UicRailTicketData)>;
#else
// C++11 fallback: wraps QVariant since std::variant is unavailable
class UicRailTicketData {
public:
    UicRailTicketData() {}
    template<typename T>
    UicRailTicketData(const T &value) : m_data(QVariant::fromValue(value)) {}
    template<typename T>
    UicRailTicketData &operator=(const T &value) { m_data = QVariant::fromValue(value); return *this; }
    QVariant toVariant() const { return m_data; }
    bool isNull() const { return m_data.isNull(); }
private:
    QVariant m_data;
};
#endif


}
}

#endif
