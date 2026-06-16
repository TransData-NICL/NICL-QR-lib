/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fcbextractor_p.h"

#include "variantvisitor_p.h"

#include <KItinerary/ExtractorValidator>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/ProgramMembership>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>

#include <type_traits>

using namespace Qt::Literals;
using namespace KItinerary;

// Helper: visit the UicRailTicketData (QVariant-wrapped in C++14, std::variant in C++17)
template <typename Func>
static auto visitFcb(Func &&func, const Fcb::UicRailTicketData &fcb)
    -> decltype(func(std::declval<Fcb::v3::UicRailTicketData>()))
{
#if __cplusplus >= 201703L
    return std::visit(std::forward<Func>(func), fcb);
#else
    return VariantVisitor<Func>(std::forward<Func>(func)).template visit<FCB_VERSIONED(UicRailTicketData)>(fcb.toVariant());
#endif
}

// Helper: visit the DocumentData ticket (QVariant in C++14, std::variant in C++17)
// For the "full" ticket variant with all document types
#define FCB_TICKET_TYPES \
    FCB_VERSIONED(ReservationData), \
    FCB_VERSIONED(CarCarriageReservationData), \
    FCB_VERSIONED(OpenTicketData), \
    FCB_VERSIONED(PassData), \
    FCB_VERSIONED(VoucherData), \
    FCB_VERSIONED(CustomerCardData), \
    FCB_VERSIONED(CountermarkData), \
    FCB_VERSIONED(ParkingGroundData), \
    FCB_VERSIONED(FIPTicketData), \
    FCB_VERSIONED(StationPassageData), \
    FCB_VERSIONED(ExtensionData), \
    FCB_VERSIONED(DelayConfirmation)

// Helper: visit a ticket variant for types that have tariff/price/reference info
#define FCB_TARIFF_TYPES \
    FCB_VERSIONED(ReservationData), \
    FCB_VERSIONED(OpenTicketData), \
    FCB_VERSIONED(PassData)

// Helper: visit a ticket variant for types that have classCode
#define FCB_CLASS_TYPES \
    FCB_VERSIONED(ReservationData), \
    FCB_VERSIONED(OpenTicketData), \
    FCB_VERSIONED(PassData)

// Helper: visit regional validity values
#define FCB_REGIONAL_TYPES \
    FCB_VERSIONED(TrainLinkType), \
    FCB_VERSIONED(ViaStationType), \
    FCB_VERSIONED(ZoneType), \
    FCB_VERSIONED(LineType), \
    FCB_VERSIONED(PolygoneType)

// --- ticketNameForDocument ---
// We use VariantVisitor to only visit types that have tariffs/infoText
struct TariffNameVisitor {
    QString &result;
    template<typename U> void operator()(U &&doc) const {
        auto n = doc.tariffs.isEmpty() ? QString() : doc.tariffs.at(0).tariffDesc;
        if (!n.isEmpty()) {
            result = n;
            return;
        }
        result = doc.infoText;
    }
};

struct PassDescVisitor {
    QString &result;
    template<typename U> void operator()(U &&doc) const {
        if (!doc.passDescription.isEmpty()) {
            result = doc.passDescription;
        }
    }
};

template <typename TicketVariantT>
static QString ticketNameForDocumentImpl(const TicketVariantT &ticket)
{
    // First try tariff types (ReservationData, OpenTicketData, PassData)
    QString result;
    makeVariantVisitor(TariffNameVisitor{result}).template visit<FCB_TARIFF_TYPES>(ticket);

    if (!result.isEmpty()) {
        return result;
    }

    // Also check PassData for passDescription
    makeVariantVisitor(PassDescVisitor{result}).template visit<FCB_VERSIONED(PassData)>(ticket);

    return result;
}

struct TicketNameVisitor {
    template<typename T> QString operator()(T &&fcb) const {
        for (const auto &doc : fcb.transportDocument) {
            auto n = ticketNameForDocumentImpl(doc.ticket);
            if (!n.isEmpty()) {
                return n;
            }
        }
        return QString();
    }
};

