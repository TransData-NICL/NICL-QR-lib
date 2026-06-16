QT = core qml gui qml-private core-private
QT -= widgets

CONFIG += c++17
# Fallback: ensure -std=c++14 even if mkspec set -std=c++11 (overrides by last-wins)
transdata_cortexa9 { QMAKE_CXXFLAGS += -std=c++14 }
mikro_cortexa8 { QMAKE_CXXFLAGS += -std=c++1y }
mikro_melyx { QMAKE_CXXFLAGS += -std=c++20 }

TARGET = KItinerary

TEMPLATE = lib

CONFIG += \
    staticlib \
   #sharedlib \
   object_parallel_to_source

DEFINES += \
   QT_DISABLE_DEPRECATED_BEFORE=0x050F00 \
   KITINERARY_STATIC_DEFINE \
   KITINERARY_NO_STD_VARIANT

# Force-include Qt5 compatibility header in all C++ compilation units
QMAKE_CXXFLAGS += -include $$PWD/internal/qt5compat.h

HEADERS = \
   $$PWD/asn1/berelement.h \
   $$PWD/asn1/bitvectorview.h \
   $$PWD/asn1/uperdecoder.h \
   $$PWD/asn1/uperelement.h \
   $$PWD/datatypes/action.h \
   $$PWD/datatypes/boattrip.h \
   $$PWD/datatypes/brand.h \
   $$PWD/datatypes/bustrip.h \
   $$PWD/datatypes/creativework.h \
   $$PWD/datatypes/datatypes.h \
   $$PWD/datatypes/datatypes_impl.h \
   $$PWD/datatypes/datatypes_p.h \
   $$PWD/datatypes/event.h \
   $$PWD/datatypes/flight.h \
   $$PWD/datatypes/organization.h \
   $$PWD/datatypes/person.h \
   $$PWD/datatypes/place.h \
   $$PWD/datatypes/programmembership.h \
   $$PWD/datatypes/rentalcar.h \
   $$PWD/datatypes/reservation.h \
   $$PWD/datatypes/taxi.h \
   $$PWD/datatypes/ticket.h \
   $$PWD/datatypes/token.h \
   $$PWD/datatypes/traintrip.h \
   $$PWD/datatypes/visit.h \
   $$PWD/engine/abstractextractor.h \
   $$PWD/engine/extractordocumentnode.h \
   $$PWD/engine/extractordocumentnodefactory.h \
   $$PWD/engine/extractordocumentprocessor.h \
   $$PWD/engine/extractorengine.h \
   $$PWD/engine/extractorfilter.h \
   $$PWD/engine/extractorrepository.h \
   $$PWD/engine/extractorresult.h \
   $$PWD/engine/extractorscriptengine_p.h \
   $$PWD/engine/scriptextractor.h \
   $$PWD/era/dosipas1.h \
   $$PWD/era/dosipas2.h \
   $$PWD/era/dosipasfactory_p.h \
   $$PWD/era/elbticket.h \
   $$PWD/era/fcbextractor_p.h \
   $$PWD/era/fcbreader_p.h \
   $$PWD/era/fcbticket.h \
   $$PWD/era/fcbticket1.h \
   $$PWD/era/fcbticket2.h \
   $$PWD/era/fcbticket3.h \
   $$PWD/era/fcbutil.h \
   $$PWD/era/ssbticketbase.h \
   $$PWD/era/ssbticketreader.h \
   $$PWD/era/ssbv1ticket.h \
   $$PWD/era/ssbv2ticket.h \
   $$PWD/era/ssbv3ticket.h \
   # $$PWD/extractors/activitypubextractor.h \
   # $$PWD/extractors/genericboardingpassextractor.h \
   # $$PWD/http/httpresponse.h \
   # $$PWD/iata/iatabcbp.h \
   # $$PWD/iata/iatabcbpconstants_p.h \
   # $$PWD/iata/iatabcbpparser.h \
   # $$PWD/iata/iatabcbpsections.h \
   $$PWD/internal/instance_counter.h \
   $$PWD/internal/parameter_type.h \
   $$PWD/internal/strict_equal.h \
   $$PWD/internal/strict_less.h \
   $$PWD/jsapi/barcode.h \
   $$PWD/jsapi/bitarray.h \
   $$PWD/jsapi/bytearray.h \
   $$PWD/jsapi/extractorengine.h \
   $$PWD/jsapi/jsonld.h \
   $$PWD/json/jsonld.h \
   $$PWD/json/jsonldfilterengine.h \
   $$PWD/json/jsonldimportfilter.h \
   # $$PWD/knowledgedb/airportdb.h \
   # $$PWD/knowledgedb/airportdb_p.h \
   # $$PWD/knowledgedb/airportnametokenizer_p.h \
   # $$PWD/knowledgedb/alphaid.h \
   # $$PWD/knowledgedb/countrydb.h \
   # $$PWD/knowledgedb/countrydb_p.h \
   # $$PWD/knowledgedb/iatacode.h \
   # $$PWD/knowledgedb/knowledgedb.h \
   # $$PWD/knowledgedb/stationidentifier.h \
   # $$PWD/knowledgedb/timezonedb_p.h \
   # $$PWD/knowledgedb/trainstationdb.h \
   # $$PWD/openssl/bignum_p.h \
   # $$PWD/openssl/opensslpp_p.h \
   # $$PWD/pdf/pdfbarcodeutil_p.h \
   # $$PWD/pdf/pdfdocument.h \
   # $$PWD/pdf/pdfdocument_p.h \
   # $$PWD/pdf/pdfextractoroutputdevice_p.h \
   # $$PWD/pdf/pdfimage.h \
   # $$PWD/pdf/pdfimage_p.h \
   # $$PWD/pdf/pdflink.h \
   # $$PWD/pdf/pdfvectorpicture_p.h \
   # $$PWD/pdf/popplerglobalparams_p.h \
   # $$PWD/pdf/popplertypes_p.h \
   # $$PWD/pdf/popplerutils_p.h \
   # $$PWD/plist/plistdata_p.h \
   # $$PWD/plist/plistreader_p.h \
   $$PWD/processors/barcodedocumentprocessorhelper.h \
   $$PWD/processors/binarydocumentprocessor.h \
   $$PWD/processors/dosipasdocumentprocessor.h \
   $$PWD/processors/eradocumentprocessor.h \
   $$PWD/processors/externalprocessor.h \
   # $$PWD/processors/genericpriceextractorhelper_p.h \
   # $$PWD/processors/htmldocumentprocessor.h \
   # $$PWD/processors/httpresponseprocessor.h \
   # $$PWD/processors/iatabcbpdocumentprocessor.h \
   # $$PWD/processors/icaldocumentprocessor.h \
   # $$PWD/processors/imagedocumentprocessor.h \
   # $$PWD/processors/jsonlddocumentprocessor.h \
   # $$PWD/processors/mimedocumentprocessor.h \
   # $$PWD/processors/pdfdocumentprocessor.h \
   # $$PWD/processors/pkpassdocumentprocessor.h \
   # $$PWD/processors/pkpassesdocumentprocessor.h \
   # $$PWD/processors/plistdocumentprocessor.h \
   # $$PWD/processors/textdocumentprocessor.h \
   $$PWD/processors/uic9183documentprocessor.h \
   # $$PWD/processors/vdvdocumentprocessor.h \
   # $$PWD/protobuf/protobufstreamreader.h \
   # $$PWD/rsp/rsp6decoder.h \
   # $$PWD/text/addressparser_p.h \
   # $$PWD/text/nameoptimizer_p.h \
   $$PWD/text/pricefinder_p.h \
   # $$PWD/text/terminalfinder_p.h \
   # $$PWD/text/timefinder_p.h \
   $$PWD/uic9183/rct2ticket.h \
   $$PWD/uic9183/uic9183block.h \
   $$PWD/uic9183/uic9183flex.h \
   $$PWD/uic9183/uic9183head.h \
   $$PWD/uic9183/uic9183header.h \
   $$PWD/uic9183/uic9183parser.h \
   $$PWD/uic9183/uic9183ticketlayout.h \
   $$PWD/uic9183/uic9183utils.h \
   $$PWD/uic9183/uicstationcode_p.h \
   $$PWD/uic9183/vendor0080block.h \
   $$PWD/uic9183/vendor0080vublockdata.h \
   $$PWD/uic9183/vendor1154block.h \
   # $$PWD/vdv/iso9796_2decoder_p.h \
   # $$PWD/vdv/vdvbasictypes.h \
   # $$PWD/vdv/vdvcertificate_p.h \
   # $$PWD/vdv/vdvdata_p.h \
   # $$PWD/vdv/vdvticket.h \
   # $$PWD/vdv/vdvticketcontent.h \
   # $$PWD/vdv/vdvticketparser.h \
   # $$PWD/barcodedecoder.h \
   # $$PWD/calendarhandler.h \
   # $$PWD/documentutil.h \
   # $$PWD/extractorcapabilities.h \
   # $$PWD/extractorpostprocessor.h \
   # $$PWD/extractorpostprocessor_p.h \
   $$PWD/extractorutil.h \
   $$PWD/extractorvalidator.h \
   # $$PWD/file.h \
   # $$PWD/flightpostprocessor_p.h \
   # $$PWD/flightutil_p.h \
   # $$PWD/htmldocument.h \
   $$PWD/jsonlddocument.h \
   $$PWD/locationutil.h \
   $$PWD/locationutil_p.h \
   $$PWD/mergeutil.h \
   $$PWD/priceutil.h \
   # $$PWD/reservationconverter.h \
   $$PWD/sortutil.h \
   $$PWD/stringutil.h \
   # $$PWD/variantvisitor_p.h \
   # $$PWD/tickettokencomparator_p.h
   $$PWD/logging.h \
   $$PWD/compare-logging.h \
   $$PWD/validator-logging.h \
   $$PWD/kitinerary_export.h

