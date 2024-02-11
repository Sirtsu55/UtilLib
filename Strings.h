#pragma once

#include <cstdint>
#include <string>
#include <cstring>
#include <unordered_map> // For hash functions

class CString;
class WString;

/// @brief CString class.
class CString
{
public:
    /// @brief Default constructor.
    CString();

    /// @brief Constructor from string.
    /// @param str String.
    CString(const std::string& str);

    /// @brief Constructor from string.
    /// @param str String.
    CString(const char* str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    CString(const std::wstring& str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    CString(const wchar_t* str);

    /// @brief Constructor from WString.
    /// @param str WString.
    CString(const WString& str);

    /// @brief Destructor.
    ~CString();

    /// @brief Copy constructor.
    /// @param other Other string.
    CString(const CString& other);

    /// @brief Move constructor.
    /// @param other Other string.
    CString(CString&& other);

    /// @brief Copy assignment operator.
    /// @param other Other string.
    CString& operator=(const CString& other);

    /// @brief Move assignment operator.
    /// @param other Other string.
    CString& operator=(CString&& other);

    /// @brief Equality operator.
    /// @param other Other string.
    bool operator==(const CString& other) const;

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

/// @brief WString class.
class WString
{
public:
    /// @brief Default constructor.
    WString();

    /// @brief Constructor from string.
    /// @param str String.
    WString(const std::string& str);

    /// @brief Constructor from string.
    /// @param str String.
    WString(const char* str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    WString(const std::wstring& str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    WString(const wchar_t* str);

    /// @brief Constructor from CString.
    /// @param str CString.
    WString(const CString& str);

    /// @brief Destructor.
    ~WString();

    /// @brief Copy constructor.
    /// @param other Other string.
    WString(const WString& other);

    /// @brief Move constructor.
    /// @param other Other string.
    WString(WString&& other);

    /// @brief Copy assignment operator.
    /// @param other Other string.
    WString& operator=(const WString& other);

    /// @brief Move assignment operator.
    /// @param other Other string.
    WString& operator=(WString&& other);

    /// @brief Equality operator.
    /// @param other Other string.
    bool operator==(const WString& other) const;

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
/// CString implementation
/// ---------------------

inline CString::CString()
{
    mStr[0] = '\0';
}

inline CString::CString(const std::string& str)
{
    strcpy(mStr, str.c_str());

    mSize = str.size();
}

inline CString::CString(const char* str)
{
    strcpy(mStr, str);

    mSize = strlen(str);
}

inline CString::CString(const std::wstring& str)
{
    wcstombs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline CString::CString(const wchar_t* str)
{
    wcstombs(mStr, str, 32);

    mSize = wcslen(str);
}

inline CString::CString(const WString& str)
{
    wcstombs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline CString::~CString()
{
}

inline CString::CString(const CString& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline CString::CString(CString&& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline CString& CString::operator=(const CString& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline CString& CString::operator=(CString&& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline bool CString::operator==(const CString& other) const
{
    return strcmp(mStr, other.mStr) == 0;
}

inline const char& CString::operator[](uint32_t index) const
{
    return mStr[index];
}

inline uint32_t CString::size() const
{
    return mSize;
}

inline const char* CString::c_str() const
{
    return mStr;
}

inline const char* CString::data() const
{
    return mStr;
}

/// ---------------------
/// WString implementation
/// ---------------------

inline WString::WString()
{
    mStr[0] = L'\0';
}

inline WString::WString(const std::string& str)
{
    mbstowcs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline WString::WString(const char* str)
{
    mbstowcs(mStr, str, 32);

    mSize = strlen(str);
}

inline WString::WString(const std::wstring& str)
{
    wcscpy(mStr, str.c_str());

    mSize = str.size();
}

inline WString::WString(const wchar_t* str)
{
    wcscpy(mStr, str);

    mSize = wcslen(str);
}

inline WString::WString(const CString& str)
{
    mbstowcs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline WString::~WString()
{
}

inline WString::WString(const WString& other)
{
    wcscpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline WString::WString(WString&& other)
{
    wcscpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline WString& WString::operator=(const WString& other)
{
    wcscpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline WString& WString::operator=(WString&& other)
{
    wcscpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline bool WString::operator==(const WString& other) const
{
    return wcscmp(mStr, other.mStr) == 0;
}

inline const wchar_t& WString::operator[](uint32_t index) const
{
    return mStr[index];
}

inline uint32_t WString::size() const
{
    return mSize;
}

inline const wchar_t* WString::c_str() const
{
    return mStr;
}

inline const wchar_t* WString::data() const
{
    return mStr;
}

/// ---------------------
/// String conversion to STL strings
/// ---------------------

inline CString::operator std::string()
{
    return std::string(mStr);
}

inline CString::operator std::wstring()
{
    std::wstring wStr = {};
    wStr.resize(32);

    mbstowcs(wStr.data(), mStr, 32);

    return wStr;
}

inline WString::operator std::string()
{
    std::string str = {};
    str.resize(32);

    wcstombs(str.data(), mStr, 32);

    return str;
}

inline WString::operator std::wstring()
{
    return std::wstring(mStr);
}