QString FcbExtractor::ticketName(const Fcb::UicRailTicketData &fcb)
{
    return visitFcb(TicketNameVisitor{}, fcb);
}

// --- fcbReference ---
// Only works with types that have referenceIA5/referenceNum
template <typename T>
static QString fcbReferenceImpl(const T &data)
{
    if (!data.referenceIA5.isEmpty()) {
        return QString::fromLatin1(data.referenceIA5);
    }
    if (data.referenceNumIsSet()) {
        return QString::number(data.referenceNum);
    }
    return {};
}

struct FcbRefVisitor {
    QString &result;
    template<typename T> void operator()(T &&doc) const { result = fcbReferenceImpl(doc); }
};

static QString fcbReferenceFromTicket(const QVariant &ticket)
{
    QString result;
    makeVariantVisitor(FcbRefVisitor{result}).template visit<FCB_TARIFF_TYPES>(ticket);
    return result;
}

struct PnrVisitor {
    template<typename T> QString operator()(T &&fcb) const {
        if (!fcb.issuingDetail.issuerPNR.isEmpty()) {
            return QString::fromLatin1(fcb.issuingDetail.issuerPNR);
        }

        for (const auto &doc : fcb.transportDocument) {
            auto pnr = fcbReferenceFromTicket(doc.ticket);
            if (!pnr.isEmpty()) {
                return pnr;
            }
        }

        return QString();
    }
};

QString FcbExtractor::pnr(const Fcb::UicRailTicketData &fcb)
{
    return visitFcb(PnrVisitor{}, fcb);
}

struct ClassCodeVisitor {
    QString &s;
    template<typename U> void operator()(U &&doc) const {
        s = FcbUtil::classCodeToString(doc.classCode);
    }
};

struct SeatingTypeVisitor {
    template<typename T> QString operator()(T &&fcb) const {
        for (const auto &doc : fcb.transportDocument) {
            QString s;
            makeVariantVisitor(ClassCodeVisitor{s}).template visit<FCB_CLASS_TYPES>(doc.ticket);
            if (!s.isEmpty()) {
                return s;
            }
        }
        return QString();
    }
};

QString FcbExtractor::seatingType(const Fcb::UicRailTicketData &fcb)
{
    return visitFcb(SeatingTypeVisitor{}, fcb);
}

static QString formatIssuerId(int num)
{
    auto id = QString::number(num);
    if (id.size() < 4) {
        id.insert(0, QString(4 - id.size(), QLatin1Char('0')));
    }
    return id;
}

struct IssuerIdVisitor {
    template<typename T> QString operator()(T &&fcb) const {
        if (fcb.issuingDetail.issuerNumIsSet()) {
            return formatIssuerId(fcb.issuingDetail.issuerNum);
        }
        if (fcb.issuingDetail.issuerIA5IsSet()) {
            return QString::fromLatin1(fcb.issuingDetail.issuerIA5);
        }
        if (fcb.issuingDetail.securityProviderNumIsSet()) {
            return formatIssuerId(fcb.issuingDetail.securityProviderNum);
        }
        if (fcb.issuingDetail.securityProviderIA5IsSet()) {
            return QString::fromLatin1(fcb.issuingDetail.securityProviderIA5);
        }
        return QString();
    }
};

QString FcbExtractor::issuerId(const Fcb::UicRailTicketData &fcb)
{
    return visitFcb(IssuerIdVisitor{}, fcb);
}

struct IssuerNameVisitor {
    Organization &issuer;
    template<typename T> void operator()(T &&fcb) const {
        if (fcb.issuingDetail.issuerNameIsSet()) {
            issuer.setName(fcb.issuingDetail.issuerName);
        }
    }
};

Organization FcbExtractor::issuer(const Fcb::UicRailTicketData &fcb)
{
    Organization issuer;
    {
        auto id = issuerId(fcb);
        if (!id.isEmpty()) {
            issuer.setIdentifier(QLatin1String("uic:") + id);
        }
    }
    visitFcb(IssuerNameVisitor{issuer}, fcb);
    return issuer;
}

