/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dosipasfactory_p.h"

#include "kitinerary_export.h"

using namespace KItinerary;

QVariant DosipasFactory::decodeDataType(QByteArrayView format, const QByteArray &data)
{
    auto fcb = decodeFcb(format, data);
    if (fcb) {
#if __cplusplus >= 201703L
        return std::visit([](auto &&f) { return QVariant::fromValue(f); }, *fcb);
#else
        return fcb->toVariant();
#endif
    }

    // TODO FCD, vendor extensions

    return {};
}

std::optional<Fcb::UicRailTicketData> DosipasFactory::decodeFcb(QByteArrayView format, const QByteArray &data)
{
    if (format == "FCB3") {
        auto fcb = Fcb::v3::UicRailTicketData(data);
        //return fcb.isValid() ? std::optional<Fcb::UicRailTicketData>(fcb) : std::nullopt;
        return std::optional<Fcb::UicRailTicketData>(fcb);
    }
    if (format == "FCB2") {
        auto fcb = Fcb::v2::UicRailTicketData(data);
        return fcb.isValid() ? std::optional<Fcb::UicRailTicketData>(fcb) : std::nullopt;
    }
    if (format == "FCB1") {
        auto fcb = Fcb::v13::UicRailTicketData(data);
        return fcb.isValid() ? std::optional<Fcb::UicRailTicketData>(fcb) : std::nullopt;
    }

    return {};
}
