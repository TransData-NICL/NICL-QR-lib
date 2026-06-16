
#ifndef KITINERARY_EXPORT_H
#define KITINERARY_EXPORT_H

#include <QDate>
#include <QDateTime>
#include <QUrl>
#include <QVariant>

// Qt5 compatibility shims for Qt6-only APIs
#include "internal/qt5compat.h"

#ifdef KITINERARY_STATIC_DEFINE
#  define KITINERARY_EXPORT
#  define KITINERARY_NO_EXPORT
#else
#  ifndef KITINERARY_EXPORT
#    ifdef KPim5Itinerary_EXPORTS
        /* We are building this library */
#      define KITINERARY_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define KITINERARY_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef KITINERARY_NO_EXPORT
#    define KITINERARY_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef KITINERARY_DEPRECATED
#  define KITINERARY_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef KITINERARY_DEPRECATED_EXPORT
#  define KITINERARY_DEPRECATED_EXPORT KITINERARY_EXPORT KITINERARY_DEPRECATED
#endif

#ifndef KITINERARY_DEPRECATED_NO_EXPORT
#  define KITINERARY_DEPRECATED_NO_EXPORT KITINERARY_NO_EXPORT KITINERARY_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef KITINERARY_NO_DEPRECATED
#    define KITINERARY_NO_DEPRECATED
#  endif
#endif

#endif /* KITINERARY_EXPORT_H */