struct PersonVisitor {
    template<typename T> Person operator()(T &&fcb) const {
        Person p;
        if (!fcb.travelerDetailIsSet() || fcb.travelerDetail.traveler.size() != 1) {
            return p;
        }
        const auto traveler = fcb.travelerDetail.traveler.at(0);
        if (traveler.firstNameIsSet() || traveler.secondNameIsSet()) {
            p.setGivenName(QString(traveler.firstName + QLatin1Char(' ') + traveler.secondName).trimmed());
        }
        p.setFamilyName(traveler.lastName);
        return p;
    }
};

Person FcbExtractor::person(const Fcb::UicRailTicketData &fcb)
{
    return visitFcb(PersonVisitor{}, fcb);
}

struct IssuingDtVisitor {
    template<typename T> QDateTime operator()(T &&data) const { return data.issuingDetail.issueingDateTime(); }
};

QDateTime FcbExtractor::issuingDateTime(const Fcb::UicRailTicketData &fcb)
{
    return visitFcb(IssuingDtVisitor{}, fcb);
}

template<typename FcbT>
struct DepDtVisitor {
    QDateTime &dt;
    const FcbT &fcb;
    template<typename U> void operator()(U &&doc) const {
        dt = doc.departureDateTime(fcb.issuingDetail.issueingDateTime());
    }
};

template<typename FcbT>
struct ValidFromInnerVisitor {
    QDateTime &dt;
    const FcbT &fcb;
    template<typename U> void operator()(U &&doc) const {
        dt = doc.validFrom(fcb.issuingDetail.issueingDateTime());
    }
};

struct ValidFromVisitor {
    template<typename T> QDateTime operator()(T &&fcb) const {
        for (const auto &doc : fcb.transportDocument) {
            // Check ReservationData for departureDateTime
            {
                QDateTime dt;
                makeVariantVisitor(DepDtVisitor<T>{dt, fcb}).template visit<FCB_VERSIONED(ReservationData)>(doc.ticket);
                if (dt.isValid()) {
                    return dt;
                }
            }

            // Check OpenTicketData and PassData for validFrom
            {
                QDateTime dt;
                makeVariantVisitor(ValidFromInnerVisitor<T>{dt, fcb}).template visit<FCB_VERSIONED(OpenTicketData), FCB_VERSIONED(PassData)>(doc.ticket);
                if (dt.isValid()) {
                    return dt;
                }
            }
        }
        return QDateTime();
    }
};

QDateTime FcbExtractor::validFrom(const Fcb::UicRailTicketData &fcb)
{
    return visitFcb(ValidFromVisitor{}, fcb);
}

template<typename FcbT>
struct ArrDtVisitor {
    QDateTime &dt;
    const FcbT &fcb;
    template<typename U> void operator()(U &&doc) const {
        dt = doc.arrivalDateTime(fcb.issuingDetail.issueingDateTime());
    }
};

template<typename FcbT>
struct ValidUntilInnerVisitor {
    QDateTime &dt;
    const FcbT &fcb;
    template<typename U> void operator()(U &&doc) const {
        dt = doc.validUntil(fcb.issuingDetail.issueingDateTime());
    }
};

struct ValidUntilVisitor {
    template<typename T> QDateTime operator()(T &&fcb) const {
        for (const auto &doc : fcb.transportDocument) {
            // Check ReservationData for arrivalDateTime
            {
                QDateTime dt;
                makeVariantVisitor(ArrDtVisitor<T>{dt, fcb}).template visit<FCB_VERSIONED(ReservationData)>(doc.ticket);
                if (dt.isValid()) {
                    return dt;
                }
            }

            // Check OpenTicketData and PassData for validUntil
            {
                QDateTime dt;
                makeVariantVisitor(ValidUntilInnerVisitor<T>{dt, fcb}).template visit<FCB_VERSIONED(OpenTicketData), FCB_VERSIONED(PassData)>(doc.ticket);
                if (dt.isValid()) {
                    return dt;
                }
            }
        }
        return QDateTime();
    }
};

