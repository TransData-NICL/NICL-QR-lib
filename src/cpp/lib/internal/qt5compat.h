/*
    Qt5 compatibility shims for Qt6-only APIs used in KItinerary.
    This header provides minimal polyfills so the codebase compiles with Qt 5.5+.
*/

#ifndef KITINERARY_QT5COMPAT_H
#define KITINERARY_QT5COMPAT_H

// std::string_view polyfill for pre-C++17 compilers (e.g. GCC 5.2 with -std=c++11)
#if __cplusplus < 201703L
#include <cstddef>
#include <cstring>
namespace std {
class string_view {
public:
    typedef std::size_t size_type;
    string_view() : m_data(0), m_size(0) {}
    string_view(const char *data, size_type size) : m_data(data), m_size(size) {}
    string_view(const char *data) : m_data(data), m_size(strlen(data)) {}
    const char *data() const { return m_data; }
    size_type size() const { return m_size; }
    bool empty() const { return m_size == 0; }
    char operator[](size_type i) const { return m_data[i]; }
    char at(size_type i) const { return m_data[i]; }
    const char *begin() const { return m_data; }
    const char *end() const { return m_data + m_size; }
private:
    const char *m_data;
    size_type m_size;
};

// std::optional polyfill for pre-C++17 compilers
struct nullopt_t { explicit nullopt_t() = default; };
static const nullopt_t nullopt{};

template<typename T>
class optional {
public:
    optional() : m_has(false), m_value() {}
    optional(nullopt_t) : m_has(false), m_value() {}
    optional(const T &v) : m_has(true), m_value(v) {}
    optional &operator=(nullopt_t) { m_has = false; return *this; }
    bool has_value() const { return m_has; }
    const T &value() const { return m_value; }
    T &value() { return m_value; }
    explicit operator bool() const { return m_has; }
    const T &operator*() const { return m_value; }
    T &operator*() { return m_value; }
    const T *operator->() const { return &m_value; }
    T *operator->() { return &m_value; }
private:
    bool m_has;
    T m_value;
};

} // namespace std
#define KITINERARY_HAS_STRING_VIEW_POLYFILL 1
#define KITINERARY_HAS_OPTIONAL_POLYFILL 1
#endif

// Guard: skip when Qt headers are not available (e.g. moc_predefs.h generation)
// Use __has_include only if the compiler supports it (GCC 5+, Clang 3+).
// For older compilers (GCC 4.8), fall back to checking QT_CORE_LIB which is
// defined via $(DEFINES) in normal compilation but NOT during moc_predefs.h generation.
#if defined(__has_include)
#  if !__has_include(<QtGlobal>)
#    define KITINERARY_QT5COMPAT_NO_QT_HEADERS
#  endif
#elif !defined(QT_CORE_LIB)
#  define KITINERARY_QT5COMPAT_NO_QT_HEADERS
#endif

#ifndef KITINERARY_QT5COMPAT_NO_QT_HEADERS

#include <QtGlobal>
#include <cmath>
#include <cstring>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

#include <QLatin1Char>
#include <QLatin1String>
#include <QByteArray>
#include <QString>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QStringView>
#endif
#include <QTimeZone>
#include <QVariant>
#include <QVector>
#include <algorithm>
#include <memory>

// std::make_unique polyfill for C++11 (available natively in C++14+)
#if __cplusplus < 201402L
namespace std {
template<typename T, typename... Args>
inline std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
} // namespace std
#endif

// QLatin1StringView is a Qt6 alias
using QLatin1StringView = QLatin1String;

// Fix qGetPtrHelper for std::unique_ptr (Qt 5.5 only knows .data(), not .get())
template<typename T, typename Deleter>
static inline T *qGetPtrHelper(const std::unique_ptr<T, Deleter> &p) { return p.get(); }

