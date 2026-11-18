#pragma once

#include <utility>

// Detect effective C++ dialect. MSVC does not update __cplusplus unless
// /Zc:__cplusplus is set, so prefer _MSVC_LANG when available.
#if defined(_MSVC_LANG)
#  define VALIJSON_CPLUSPLUS _MSVC_LANG
#else
#  define VALIJSON_CPLUSPLUS __cplusplus
#endif

// Prefer std::optional under C++17+. Override with -DVALIJSON_USE_STD_OPTIONAL=0/1.
#ifndef VALIJSON_USE_STD_OPTIONAL
#  if VALIJSON_CPLUSPLUS >= 201703L
#    define VALIJSON_USE_STD_OPTIONAL 1
#  else
#    define VALIJSON_USE_STD_OPTIONAL 0
#  endif
#endif

#if VALIJSON_USE_STD_OPTIONAL

#include <optional>

namespace valijson {
namespace internal {

using std::make_optional;
using std::nullopt;
using std::optional;

} // namespace internal
} // namespace valijson

#else

#include <new>
#include <type_traits>

namespace valijson {
namespace internal {

struct nullopt_t
{
    explicit constexpr nullopt_t(int) {}
};

constexpr nullopt_t nullopt{0};

template<typename T>
class optional
{
public:
    optional()
      : m_engaged(false)
    {
    }

    optional(nullopt_t)
      : m_engaged(false)
    {
    }

    optional(const T &value)
      : m_engaged(false)
    {
        construct(value);
    }

    optional(T &&value)
      : m_engaged(false)
    {
        construct(std::move(value));
    }

    optional(const optional &other)
      : m_engaged(false)
    {
        if (other.m_engaged) {
            construct(*other);
        }
    }

    optional(optional &&other)
      : m_engaged(false)
    {
        if (other.m_engaged) {
            construct(std::move(*other));
        }
    }

    ~optional()
    {
        reset();
    }

    optional &operator=(nullopt_t)
    {
        reset();
        return *this;
    }

    optional &operator=(const optional &other)
    {
        if (this == &other) {
            return *this;
        }
        if (other.m_engaged) {
            if (m_engaged) {
                **this = *other;
            } else {
                construct(*other);
            }
        } else {
            reset();
        }
        return *this;
    }

    optional &operator=(optional &&other)
    {
        if (this == &other) {
            return *this;
        }
        if (other.m_engaged) {
            if (m_engaged) {
                **this = std::move(*other);
            } else {
                construct(std::move(*other));
            }
        } else {
            reset();
        }
        return *this;
    }

    optional &operator=(const T &value)
    {
        if (m_engaged) {
            **this = value;
        } else {
            construct(value);
        }
        return *this;
    }

    optional &operator=(T &&value)
    {
        if (m_engaged) {
            **this = std::move(value);
        } else {
            construct(std::move(value));
        }
        return *this;
    }

    bool has_value() const
    {
        return m_engaged;
    }

    explicit operator bool() const
    {
        return m_engaged;
    }

    T &operator*()
    {
        return *pointer();
    }

    const T &operator*() const
    {
        return *pointer();
    }

    T *operator->()
    {
        return pointer();
    }

    const T *operator->() const
    {
        return pointer();
    }

    void reset()
    {
        if (m_engaged) {
            pointer()->~T();
            m_engaged = false;
        }
    }

private:
    void construct(const T &value)
    {
        new (storage()) T(value);
        m_engaged = true;
    }

    void construct(T &&value)
    {
        new (storage()) T(std::move(value));
        m_engaged = true;
    }

    void *storage()
    {
        return static_cast<void *>(&m_storage);
    }

    const void *storage() const
    {
        return static_cast<const void *>(&m_storage);
    }

    T *pointer()
    {
        return static_cast<T *>(storage());
    }

    const T *pointer() const
    {
        return static_cast<const T *>(storage());
    }

    typename std::aligned_storage<sizeof(T), alignof(T)>::type m_storage;
    bool m_engaged;
};

template<typename T>
optional<typename std::decay<T>::type> make_optional(T &&value)
{
    return optional<typename std::decay<T>::type>(std::forward<T>(value));
}

} // namespace internal
} // namespace valijson

#endif