QDateTime FcbExtractor::validUntil(const Fcb::UicRailTicketData &fcb)
{
    return visitFcb(ValidUntilVisitor{}, fcb);
}

struct PriceInnerVisitor {
    double fract;
    double &price;
    template<typename U> void operator()(U &&doc) const {
        price = doc.priceIsSet() ? doc.price / fract : NAN;
    }
};

struct PriceVisitor {
    template<typename T> FcbExtractor::PriceData operator()(T &&fcb) const {
        FcbExtractor::PriceData p;
        p.currency = QString::fromUtf8(fcb.issuingDetail.currency);
        const auto fract = std::pow(10, fcb.issuingDetail.currencyFract);
        for (const auto &doc : fcb.transportDocument) {
            double price = NAN;
            makeVariantVisitor(PriceInnerVisitor{fract, price}).template visit<FCB_TARIFF_TYPES>(doc.ticket);
            p.price = price;
            if (!std::isnan(p.price)) {
                continue;
            }
        }
        return p;
    }
};

FcbExtractor::PriceData FcbExtractor::price(const Fcb::UicRailTicketData &fcb)
{
    return visitFcb(PriceVisitor{}, fcb);
}

template <typename CardReferenceTypeT>
static ProgramMembership extractCustomerCard(const CardReferenceTypeT &card)
{
    ProgramMembership p;
    p.setProgramName(card.cardName);
    if (card.cardIdNumIsSet()) {
        p.setMembershipNumber(QString::number(card.cardIdNum));
    } else if (card.cardIdIA5IsSet()) {
        p.setMembershipNumber(QString::fromUtf8(card.cardIdIA5));
    }
    return p;
}

template <typename TariffTypeT>
static ProgramMembership extractCustomerCard(const QList <TariffTypeT> &tariffs)
{
    // TODO what do we do with the (so far theoretical) case of multiple discount cards in use?
    for (const auto &tariff : tariffs) {
        for (const auto &card : tariff.reductionCard) {
            return extractCustomerCard(card);
        }
    }

    return {};
}

struct CurrFractVisitor {
    template<typename T> int operator()(T &&f) const { return f.issuingDetail.currencyFract; }
};

struct CurrCodeVisitor {
    template<typename T> QByteArray operator()(T &&f) const { return f.issuingDetail.currency; }
};

struct ExtractResVisitor {
    const Fcb::UicRailTicketData &fcb;
    QList<QVariant> &result;
    Ticket ticket;
    QDateTime issuingDt;
    template<typename T> void operator()(T &&irt) const {
        Ticket t(ticket);

        TrainTrip trip;
        trip.setProvider(FcbExtractor::issuer(fcb));
        if (trip.provider().identifier().isEmpty() && trip.provider().name().isEmpty()) {
            trip.setProvider(ticket.issuedBy());
        }
        t.setIssuedBy({});

        TrainStation dep;
        FcbExtractor::readDepartureStation(irt, dep);
        trip.setDepartureStation(dep);

        TrainStation arr;
        FcbExtractor::readArrivalStation(irt, arr);
        trip.setArrivalStation(arr);

        trip.setDepartureTime(irt.departureDateTime(issuingDt));
        trip.setArrivalTime(irt.arrivalDateTime(issuingDt));

        if (irt.trainNumIsSet()) {
            trip.setTrainNumber(irt.serviceBrandAbrUTF8 + QLatin1Char(' ') + QString::number(irt.trainNum));
        } else {
            trip.setTrainNumber(irt.serviceBrandAbrUTF8 + QLatin1Char(' ') + QString::fromUtf8(irt.trainIA5));
        }

        Seat s;
        s.setSeatingType(FcbUtil::classCodeToString(irt.classCode));
        if (irt.placesIsSet()) {
            s.setSeatSection(QString::fromUtf8(irt.places.coach));
            QStringList l;
            for (const auto &b : irt.places.placeIA5) {
                l.push_back(QString::fromUtf8(b));
            }
            for (auto i : irt.places.placeNum) {
                l.push_back(QString::number(i));
            }
            s.setSeatNumber(l.join(QLatin1String(", ")));
            // TODO other seat encoding variants
        }
        t.setTicketedSeat(s);

        TrainReservation trainRes;
        trainRes.setReservationNumber(FcbExtractor::pnr(fcb));
        if (trainRes.reservationNumber().isEmpty()) {
            trainRes.setReservationNumber(ticket.ticketNumber());
        }
        t.setTicketNumber(fcbReferenceImpl(irt));
        trainRes.setUnderName(FcbExtractor::person(fcb));
        trainRes.setProgramMembershipUsed(::extractCustomerCard(irt.tariffs));

        if (irt.priceIsSet()) {
            trainRes.setTotalPrice(irt.price / std::pow(10, visitFcb(CurrFractVisitor{}, fcb)));
        }
        trainRes.setPriceCurrency(QString::fromUtf8(visitFcb(CurrCodeVisitor{}, fcb)));

        ExtractorValidator validator;
        validator.setAcceptedTypes<TrainTrip>();
        //if (validator.isValidElement(trip)) {
            trainRes.setReservationFor(trip);
            trainRes.setReservedTicket(t);
            result.push_back(trainRes);
        //}
    }
};