// qScopeGuard polyfill for Qt < 5.12
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
template<typename F>
class QScopeGuard {
    F m_func;
    bool m_active;
public:
    explicit QScopeGuard(F &&f) : m_func(std::move(f)), m_active(true) {}
    explicit QScopeGuard(const F &f) : m_func(f), m_active(true) {}
    ~QScopeGuard() { if (m_active) m_func(); }
    void dismiss() { m_active = false; }
    QScopeGuard(const QScopeGuard &) = delete;
    QScopeGuard &operator=(const QScopeGuard &) = delete;
    QScopeGuard(QScopeGuard &&other) : m_func(std::move(other.m_func)), m_active(other.m_active) { other.m_active = false; }
};
template<typename F>
QScopeGuard<typename std::decay<F>::type> qScopeGuard(F &&f) {
    return QScopeGuard<typename std::decay<F>::type>(std::forward<F>(f));
}
#endif

// std::as_const polyfill for C++ < 17
#if __cplusplus < 201703L
namespace std {
template<typename T>
inline const T &as_const(T &t) { return t; }
} // namespace std
#endif

// qsizetype polyfill for Qt < 5.15
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
typedef int qsizetype;
#endif

// Qt::SkipEmptyParts was introduced in Qt 5.14; older Qt uses QString::SkipEmptyParts
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace Qt { static const QString::SplitBehavior SkipEmptyParts = QString::SkipEmptyParts; }
#endif

// Q_NAMESPACE and Q_ENUM_NS polyfills for Qt < 5.8
#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
#ifndef Q_NAMESPACE
#define Q_NAMESPACE
#endif
#ifndef Q_ENUM_NS
#define Q_ENUM_NS(x)
#endif
#ifndef Q_FLAG_NS
#define Q_FLAG_NS(x)
#endif
#endif

// QStringView polyfill for Qt < 5.10 — wraps const QChar* + size
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
class QStringView {
public:
    QStringView() noexcept : m_data(nullptr), m_size(0) {}
    QStringView(const QString &str) noexcept : m_data(str.constData()), m_size(str.size()) {}
    QStringView(const QChar *data, int size) noexcept : m_data(data), m_size(size) {}
    // Support u"..." char16_t string literals (char16_t maps directly to QChar storage)
    QStringView(const char16_t *str, int size) noexcept
        : m_data(reinterpret_cast<const QChar*>(str)), m_size(size) {}
    template<int N>
    QStringView(const char16_t (&str)[N]) noexcept
        : m_data(reinterpret_cast<const QChar*>(str)), m_size(N - 1) {}

    const QChar *data() const noexcept { return m_data; }
    int size() const noexcept { return m_size; }
    bool isEmpty() const noexcept { return m_size == 0; }
    bool empty() const noexcept { return m_size == 0; }
    bool isNull() const noexcept { return m_data == nullptr; }

    const QChar *begin() const noexcept { return m_data; }
    const QChar *end() const noexcept { return m_data + m_size; }


    QChar operator[](int i) const { return m_data[i]; }
    QChar at(int i) const { return m_data[i]; }
    QChar front() const { return m_data[0]; }
    QChar back() const { return m_data[m_size - 1]; }
    QChar last() const { return m_data[m_size - 1]; }

    QStringView mid(int pos, int n = -1) const noexcept {
        if (pos >= m_size) return QStringView();
        if (n < 0 || pos + n > m_size) n = m_size - pos;
        return QStringView(m_data + pos, n);
    }
    QStringView left(int n) const noexcept {
        return QStringView(m_data, qMin(n, m_size));
    }

    QString toString() const { return QString(m_data, m_size); }
    QByteArray toLatin1() const { return toString().toLatin1(); }
    QByteArray toUtf8() const { return toString().toUtf8(); }

    int toInt(bool *ok = nullptr, int base = 10) const {
        return toString().toInt(ok, base);
    }
    ushort toUShort(bool *ok = nullptr, int base = 10) const {
        return toString().toUShort(ok, base);
    }

    QStringView trimmed() const noexcept {
        int start = 0, e = m_size;
        while (start < e && m_data[start].isSpace()) ++start;
        while (e > start && m_data[e - 1].isSpace()) --e;
        return QStringView(m_data + start, e - start);
    }

