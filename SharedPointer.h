#pragma once

template<typename T>
class SharedPointerImpl
{
private:
    /// @brief Internal data structure for the shared pointer
    struct InternalData
    {
        /// @brief Default constructor, initializes the data and sets the reference count to 1
        InternalData() : Data(), ReferenceCount(1) {}

        template<typename... Args>
        InternalData(Args&&... args) : Data(std::forward<Args>(args)...), ReferenceCount(1)
        {
        }

        /// @brief Instance of the data
        T Data;

        /// @brief Reference count
        uint32_t ReferenceCount;
    };

    /// @brief initialize the pointer with
    SharedPointerImpl(InternalData* data) : mData(data) {}

public:
    /// --------------------------------------------------------
    /// Constructors & Destructor
    /// --------------------------------------------------------

    /// @brief Create a shared pointer from template arguments.
    template<typename... Args>
    static constexpr SharedPointerImpl<T> CreateSharedPointer(Args&&... args)
    {
        return SharedPointerImpl<T>(new InternalData(std::forward<Args>(args)...));
    }

    /// @brief Create a shared pointer with default constructor of the underlying type
    static constexpr SharedPointerImpl<T> CreateSharedPointer() { return SharedPointerImpl<T>(new InternalData()); }

    /// @breif Default constructor, null pointer
    SharedPointerImpl() : mData(nullptr) {}

    /// @brief Constructor, initializes the pointer and sets the reference count to 1
    SharedPointerImpl(std::nullptr_t) : mData(nullptr) {}

    /// @brief Copy constructor, increments the reference count
    /// @param other the other shared pointer
    SharedPointerImpl(const SharedPointerImpl& other)
    {
        mData = other.mData;
        AddReference();
    }

    /// @brief Move constructor, moves the data from the other shared pointer
    /// @param other the other shared pointer
    SharedPointerImpl(SharedPointerImpl&& other) noexcept
    {
        mData = other.mData;
        other.mData = nullptr;
    }

    /// @brief Destructor, decrements the reference count and deletes the data if the reference count is 0
    ~SharedPointerImpl() { RemoveReference(); }

    /// --------------------------------------------------------
    /// Operators
    /// --------------------------------------------------------

    /// @brief Copy assignment operator, increments the reference count
    /// @param other the other shared pointer
    /// @return reference to this shared pointer
    SharedPointerImpl& operator=(const SharedPointerImpl& other)
    {
        if (this != &other)
        {
            RemoveReference();

            mData = other.mData;

            AddReference();
        }
        return *this;
    }

    /// @brief Move assignment operator, moves the data from the other shared pointer
    /// @param other the other shared pointer
    /// @return reference to this shared pointer
    SharedPointerImpl& operator=(SharedPointerImpl&& other)
    {
        if (this != &other)
        {
            RemoveReference();

            mData = other.mData;
            other.mData = nullptr;
        }
        return *this;
    }

    /// @brief Access operator
    /// @return pointer to the data
    T* operator->() const { return &mData->Data; }

    /// @brief Dereference operator
    /// @return reference to the data
    T& operator*() const { return mData->Data; }

    /// @brief bool operator
    /// @return true if the pointer is not null
    operator bool() { return mData != nullptr; }

    /// @brief Equality operator
    /// @param other the other shared pointer
    /// @return true if the pointers are equal
    bool operator==(const SharedPointerImpl& other) const { return mData == other.mData; }

    bool operator==(std::nullptr_t) const { return mData == nullptr; }

    /// --------------------------------------------------------
    /// Methods
    /// --------------------------------------------------------

    /// @brief Get the reference count
    /// @return the reference count
    T* get() const { return &mData->Data; }

    /// @brief Get the reference count
    /// @return the reference count
    uint32_t use_count() { return mData->ReferenceCount; }

    /// @brief Check if the pointer is unique
    /// @return true if the reference count is 1
    bool unique() { return mData->ReferenceCount == 1; }

    /// @todo maybe implement standard library functions? not sure if necessary though

private:
    inline void AddReference()
    {
        if (mData == nullptr)
            return;
        mData->ReferenceCount++;
    }

    inline void RemoveReference()
    {
        if (mData == nullptr)
            return;
        mData->ReferenceCount--;
        if (mData->ReferenceCount == 0)
            delete mData;
    }

    /// @brief Internal data structure for the shared pointer
    InternalData* mData;
};