void FcbExtractor::extractReservation(const QVariant &res, const Fcb::UicRailTicketData &fcb, const Ticket &ticket, QList<QVariant> &result)
{
    const auto issuingDt = FcbExtractor::issuingDateTime(fcb);
    makeVariantVisitor(ExtractResVisitor{fcb, result, ticket, issuingDt}).template visit<FCB_VERSIONED(ReservationData)>(res);
}

template<typename CodeTableType>
struct ValidRegionVisitor {
    const TrainTrip &baseTrip;
    CodeTableType stationCodeTable;
    QDateTime issuingDt;
    const TrainReservation &baseRes;
    QList<QVariant> &result;
    bool &found;
    template<typename U> void operator()(U &&trainLink) const {
        TrainTrip trip(baseTrip);

        if (trainLink.fromStationNameUTF8IsSet()) {
            TrainStation dep;
            FcbExtractor::readDepartureStation(trainLink, stationCodeTable, dep);
            trip.setDepartureStation(dep);
        }

        if (trainLink.toStationNameUTF8IsSet()) {
            TrainStation arr;
            FcbExtractor::readArrivalStation(trainLink, stationCodeTable, arr);
            trip.setArrivalStation(arr);
        }

        trip.setDepartureDay({}); // reset explicit value in case of departure after midnight
        trip.setDepartureTime(trainLink.departureDateTime(issuingDt));

        if (trainLink.trainNumIsSet()) {
            trip.setTrainNumber(QString::number(trainLink.trainNum));
        } else {
            trip.setTrainNumber(QString::fromUtf8(trainLink.trainIA5));
        }

        ExtractorValidator validator;
        validator.setAcceptedTypes<TrainTrip>();
        if (validator.isValidElement(trip)) {
            TrainReservation r(baseRes);
            r.setReservationFor(trip);
            result.push_back(r);
            found = true;
        }
    }
};

template <typename T, typename CodeTableType>
static bool extractValidRegion(const T &regionalValidity, CodeTableType stationCodeTable, const QDateTime &issuingDt, const TrainReservation &baseRes, const TrainTrip &baseTrip, QList<QVariant> &result)
{
    bool found = false;
    makeVariantVisitor(ValidRegionVisitor<CodeTableType>{baseTrip, stationCodeTable, issuingDt, baseRes, result, found}).template visit<FCB_VERSIONED(TrainLinkType)>(regionalValidity);
    return found;
}

struct ZoneIdVisitor {
    QVariantList &zoneIds;
    template<typename T> void operator()(T &&zone) const {
        if (!zone.zoneIdIsSet()) {
            return;
        }
        for (auto zoneId : zone.zoneId) {
            zoneIds.push_back(zoneId);
        }
    }
};

template <typename RegionalValidityList>
static QVariantList extractZoneIds(const RegionalValidityList &regionalValidityList)
{
    QVariantList zoneIds;
    for (const auto &regionalValidity : regionalValidityList) {
        makeVariantVisitor(ZoneIdVisitor{zoneIds}).template visit<FCB_VERSIONED(ZoneType)>(regionalValidity.value);
    }
    return zoneIds;
}