    int indexOf(QChar ch, int from = 0) const {
        for (int i = from; i < m_size; ++i)
            if (m_data[i] == ch) return i;
        return -1;
    }
    int lastIndexOf(QChar ch, int from = -1) const {
        int start = (from < 0) ? m_size - 1 : from;
        for (int i = start; i >= 0; --i)
            if (m_data[i] == ch) return i;
        return -1;
    }

    int compare(QStringView other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
        return toString().compare(other.toString(), cs);
    }
    bool startsWith(QStringView other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
        return toString().startsWith(other.toString(), cs);
    }
    bool endsWith(QStringView other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
        return toString().endsWith(other.toString(), cs);
    }
    bool startsWith(QLatin1String other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
        return toString().startsWith(other, cs);
    }
    bool endsWith(QLatin1String other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
        return toString().endsWith(other, cs);
    }

    QVector<QStringView> split(QChar sep) const {
        QVector<QStringView> result;
        int start = 0;
        for (int i = 0; i <= m_size; ++i) {
            if (i == m_size || m_data[i] == sep) {
                result.append(QStringView(m_data + start, i - start));
                start = i + 1;
            }
        }
        return result;
    }

    bool operator==(QStringView other) const { return compare(other) == 0; }
    bool operator!=(QStringView other) const { return compare(other) != 0; }
    bool operator<(QStringView other) const { return compare(other) < 0; }
    bool operator==(QLatin1String other) const { return toString() == other; }
    bool operator!=(QLatin1String other) const { return toString() != other; }

private:
    const QChar *m_data;
    int m_size;
};

// Free-standing comparison operators for QString <-> QStringView interop
inline bool operator==(const QString &lhs, QStringView rhs) { return lhs == rhs.toString(); }
inline bool operator!=(const QString &lhs, QStringView rhs) { return lhs != rhs.toString(); }
inline bool operator==(QStringView lhs, const QString &rhs) { return lhs.toString() == rhs; }
inline bool operator!=(QStringView lhs, const QString &rhs) { return lhs.toString() != rhs; }
inline bool operator<(const QString &lhs, QStringView rhs) { return lhs < rhs.toString(); }
inline bool operator<(QStringView lhs, const QString &rhs) { return lhs.toString() < rhs; }

// QDebug streaming for QStringView polyfill
#include <QDebug>
inline QDebug operator<<(QDebug dbg, QStringView sv) { return dbg << sv.toString(); }

#endif // QT_VERSION < 5.10.0

// QByteArrayView polyfill (minimal subset used by KItinerary)
class QByteArrayView {
public:
    constexpr QByteArrayView() noexcept : m_data(nullptr), m_size(0) {}
    constexpr QByteArrayView(const char *data, int size) noexcept : m_data(data), m_size(size) {}
    QByteArrayView(const QByteArray &ba) noexcept : m_data(ba.constData()), m_size(ba.size()) {}

    constexpr const char *data() const noexcept { return m_data; }
    constexpr int size() const noexcept { return m_size; }
    constexpr bool isEmpty() const noexcept { return m_size == 0; }

    constexpr const char *begin() const noexcept { return m_data; }
    constexpr const char *end() const noexcept { return m_data + m_size; }

    int toInt(bool *ok = nullptr, int base = 10) const {
        return QByteArray(m_data, m_size).toInt(ok, base);
    }

    QByteArray toByteArray() const { return QByteArray(m_data, m_size); }

    bool operator==(const QByteArrayView &other) const noexcept {
        if (m_size != other.m_size) return false;
        for (int i = 0; i < m_size; ++i)
            if (m_data[i] != other.m_data[i]) return false;
        return true;
    }

    bool operator==(const char *str) const noexcept {
        if (!str) return m_size == 0;
        const int len = static_cast<int>(strlen(str));
        if (m_size != len) return false;
        return memcmp(m_data, str, len) == 0;
    }
    bool operator!=(const char *str) const noexcept { return !(*this == str); }

