/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dosipasdocumentprocessor.h"

#include "variantvisitor_p.h"

#include "era/dosipas1.h"
#include "era/dosipas2.h"
#include "era/fcbextractor_p.h"

#include <KItinerary/ExtractorResult>
#include <KItinerary/Ticket>

using namespace Qt::Literals;
using namespace KItinerary;

namespace {
struct DosipasRawDataVisitor { template<typename T> QByteArray operator()(T &&dosipas) const { return dosipas.rawData(); } };
struct DosipasFcbVisitor {
    template<typename T> std::optional<Fcb::UicRailTicketData> operator()(T &&dosipas) const {
        for (const auto &payload : dosipas.level2SignedData.level1Data.dataSequence) {
            auto f = payload.fcb();
            if (f) { return f; }
        }
        return std::nullopt;
    }
};
struct DosipasFcbExtractVisitor {
    QList<QVariant> &results;
    const std::optional<Fcb::UicRailTicketData> &fcb;
    Ticket ticket;
    template<typename T> void operator()(T &&fcbV) const {
        for (const auto &doc : fcbV.transportDocument) {
            FcbExtractor::extractReservation(doc.ticket, *fcb, ticket, results);
            FcbExtractor::extractOpenTicket(doc.ticket, *fcb, ticket, results);
            FcbExtractor::extractCustomerCard(doc.ticket, *fcb, ticket, results);
        }
    }
};
} // anonymous namespace

bool DosipasDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView /*fileName*/) const
{
    return encodedData.startsWith("\x01Uc") || encodedData.startsWith("\x01Ue")
        || encodedData.startsWith("\x81Uc") || encodedData.startsWith("\x81Ue");
}

ExtractorDocumentNode DosipasDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    {
        auto container = Dosipas::v2::UicBarcodeHeader(encodedData);
        if (container.isValid()) {
            node.setContent(container);
            return node;
        }
    }
    {
        auto container = Dosipas::v1::UicBarcodeHeader(encodedData);
        if (container.isValid()) {
            node.setContent(container);
        }
    }
    return node;
}

void DosipasDocumentProcessor::preExtract(ExtractorDocumentNode &node, const ExtractorEngine * /*engine*/) const
{
    Ticket ticket;
    const auto rawData = makeVariantVisitor(DosipasRawDataVisitor{})
        .template visit<Dosipas::v1::UicBarcodeHeader, Dosipas::v2::UicBarcodeHeader>(node.content());
    ticket.setTicketToken(QLatin1String("aztecbin:") + QString::fromLatin1(rawData.toBase64()));

    const auto fcb = makeVariantVisitor(DosipasFcbVisitor{})
        .template visit<Dosipas::v1::UicBarcodeHeader, Dosipas::v2::UicBarcodeHeader>(node.content());

    if (!fcb) {
        node.addResult(QList<QVariant>({ticket}));
        return;
    }

    QList<QVariant> results;
#if __cplusplus >= 201703L
    std::visit([&results, &fcb, ticket](auto &&fcbV) {
        for (const auto &doc : fcbV.transportDocument) {
#if !defined(KITINERARY_NO_STD_VARIANT)
            const auto docTicket = QVariant::fromStdVariant(doc.ticket);
#else
            const QVariant &docTicket = doc.ticket;
#endif
            FcbExtractor::extractReservation(docTicket, *fcb, ticket, results);
            FcbExtractor::extractOpenTicket(docTicket, *fcb, ticket, results);
            FcbExtractor::extractCustomerCard(docTicket, *fcb, ticket, results);
        }
    }, *fcb);
#else
    makeVariantVisitor(DosipasFcbExtractVisitor{results, fcb, ticket}).template visit<FCB_VERSIONED(UicRailTicketData)>(fcb->toVariant());
#endif

    if (!results.isEmpty()) {
        node.addResult(results);
        return;
    }

    ticket.setName(FcbExtractor::ticketName(*fcb));
    Seat seat;
    seat.setSeatingType(FcbExtractor::seatingType(*fcb));
    ticket.setTicketedSeat(seat);
    ticket.setIssuedBy(FcbExtractor::issuer(*fcb));
    ticket.setTicketNumber(FcbExtractor::pnr(*fcb));
    ticket.setUnderName(FcbExtractor::person(*fcb));
    ticket.setValidFrom(FcbExtractor::validFrom(*fcb));
    ticket.setValidUntil(FcbExtractor::validUntil(*fcb));
    FcbExtractor::applyPrice(ticket, *fcb);

    node.addResult(QList<QVariant>({ticket}));
}