SOURCES = \
   $$PWD/asn1/berelement.cpp \
   $$PWD/asn1/bitvectorview.cpp \
   $$PWD/asn1/uperdecoder.cpp \
   $$PWD/asn1/uperelement.cpp \
   $$PWD/datatypes/action.cpp \
   $$PWD/datatypes/boattrip.cpp \
   $$PWD/datatypes/brand.cpp \
   $$PWD/datatypes/bustrip.cpp \
   $$PWD/datatypes/creativework.cpp \
   $$PWD/datatypes/event.cpp \
   $$PWD/datatypes/flight.cpp \
   $$PWD/datatypes/organization.cpp \
   $$PWD/datatypes/person.cpp \
   $$PWD/datatypes/place.cpp \
   $$PWD/datatypes/programmembership.cpp \
   $$PWD/datatypes/rentalcar.cpp \
   $$PWD/datatypes/reservation.cpp \
   $$PWD/datatypes/taxi.cpp \
   $$PWD/datatypes/ticket.cpp \
   $$PWD/datatypes/token.cpp \
   $$PWD/datatypes/traintrip.cpp \
   $$PWD/datatypes/visit.cpp \
   $$PWD/engine/abstractextractor.cpp \
   $$PWD/engine/extractordocumentnode.cpp \
   $$PWD/engine/extractordocumentnodefactory.cpp \
   $$PWD/engine/extractordocumentprocessor.cpp \
   $$PWD/engine/extractorengine.cpp \
   $$PWD/engine/extractorfilter.cpp \
   $$PWD/engine/extractorrepository.cpp \
   $$PWD/engine/extractorresult.cpp \
   $$PWD/engine/extractorscriptengine.cpp \
   $$PWD/engine/scriptextractor.cpp \
   $$PWD/era/dosipas1.cpp \
   $$PWD/era/dosipas2.cpp \
   $$PWD/era/dosipasfactory.cpp \
   $$PWD/era/elbticket.cpp \
   $$PWD/era/fcbextractor.cpp \
   # $$PWD/era/fcbticket.cpp \
   $$PWD/era/fcbticket1.cpp \
   $$PWD/era/fcbticket2.cpp \
   $$PWD/era/fcbticket3.cpp \
   $$PWD/era/fcbutil.cpp \
   $$PWD/era/ssbticketbase.cpp \
   $$PWD/era/ssbticketreader.cpp \
   $$PWD/era/ssbv1ticket.cpp \
   $$PWD/era/ssbv2ticket.cpp \
   $$PWD/era/ssbv3ticket.cpp \
   # $$PWD/extractors/activitypubextractor.cpp \
   # $$PWD/extractors/genericboardingpassextractor.cpp \
   # $$PWD/http/httpresponse.cpp \
   # $$PWD/iata/iatabcbp.cpp \
   # $$PWD/iata/iatabcbpparser.cpp \
   # $$PWD/iata/iatabcbpsections.cpp \
   $$PWD/jsapi/barcode.cpp \
   $$PWD/jsapi/bitarray.cpp \
   $$PWD/jsapi/bytearray.cpp \
   $$PWD/jsapi/extractorengine.cpp \
   $$PWD/jsapi/jsonld.cpp \
   $$PWD/json/jsonld.cpp \
   $$PWD/json/jsonldfilterengine.cpp \
   $$PWD/json/jsonldimportfilter.cpp \
   # $$PWD/knowledgedb/airportdb.cpp \
   # $$PWD/knowledgedb/airportdb_data.cpp \
   # $$PWD/knowledgedb/airportnametokenizer.cpp \
   # $$PWD/knowledgedb/alphaid.cpp \
   # $$PWD/knowledgedb/countrydb.cpp \
   # $$PWD/knowledgedb/countrydb_data.cpp \
   # $$PWD/knowledgedb/iatacode.cpp \
   # $$PWD/knowledgedb/knowledgedb.cpp \
   # $$PWD/knowledgedb/stationidentifier.cpp \
   # $$PWD/knowledgedb/timezonedb.cpp \
   # $$PWD/knowledgedb/trainstationdb.cpp \
   # $$PWD/knowledgedb/trainstationdb_data.cpp \
   # $$PWD/pdf/pdfbarcodeutil.cpp \
   # $$PWD/pdf/pdfdocument.cpp \
   # $$PWD/pdf/pdfextractoroutputdevice.cpp \
   # $$PWD/pdf/pdfimage.cpp \
   # $$PWD/pdf/pdflink.cpp \
   # $$PWD/pdf/pdfvectorpicture.cpp \
   # $$PWD/pdf/popplerglobalparams.cpp \
   # $$PWD/pdf/popplerutils.cpp \
   # $$PWD/plist/plistreader.cpp \
   $$PWD/processors/barcodedocumentprocessorhelper.cpp \
   $$PWD/processors/binarydocumentprocessor.cpp \
   $$PWD/processors/dosipasdocumentprocessor.cpp \
   $$PWD/processors/eradocumentprocessor.cpp \
   $$PWD/processors/externalprocessor.cpp \
   # $$PWD/processors/genericpriceextractorhelper.cpp \
   # $$PWD/processors/htmldocumentprocessor.cpp \
   # $$PWD/processors/httpresponseprocessor.cpp \
   # $$PWD/processors/iatabcbpdocumentprocessor.cpp \
   # $$PWD/processors/icaldocumentprocessor.cpp \
   # $$PWD/processors/imagedocumentprocessor.cpp \
   # $$PWD/processors/jsonlddocumentprocessor.cpp \
   # $$PWD/processors/mimedocumentprocessor.cpp \
   # $$PWD/processors/pdfdocumentprocessor.cpp \
   # $$PWD/processors/pkpassdocumentprocessor.cpp \
   # $$PWD/processors/pkpassesdocumentprocessor.cpp \
   # $$PWD/processors/plistdocumentprocessor.cpp \
   # $$PWD/processors/textdocumentprocessor.cpp \
   $$PWD/processors/uic9183documentprocessor.cpp \
   # $$PWD/processors/vdvdocumentprocessor.cpp \
   # $$PWD/protobuf/protobufstreamreader.cpp \
   # $$PWD/rsp/rsp6decoder.cpp \
   # $$PWD/text/addressparser.cpp \
   # $$PWD/text/nameoptimizer.cpp \
   $$PWD/text/pricefinder.cpp \
   # $$PWD/text/terminalfinder.cpp \
   # $$PWD/text/timefinder.cpp \
   $$PWD/uic9183/rct2ticket.cpp \
   $$PWD/uic9183/uic9183block.cpp \
   $$PWD/uic9183/uic9183flex.cpp \
   $$PWD/uic9183/uic9183head.cpp \
   $$PWD/uic9183/uic9183header.cpp \
   $$PWD/uic9183/uic9183parser.cpp \
   $$PWD/uic9183/uic9183ticketlayout.cpp \
   $$PWD/uic9183/uic9183utils.cpp \
   $$PWD/uic9183/uicstationcode.cpp \
   $$PWD/uic9183/vendor0080block.cpp \
   $$PWD/uic9183/vendor0080vublockdata.cpp \
   $$PWD/uic9183/vendor1154block.cpp \
   # $$PWD/vdv/certs/cert-downloader.cpp \
   # $$PWD/vdv/iso9796_2decoder.cpp \
   # $$PWD/vdv/vdvcertificate.cpp \
   # $$PWD/vdv/vdvticket.cpp \
   # $$PWD/vdv/vdvticketcontent.cpp \
   # $$PWD/vdv/vdvticketparser.cpp \
   # $$PWD/barcodedecoder.cpp \
   # $$PWD/calendarhandler.cpp \
   # $$PWD/documentutil.cpp \
   # $$PWD/extractorcapabilities.cpp \
   # $$PWD/extractorpostprocessor.cpp \
   $$PWD/extractorutil.cpp \
   $$PWD/extractorvalidator.cpp \
   # $$PWD/file.cpp \
   # $$PWD/flightpostprocessor.cpp \
   # $$PWD/flightutil.cpp \
   # $$PWD/htmldocument.cpp \
   $$PWD/jsonlddocument.cpp \
   $$PWD/locationutil.cpp \
   $$PWD/mergeutil.cpp \
   $$PWD/priceutil.cpp \
   # $$PWD/reservationconverter.cpp \
   $$PWD/sortutil.cpp \
   $$PWD/stringutil.cpp \
   # $$PWD/tickettokencomparator.cpp \
   $$PWD/logging.cpp \
   $$PWD/compare-logging.cpp \
   $$PWD/validator-logging.cpp