    QByteArrayView mid(int pos, int len = -1) const noexcept {
        if (pos >= m_size) return QByteArrayView();
        if (len < 0 || pos + len > m_size) len = m_size - pos;
        return QByteArrayView(m_data + pos, len);
    }
    QByteArrayView first(int n) const noexcept { return QByteArrayView(m_data, qMin(n, m_size)); }
    QByteArrayView sliced(int pos) const noexcept { return mid(pos); }
    QByteArrayView sliced(int pos, int n) const noexcept { return mid(pos, n); }
    constexpr char operator[](int i) const noexcept { return m_data[i]; }

private:
    const char *m_data;
    int m_size;
};

// Qt::Literals::StringLiterals provides the _L1 and _s user-defined literals in Qt6
// Using inline namespace so "using namespace Qt::Literals;" brings in _L1/_s
namespace Qt {
inline namespace Literals {
inline namespace StringLiterals {

inline QLatin1String operator"" _L1(const char *str, std::size_t size) noexcept
{
    return QLatin1String(str, static_cast<int>(size));
}

inline QLatin1Char operator"" _L1(char ch) noexcept
{
    return QLatin1Char(ch);
}

inline QString operator"" _s(const char16_t *str, std::size_t size) noexcept
{
    // Qt 5.5 fromUtf16() only takes const ushort*, not const char16_t*
    return QString::fromUtf16(reinterpret_cast<const ushort*>(str), static_cast<int>(size));
}

} // namespace StringLiterals
} // namespace Literals
} // namespace Qt

// std::ranges polyfill for C++17 — minimal subset used in KItinerary
namespace std {
namespace ranges {
    template<class Container, class Pred>
    inline bool any_of(const Container &c, Pred pred) {
        return std::any_of(std::begin(c), std::end(c), pred);
    }
    template<class Container, class Pred>
    inline bool all_of(const Container &c, Pred pred) {
        return std::all_of(std::begin(c), std::end(c), pred);
    }
    template<class Container, class Pred>
    inline bool none_of(const Container &c, Pred pred) {
        return std::none_of(std::begin(c), std::end(c), pred);
    }
    template<class Container, class Value>
    inline auto find(const Container &c, const Value &val) -> decltype(std::find(std::begin(c), std::end(c), val)) {
        return std::find(std::begin(c), std::end(c), val);
    }
    template<class Container, class Pred>
    inline auto find_if(const Container &c, Pred pred) -> decltype(std::find_if(std::begin(c), std::end(c), pred)) {
        return std::find_if(std::begin(c), std::end(c), pred);
    }
    template<class Container, class Value>
    inline auto count(const Container &c, const Value &val) -> decltype(std::count(std::begin(c), std::end(c), val)) {
        return std::count(std::begin(c), std::end(c), val);
    }
    template<class Container, class OutputIt>
    inline auto copy(const Container &c, OutputIt out) -> decltype(std::copy(std::begin(c), std::end(c), out)) {
        return std::copy(std::begin(c), std::end(c), out);
    }
    template<class Container, class UnaryOp>
    inline auto for_each(const Container &c, UnaryOp op) -> decltype(std::for_each(std::begin(c), std::end(c), op)) {
        return std::for_each(std::begin(c), std::end(c), op);
    }
    template<class Container, class Pred>
    inline auto count_if(const Container &c, Pred pred) -> decltype(std::count_if(std::begin(c), std::end(c), pred)) {
        return std::count_if(std::begin(c), std::end(c), pred);
    }
    template<class Container>
    inline void sort(Container &c) {
        std::sort(std::begin(c), std::end(c));
    }
    template<class Container, class Compare>
    inline void sort(Container &c, Compare comp) {
        std::sort(std::begin(c), std::end(c), comp);
    }
} // namespace ranges
} // namespace std

#endif // QT_VERSION < 6.0.0

#endif // !KITINERARY_QT5COMPAT_NO_QT_HEADERS

#endif // KITINERARY_QT5COMPAT_H
