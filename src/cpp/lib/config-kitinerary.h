/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CONFIG_KITINERARY_H
#define CONFIG_KITINERARY_H

// namespaced by "K" to not interfere with defines poppler provides itself
#define KPOPPLER_VERSION_STRING "26.01.0"
#define KPOPPLER_VERSION_MAJOR 26
#define KPOPPLER_VERSION_MINOR 1
#define KPOPPLER_VERSION_PATCH 0
#define KPOPPLER_VERSION ((26<<16)|(1<<8)|(0))

#define ZXING_VERSION_STRING "2.3.0"
#define ZXING_VERSION_MAJOR 2
#define ZXING_VERSION_MINOR 3
#define ZXING_VERSION_PATCH 0
#define ZXING_VERSION ((2<<16)|(3<<8)|(0))

// QT_VERSION_CHECK isn't available in here for the below check
#define K_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

// this might compile with older versions too, but it only actually works post 1.1.1
#if ZXING_VERSION > K_VERSION_CHECK(1, 1, 1)
    #define ZXING_USE_READBARCODE 1
#else
    #define ZXING_USE_READBARCODE 0
#endif

#define HAVE_LIBXML2 1
#define HAVE_PHONENUMBER 0
#define HAVE_KCONTACTS 0

#define CMAKE_INSTALL_FULL_LIBEXECDIR_KF5 "/usr/lib/x86_64-linux-gnu/libexec/kf5"

#endif // CONFIG_KITINERARY_H