# INCLUDEPATH = \
#     $$PWD/. \
#     $$PWD/asn1 \
#     $$PWD/datatypes \
#     $$PWD/engine \
#     $$PWD/era \
#     $$PWD/extractors \
#     $$PWD/http \
#     $$PWD/iata \
#     $$PWD/internal \
#     $$PWD/jsapi \
#     $$PWD/json \
#     $$PWD/kitinerary \
#     $$PWD/knowledgedb \
#     $$PWD/openssl \
#     $$PWD/pdf \
#     $$PWD/plist \
#     $$PWD/processors \
#     $$PWD/protobuf \
#     $$PWD/rsp \
#     $$PWD/text \
#     $$PWD/uic9183 \
#     $$PWD/vdv

INCLUDEPATH += \
    $$PWD \
    $$PWD/.. \
    $$PWD/UIC9183 \
    $$PWD/datatypes \
    $$PWD/internal \
    /usr/include/KF5/KI18n \
    /usr/include/KF5/KMime

# Force moc generation for headers using KITINERARY_GADGET macro
# (qmake moc scanner only detects literal Q_GADGET/Q_OBJECT tokens)
# We generate moc_*.cpp into $$OUT_PWD and add them as SOURCES so they compile
# as separate .o files. The #include "moc_*.cpp" in the datatypes .cpp files
# must be commented out to avoid duplicate symbols.
defineReplace(addMocTarget) {
    name = $$1
    header = $$2
   mocfile = moc_$${name}.cpp
   mocinc = $$join(INCLUDEPATH, ' -I', -I, )
   mocbin = $$[QT_HOST_BINS]/moc
   !exists($$mocbin) {
       mocbin = $$[QT_HOST_LIBEXECS]/moc
   }
   mikro_melyx:!exists($$mocbin) {
       mocbin = /usr/toolchain/mikro_melyx/sysroots/x86_64-melyxsdk-linux/usr/libexec/moc
   }
   # Qt >= 5.8 moc supports --include; use moc_predefs.h to match compiler __cplusplus
   greaterThan(QT_MINOR_VERSION, 7)|greaterThan(QT_MAJOR_VERSION, 5) {
       mocpredefs = --include $$OUT_PWD/moc_predefs.h
       mocdeps = $$header $$OUT_PWD/moc_predefs.h
   } else {
       mocpredefs =
       mocdeps = $$header
   }
    $${name}.target = $$mocfile
   $${name}.depends = $$mocdeps
    $${name}.commands = $$mocbin $(DEFINES) $$mocpredefs $$mocinc $$header -o $$mocfile
    export($${name}.target)
    export($${name}.depends)
    export($${name}.commands)
    SOURCES += $$mocfile
    export(SOURCES)
    return($$name)
}