struct ExtractOpenTicketVisitor {
    const Fcb::UicRailTicketData &fcb;
    Ticket ticket;
    QList<QVariant> &result;
    QDateTime issuingDt;
    template<typename T> void operator()(T &&nrt) const {
        Seat s;
        s.setSeatingType(FcbUtil::classCodeToString(nrt.classCode));
        Ticket t(ticket);
        t.setTicketedSeat(s);

        // Set ticket name from tariff description or tariff ID
        if (!nrt.tariffs.isEmpty()) {
            if (!nrt.tariffs.at(0).tariffDesc.isEmpty()) {
                t.setName(nrt.tariffs.at(0).tariffDesc);
            } else if (!nrt.tariffs.at(0).tariffIdIA5.isEmpty()) {
                t.setName(QString::fromUtf8(nrt.tariffs.at(0).tariffIdIA5));
            }
        }

        auto zoneIds = extractZoneIds(nrt.validRegion);
        if (nrt.returnIncluded) {
            const auto returnZoneIds = extractZoneIds(nrt.returnDescription.validReturnRegion);
            zoneIds += returnZoneIds;
        }
        if (!zoneIds.isEmpty()) {
            t.setSubjectOf(zoneIds);
        }

        QVariantList travelerIds;
        for (const auto &tariff : nrt.tariffs) {
            if (tariff.traveleridIsSet()) {
                for (auto travelerId : tariff.travelerid) {
                    travelerIds.push_back(travelerId);
                }
            }
        }

        TrainReservation trainRes;
        trainRes.setReservationNumber(FcbExtractor::pnr(fcb));
        if (trainRes.reservationNumber().isEmpty()) {
            trainRes.setReservationNumber(ticket.ticketNumber());
        }
        t.setTicketNumber(fcbReferenceImpl(nrt));
        t.setIssuedBy({});
        trainRes.setReservedTicket(t);

        trainRes.setUnderName(FcbExtractor::person(fcb));
        trainRes.setProgramMembershipUsed(::extractCustomerCard(nrt.tariffs));
        trainRes.setModifiedTime(issuingDt);
        if (!travelerIds.isEmpty()) {
            trainRes.setSubjectOf(travelerIds);
        }

        if (nrt.priceIsSet()) {
            trainRes.setTotalPrice(nrt.price / std::pow(10, visitFcb(CurrFractVisitor{}, fcb)));
        }
        trainRes.setPriceCurrency(QString::fromUtf8(visitFcb(CurrCodeVisitor{}, fcb)));

        TrainTrip baseTrip;
        baseTrip.setProvider(FcbExtractor::issuer(fcb));
        if (baseTrip.provider().name().isEmpty() && baseTrip.provider().identifier().isEmpty()) {
            baseTrip.setProvider(ticket.issuedBy());
        }
        TrainStation dep;
        FcbExtractor::readDepartureStation(nrt, dep);
        baseTrip.setDepartureStation(dep);
        TrainStation arr;
        FcbExtractor::readArrivalStation(nrt, arr);
        baseTrip.setArrivalStation(arr);
        baseTrip.setDepartureDay(nrt.validFrom(issuingDt).date());
        baseTrip.setDepartureTime(nrt.validFrom(issuingDt));
        baseTrip.setArrivalTime(nrt.validUntil(issuingDt));

        ExtractorValidator validator;
        validator.setAcceptedTypes<TrainTrip>();

        // check for TrainLinkType regional validity constrains
        bool trainLinkTypeFound = false;
        for (const auto &regionalValidity : nrt.validRegion) {
            trainLinkTypeFound |= extractValidRegion(regionalValidity.value, nrt.stationCodeTable, issuingDt, trainRes, baseTrip, result);
        }

        if (!trainLinkTypeFound) {
            //if (validator.isValidElement(baseTrip)) {
                trainRes.setReservationFor(baseTrip);
                result.push_back(trainRes);
            //}
        }

        // same for return trips
        if (nrt.returnIncluded) {
            TrainStation retDep;
            FcbExtractor::readDepartureStation(nrt.returnDescription, nrt.stationCodeTable, retDep);
            TrainStation retArr;
            FcbExtractor::readArrivalStation(nrt.returnDescription, nrt.stationCodeTable, retArr);

            TrainTrip retBaseTrip;
            retBaseTrip.setProvider(baseTrip.provider());
            retBaseTrip.setDepartureStation(retDep);
            retBaseTrip.setArrivalStation(retArr);

            bool retTrainLinkTypeFound = false;
            for (const auto &regionalValidity : nrt.returnDescription.validReturnRegion) {
                retTrainLinkTypeFound |= extractValidRegion(regionalValidity.value, nrt.stationCodeTable, issuingDt, trainRes, retBaseTrip, result);
            }

            if (!retTrainLinkTypeFound && validator.isValidElement(retBaseTrip)) {
                trainRes.setReservationFor(retBaseTrip);
                result.push_back(retBaseTrip);
            }
        }
    }
};

