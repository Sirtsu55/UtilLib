#pragma once

#include <cstdint>
#include <string>
#include <cstring>
#include <unordered_map> // For hash functions

class NarrowString;
class WideString;

/// @brief NarrowString class.
class NarrowString
{
public:
    /// @brief Default constructor.
    NarrowString();

    /// @brief Constructor from string.
    /// @param str String.
    NarrowString(const std::string& str);

    /// @brief Constructor from string.
    /// @param str String.
    NarrowString(const char* str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    NarrowString(const std::wstring& str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    NarrowString(const wchar_t* str);

    /// @brief Constructor from WideString.
    /// @param str WideString.
    NarrowString(const WideString& str);

    /// @brief Destructor.
    ~NarrowString();

    /// @brief Copy constructor.
    /// @param other Other string.
    NarrowString(const NarrowString& other);

    /// @brief Move constructor.
    /// @param other Other string.
    NarrowString(NarrowString&& other) noexcept;

    /// @brief Copy assignment operator.
    /// @param other Other string.
    NarrowString& operator=(const NarrowString& other);

    /// @brief Move assignment operator.
    /// @param other Other string.
    NarrowString& operator=(NarrowString&& other) noexcept;

    /// @brief Equality operator.
    /// @param other Other string.
    bool operator==(const NarrowString& other) const;

    /// @brief Array subscript operator.
    /// @param index Index.
    /// @return Character at the index.
    const char& operator[](uint32_t index) const;

    /// @brief Conversion to STL string.
    /// @param str String.
    operator std::string();

    /// @brief Conversion to STL wide string.
    /// @param str Wide string.
    operator std::wstring();

    /// ---------------------
    /// STL string operations
    /// ---------------------

    /// @brief Get the size of the string.
    /// @return Size of the string.
    uint32_t size() const;

    /// @brief Get the string.
    /// @return String.
    const char* c_str() const;

    /// @brief Get the string.
    /// @return String.
    const char* data() const;

private:
    char mStr[32];

    uint32_t mSize = 0;
};

/// @brief WideString class.
class WideString
{
public:
    /// @brief Default constructor.
    WideString();

    /// @brief Constructor from string.
    /// @param str String.
    WideString(const std::string& str);

    /// @brief Constructor from string.
    /// @param str String.
    WideString(const char* str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    WideString(const std::wstring& str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    WideString(const wchar_t* str);

    /// @brief Constructor from NarrowString.
    /// @param str NarrowString.
    WideString(const NarrowString& str);

    /// @brief Destructor.
    ~WideString();

    /// @brief Copy constructor.
    /// @param other Other string.
    WideString(const WideString& other);

    /// @brief Move constructor.
    /// @param other Other string.
    WideString(WideString&& other) noexcept;

    /// @brief Copy assignment operator.
    /// @param other Other string.
    WideString& operator=(const WideString& other);

    /// @brief Move assignment operator.
    /// @param other Other string.
    WideString& operator=(WideString&& other) noexcept;

    /// @brief Equality operator.
    /// @param other Other string.
    bool operator==(const WideString& other) const;

    /// @brief Array subscript operator.
    /// @param index Index.
    /// @return Character at the index.
    const wchar_t& operator[](uint32_t index) const;

    /// @brief Conversion to STL string.
    operator std::string();

    /// @brief Conversion to STL wide string.
    /// @param str Wide string.
    operator std::wstring();

    /// ---------------------
    /// STL string operations
    /// ---------------------

    /// @brief Get the size of the string.
    /// @return Size of the string.
    uint32_t size() const;

    /// @brief Get the string.
    /// @return String.
    const wchar_t* c_str() const;

    /// @brief Get the string.
    /// @return String.
    const wchar_t* data() const;

private:
    wchar_t mStr[32];

    uint32_t mSize = 0;
};

/// ---------------------
/// NarrowString implementation
/// ---------------------

inline NarrowString::NarrowString()
{
    mStr[0] = '\0';
}

inline NarrowString::NarrowString(const std::string& str)
{
    strcpy(mStr, str.c_str());

    mSize = str.size();
}

inline NarrowString::NarrowString(const char* str)
{
    strcpy(mStr, str);

    mSize = strlen(str);
}

inline NarrowString::NarrowString(const std::wstring& str)
{
    wcstombs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline NarrowString::NarrowString(const wchar_t* str)
{
    wcstombs(mStr, str, 32);

    mSize = wcslen(str);
}

inline NarrowString::NarrowString(const WideString& str)
{
    wcstombs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline NarrowString::~NarrowString()
{
}

inline NarrowString::NarrowString(const NarrowString& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline NarrowString::NarrowString(NarrowString&& other) noexcept
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline NarrowString& NarrowString::operator=(const NarrowString& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline NarrowString& NarrowString::operator=(NarrowString&& other) noexcept
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline bool NarrowString::operator==(const NarrowString& other) const
{
    return strcmp(mStr, other.mStr) == 0;
}

inline const char& NarrowString::operator[](uint32_t index) const
{
    return mStr[index];
}

inline uint32_t NarrowString::size() const
{
    return mSize;
}

inline const char* NarrowString::c_str() const
{
    return mStr;
}

inline const char* NarrowString::data() const
{
    return mStr;
}

/// ---------------------
/// WideString implementation
/// ---------------------

inline WideString::WideString()
{
    mStr[0] = L'\0';
}

inline WideString::WideString(const std::string& str)
{
    mbstowcs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline WideString::WideString(const char* str)
{
    mbstowcs(mStr, str, 32);

    mSize = strlen(str);
}

inline WideString::WideString(const std::wstring& str)
{
    wcscpy(mStr, str.c_str());

    mSize = str.size();
}

inline WideString::WideString(const wchar_t* str)
{
    wcscpy(mStr, str);

    mSize = wcslen(str);
}

inline WideString::WideString(const NarrowString& str)
{
    mbstowcs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline WideString::~WideString()
{
}

inline WideString::WideString(const WideString& other)
{
    wcscpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline WideString::WideString(WideString&& other) noexcept
{
    wcscpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline WideString& WideString::operator=(const WideString& other)
{
    wcscpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline WideString& WideString::operator=(WideString&& other) noexcept
{
    wcscpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline bool WideString::operator==(const WideString& other) const
{
    return wcscmp(mStr, other.mStr) == 0;
}

inline const wchar_t& WideString::operator[](uint32_t index) const
{
    return mStr[index];
}

inline uint32_t WideString::size() const
{
    return mSize;
}

inline const wchar_t* WideString::c_str() const
{
    return mStr;
}

inline const wchar_t* WideString::data() const
{
    return mStr;
}

/// ---------------------
/// String conversion to STL strings
/// ---------------------

inline NarrowString::operator std::string()
{
    return std::string(mStr);
}

inline NarrowString::operator std::wstring()
{
    std::wstring wStr = {};
    wStr.resize(32);

    mbstowcs(wStr.data(), mStr, 32);

    return wStr;
}

inline WideString::operator std::string()
{
    std::string str = {};
    str.resize(32);

    wcstombs(str.data(), mStr, 32);

    return str;
}

inline WideString::operator std::wstring()
{
    return std::wstring(mStr);
}

/// ---------------------
/// String hash functions, use string_view instead of string
/// ---------------------

namespace std
{
    template<>
    struct hash<NarrowString>
    {
        size_t operator()(const NarrowString& str) const
        {
            return hash<std::string_view>()(std::string_view(str.c_str(), str.size()));
        }
    };

    template<>
    struct hash<WideString>
    {
        size_t operator()(const WideString& str) const
        {
            return hash<std::wstring_view>()(std::wstring_view(str.c_str(), str.size()));
        }
    };

} // namespace std
