/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonldfilterengine.h"
#include "jsonld.h"

#include <QJsonArray>
#include <QJsonObject>

#include <cstring>

using namespace KItinerary;

void KItinerary::JsonLd::renameProperty(QJsonObject &obj, const char *oldName, const char *newName)
{
  const auto value = obj.value(QLatin1StringView(oldName));
  if (!value.isUndefined() && !obj.contains(QLatin1StringView(newName))) {
    obj.insert(QLatin1StringView(newName), value);
    obj.remove(QLatin1StringView(oldName));
  }
}

JsonLdFilterEngine::JsonLdFilterEngine() = default;
JsonLdFilterEngine::~JsonLdFilterEngine() = default;

void JsonLdFilterEngine::filterRecursive(QJsonObject &obj)
{
    auto type = JsonLd::typeName(obj).toUtf8();

    // normalize type
    if (m_typeMappings)  {
        const auto it = std::lower_bound(m_typeMappings, m_typeMappings + m_typeMappingsSize, type, [](const TypeMapping &lhs, const QByteArray &rhs) {
            return std::strcmp(lhs.fromType, rhs.constData()) < 0;
        });
        if (it != (m_typeMappings + m_typeMappingsSize) && std::strcmp((*it).fromType, type.constData()) == 0) {
            type = it->toType;
            obj.insert(QStringLiteral("@type"), QLatin1StringView(type));
        }
    }

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if ((*it).type() == QJsonValue::Object) {
            QJsonObject subObj = (*it).toObject();
            filterRecursive(subObj);
            *it = subObj;
        } else if ((*it).type() == QJsonValue::Array) {
            QJsonArray array = (*it).toArray();
            filterRecursive(array);
            *it = array;
        }
    }

    // rename properties
    if (m_propertyMappings) {
        // std::equal_range needs a comparator that handles both directions:
        // PropertyMapping vs QByteArray, and QByteArray vs PropertyMapping
        struct PropertyCmp {
            bool operator()(const JsonLdFilterEngine::PropertyMapping &lhs, const QByteArray &rhs) const {
                return std::strcmp(lhs.type, rhs.constData()) < 0;
            }
            bool operator()(const QByteArray &lhs, const JsonLdFilterEngine::PropertyMapping &rhs) const {
                return std::strcmp(lhs.constData(), rhs.type) < 0;
            }
        };
        const auto range = std::equal_range(m_propertyMappings, m_propertyMappings + m_propertyMappingsSize, type, PropertyCmp{});
        for (auto it = range.first; it != range.second; ++it) {
            JsonLd::renameProperty(obj, (*it).fromName, (*it).toName);
        }
    }

    // apply filter functions
    if (m_typeFilters) {
        const auto filterIt = std::lower_bound(m_typeFilters, m_typeFilters + m_typeFiltersSize, type, [](const TypeFilter &lhs, const QByteArray &rhs) {
            return std::strcmp(lhs.type, rhs.constData()) < 0;
        });
        if (filterIt != (m_typeFilters + m_typeFiltersSize) && std::strcmp((*filterIt).type, type.constData()) == 0) {
            (*filterIt).filterFunc(obj);
        }
    }
}

void JsonLdFilterEngine::filterRecursive(QJsonArray &array)
{
    for (auto it = array.begin(); it != array.end(); ++it) {
        if ((*it).type() == QJsonValue::Object) {
            QJsonObject subObj = (*it).toObject();
            filterRecursive(subObj);
            *it = subObj;
        } else if ((*it).type() == QJsonValue::Array) {
            QJsonArray array = (*it).toArray();
            filterRecursive(array);
            *it = array;
        }
    }
}

void JsonLdFilterEngine::setTypeMappings(const JsonLdFilterEngine::TypeMapping *typeMappings, std::size_t count)
{
    m_typeMappings = typeMappings;
    m_typeMappingsSize = count;
}

void JsonLdFilterEngine::setTypeFilters(const JsonLdFilterEngine::TypeFilter *typeFilters, std::size_t count)
{
    m_typeFilters = typeFilters;
    m_typeFiltersSize = count;
}

void JsonLdFilterEngine::setPropertyMappings(const JsonLdFilterEngine::PropertyMapping *propertyMappings, std::size_t count)
{
    m_propertyMappings = propertyMappings;
    m_propertyMappingsSize = count;
}
