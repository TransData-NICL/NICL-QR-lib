/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183flex.h"
#include "variantvisitor_p.h"

#ifndef KITINERARY_HAS_OPTIONAL_POLYFILL
#include <optional>
#endif

using namespace KItinerary;

namespace KItinerary {
class Uic9183FlexPrivate : public QSharedData
{
public:
    std::optional<Fcb::UicRailTicketData> m_data;
    Uic9183Block m_block;
};
}

Uic9183Flex::Uic9183Flex() = default;
Uic9183Flex::Uic9183Flex(const Uic9183Block &block)
    : d(new Uic9183FlexPrivate)
{
    if (block.version() == 3) {
        auto fcb = Fcb::v3::UicRailTicketData(block);
        if (fcb.isValid()) {
            d->m_data = Fcb::UicRailTicketData(std::move(fcb));
            d->m_block = block;
            return;
        }
    }
    if (block.version() == 2) {
        auto fcb = Fcb::v2::UicRailTicketData(block);
        if (fcb.isValid()) {
            d->m_data = Fcb::UicRailTicketData(std::move(fcb));
            d->m_block = block;
            return;
        }
    }
    {
        auto fcb = Fcb::v13::UicRailTicketData(block);
        if (fcb.isValid()) {
            d->m_data = Fcb::UicRailTicketData(std::move(fcb));
            d->m_block = block;
        }
    }
}

Uic9183Flex::Uic9183Flex(const Uic9183Flex&) = default;
Uic9183Flex& Uic9183Flex::operator=(const Uic9183Flex&) = default;
Uic9183Flex::~Uic9183Flex() = default;

bool Uic9183Flex::isValid() const
{
    return d && !d->m_block.isNull() && d->m_data.has_value();
}

#if __cplusplus >= 201703L

QDateTime Uic9183Flex::issuingDateTime() const
{
    return isValid() ? std::visit([](auto &&data) { return data.issuingDetail.issueingDateTime(); }, *d->m_data) : QDateTime();
}

bool Uic9183Flex::hasTransportDocument() const
{
    return isValid() && !std::visit([](auto &&data) { return data.transportDocument.empty(); }, *d->m_data);
}

QList<QVariant>Uic9183Flex::transportDocuments() const
{
    return isValid() ? std::visit([](auto &&data) {
        QList<QVariant> l;
        l.reserve(data.transportDocument.size());
        std::transform(data.transportDocument.begin(), data.transportDocument.end(), std::back_inserter(l), [](const auto &doc) {
#if !defined(KITINERARY_NO_STD_VARIANT)
            return QVariant::fromStdVariant(doc.ticket);
#else
            return doc.ticket;
#endif
        });
        return l;
    }, *d->m_data) : QList<QVariant>();
}

QVariant Uic9183Flex::fcbVariant() const
{
    return isValid() ? std::visit([](auto &&data) { return QVariant::fromValue(data); }, *d->m_data) : QVariant();
}

#else // C++14 fallback

struct IssuingDtVisitor { template<typename T> QDateTime operator()(T &&data) const { return data.issuingDetail.issueingDateTime(); } };
struct HasTransportDocVisitor { template<typename T> bool operator()(T &&data) const { return data.transportDocument.empty(); } };
struct DocTicketVisitor { template<typename U> QVariant operator()(U &&doc) const { return doc.ticket; } };
struct TransportDocsVisitor {
    template<typename T> QList<QVariant> operator()(T &&data) const {
        QList<QVariant> l;
        l.reserve(data.transportDocument.size());
        std::transform(data.transportDocument.begin(), data.transportDocument.end(), std::back_inserter(l), DocTicketVisitor{});
        return l;
    }
};
struct FcbVariantVisitor { template<typename T> QVariant operator()(T &&data) const { return QVariant::fromValue(data); } };

QDateTime Uic9183Flex::issuingDateTime() const
{
    if (!isValid()) return QDateTime();
    return makeVariantVisitor(IssuingDtVisitor{})
        .template visit<FCB_VERSIONED(UicRailTicketData)>(d->m_data->toVariant());
}

bool Uic9183Flex::hasTransportDocument() const
{
    if (!isValid()) return false;
    return !makeVariantVisitor(HasTransportDocVisitor{})
        .template visit<FCB_VERSIONED(UicRailTicketData)>(d->m_data->toVariant());
}

QList<QVariant>Uic9183Flex::transportDocuments() const
{
    if (!isValid()) return QList<QVariant>();
    return makeVariantVisitor(TransportDocsVisitor{})
        .template visit<FCB_VERSIONED(UicRailTicketData)>(d->m_data->toVariant());
}

QVariant Uic9183Flex::fcbVariant() const
{
    if (!isValid()) return QVariant();
    return makeVariantVisitor(FcbVariantVisitor{})
        .template visit<FCB_VERSIONED(UicRailTicketData)>(d->m_data->toVariant());
}

#endif

const Fcb::UicRailTicketData& Uic9183Flex::fcb() const
{
    return *d->m_data;
}

#include "moc_uic9183flex.cpp"