QMAKE_EXTRA_TARGETS += \
    $$addMocTarget(action,       $$PWD/datatypes/action.h) \
    $$addMocTarget(boattrip,     $$PWD/datatypes/boattrip.h) \
    $$addMocTarget(brand,        $$PWD/datatypes/brand.h) \
    $$addMocTarget(bustrip,      $$PWD/datatypes/bustrip.h) \
    $$addMocTarget(creativework, $$PWD/datatypes/creativework.h) \
    $$addMocTarget(event,        $$PWD/datatypes/event.h) \
    $$addMocTarget(flight,       $$PWD/datatypes/flight.h) \
    $$addMocTarget(organization, $$PWD/datatypes/organization.h) \
    $$addMocTarget(person,       $$PWD/datatypes/person.h) \
    $$addMocTarget(place,        $$PWD/datatypes/place.h) \
    $$addMocTarget(programmembership, $$PWD/datatypes/programmembership.h) \
    $$addMocTarget(rentalcar,    $$PWD/datatypes/rentalcar.h) \
    $$addMocTarget(reservation,  $$PWD/datatypes/reservation.h) \
    $$addMocTarget(taxi,         $$PWD/datatypes/taxi.h) \
    $$addMocTarget(ticket,       $$PWD/datatypes/ticket.h) \
    $$addMocTarget(traintrip,    $$PWD/datatypes/traintrip.h) \
    $$addMocTarget(visit,        $$PWD/datatypes/visit.h) \
    $$addMocTarget(fcbticket1,    $$PWD/era/fcbticket1.h) \
    $$addMocTarget(fcbticket2,    $$PWD/era/fcbticket2.h) \
    $$addMocTarget(fcbticket3,    $$PWD/era/fcbticket3.h) \
    $$addMocTarget(dosipas1,      $$PWD/era/dosipas1.h) \
    $$addMocTarget(dosipas2,      $$PWD/era/dosipas2.h)
