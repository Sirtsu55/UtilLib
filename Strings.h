#pragma once

#include <cstdint>
#include <string>
#include <cstring>

class NString;
class WideString;

/// @brief NString class.
class NString
{
public:
    /// @brief Default constructor.
    NString();

    /// @brief Constructor from string.
    /// @param str String.
    NString(const std::string& str);

    /// @brief Constructor from string.
    /// @param str String.
    NString(const char* str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    NString(const std::wstring& str);

    /// @brief Constructor from wide string.
    /// @param str Wide string.
    NString(const wchar_t* str);

    /// @brief Constructor from WideString.
    /// @param str WideString.
    NString(const WideString& str);

    /// @brief Destructor.
    ~NString();

    /// @brief Copy constructor.
    /// @param other Other string.
    NString(const NString& other);

    /// @brief Move constructor.
    /// @param other Other string.
    NString(NString&& other);

    /// @brief Copy assignment operator.
    /// @param other Other string.
    NString& operator=(const NString& other);

    /// @brief Move assignment operator.
    /// @param other Other string.
    NString& operator=(NString&& other);

    /// @brief Equality operator.
    /// @param other Other string.
    bool operator==(const NString& other) const;

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

    /// @brief Constructor from NString.
    /// @param str NString.
    WideString(const NString& str);

    /// @brief Destructor.
    ~WideString();

    /// @brief Copy constructor.
    /// @param other Other string.
    WideString(const WideString& other);

    /// @brief Move constructor.
    /// @param other Other string.
    WideString(WideString&& other);

    /// @brief Copy assignment operator.
    /// @param other Other string.
    WideString& operator=(const WideString& other);

    /// @brief Move assignment operator.
    /// @param other Other string.
    WideString& operator=(WideString&& other);

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
/// NString implementation
/// ---------------------

inline NString::NString()
{
    mStr[0] = '\0';
}

inline NString::NString(const std::string& str)
{
    strcpy(mStr, str.c_str());

    mSize = str.size();
}

inline NString::NString(const char* str)
{
    strcpy(mStr, str);

    mSize = strlen(str);
}

inline NString::NString(const std::wstring& str)
{
    wcstombs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline NString::NString(const wchar_t* str)
{
    wcstombs(mStr, str, 32);

    mSize = wcslen(str);
}

inline NString::NString(const WideString& str)
{
    wcstombs(mStr, str.c_str(), 32);

    mSize = str.size();
}

inline NString::~NString()
{
}

inline NString::NString(const NString& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline NString::NString(NString&& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;
}

inline NString& NString::operator=(const NString& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline NString& NString::operator=(NString&& other)
{
    strcpy(mStr, other.mStr);

    mSize = other.mSize;

    return *this;
}

inline bool NString::operator==(const NString& other) const
{
    return strcmp(mStr, other.mStr) == 0;
}

inline const char& NString::operator[](uint32_t index) const
{
    return mStr[index];
}

inline uint32_t NString::size() const
{
    return mSize;
}

inline const char* NString::c_str() const
{
    return mStr;
}

inline const char* NString::data() const
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

inline WideString::WideString(const NString& str)
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

inline WideString::WideString(WideString&& other)
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

inline WideString& WideString::operator=(WideString&& other)
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

inline NString::operator std::string()
{
    return std::string(mStr);
}

inline NString::operator std::wstring()
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