void FcbExtractor::extractOpenTicket(const QVariant &res, const Fcb::UicRailTicketData &fcb, const Ticket &ticket, QList<QVariant> &result)
{
    const auto issuingDt = FcbExtractor::issuingDateTime(fcb);
    makeVariantVisitor(ExtractOpenTicketVisitor{fcb, ticket, result, issuingDt}).template visit<FCB_VERSIONED(OpenTicketData)>(res);
}

struct CustCardVisitor {
    const Fcb::UicRailTicketData &fcb;
    QList<QVariant> &result;
    Ticket ticket;
    template<typename T> void operator()(T &&ccd) const {
        ProgramMembership pm;
        if (ccd.cardIdNumIsSet()) {
            pm.setMembershipNumber(QString::number(ccd.cardIdNum));
        } else {
            pm.setMembershipNumber(QString::fromUtf8(ccd.cardIdIA5));
        }
        pm.setProgramName(ccd.cardTypeDescr);
        pm.setMember(FcbExtractor::person(fcb));
        pm.setValidFrom(QDateTime(ccd.validFrom(), QTime(0, 0)));
        pm.setValidUntil(QDateTime(ccd.validUntil(), QTime(0, 0)));
        pm.setToken(ticket.ticketToken());
        result.push_back(pm);
    }
};

void FcbExtractor::extractCustomerCard(const QVariant &ccd, const Fcb::UicRailTicketData &fcb, const Ticket &ticket, QList<QVariant> &result)
{
    makeVariantVisitor(CustCardVisitor{fcb, result, ticket}).template visit<FCB_VERSIONED(CustomerCardData)>(ccd);
}

struct ReadDepStationVisitor {
    TrainStation &station;
    template<typename T> void operator()(T &&data) const {
        FcbExtractor::readDepartureStation(data, station);
    }
};

void FcbExtractor::readDepartureStation(const QVariant &doc, TrainStation &station)
{
    makeVariantVisitor(ReadDepStationVisitor{station}).template visit<FCB_VERSIONED(ReservationData), FCB_VERSIONED(OpenTicketData)>(doc);
}

struct ReadArrStationVisitor {
    TrainStation &station;
    template<typename T> void operator()(T &&data) const {
        FcbExtractor::readArrivalStation(data, station);
    }
};

void FcbExtractor::readArrivalStation(const QVariant &doc, TrainStation &station)
{
    makeVariantVisitor(ReadArrStationVisitor{station}).template visit<FCB_VERSIONED(ReservationData), FCB_VERSIONED(OpenTicketData)>(doc);
}

void FcbExtractor::fixStationCode(TrainStation &station)
{
    // UIC codes in Germany are wildly unreliable, there seem to be different
    // code tables in use by different operators, so we unfortunately have to ignore
    // those entirely
    if (station.identifier().startsWith(QLatin1String("uic:80"))) {
      PostalAddress addr;
      addr.setAddressCountry(QStringLiteral("DE"));
      station.setAddress(addr);
      station.setIdentifier(QString());
    }
}
