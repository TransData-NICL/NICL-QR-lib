/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_ADDRESSPARSER_H
#define KITINERARY_ADDRESSPARSER_H

#include "config-kitinerary.h"
#include <KItinerary/Place>

#if HAVE_KCONTACTS
namespace KContacts {
class AddressFormat;
}
#endif

namespace KItinerary {

/** Country-specific address parsing utilities. */
class AddressParser
{
public:
    explicit AddressParser();
    ~AddressParser();

    /** The assumed country when no other country information is known. */
    void setFallbackCountry(const QString &countryCode);

    /** Parse an already partially split address further. */
    void parse(PostalAddress addr);

    PostalAddress result() const;

private:
    void splitPostalCode();
#if HAVE_KCONTACTS
    KContacts::AddressFormat addressFormat() const;
#endif

    PostalAddress m_address;
    QString m_fallbackCountry;
};

}

#endif // KITINERARY_ADDRESSPARSER_H
