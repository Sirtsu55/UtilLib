#pragma once

template<typename T>
class SharedPointer
{
public:
    /// @brief Internal data structure for the shared pointer.
    struct InternalData
    {
        /// @brief Default constructor, initializes the data and sets the reference count to 1.
        InternalData() : Data(), ReferenceCount(1) {}

        template<typename... Args>
        InternalData(Args&&... args) : Data(std::forward<Args>(args)...), ReferenceCount(1)
        {
        }

        /// @brief Reference count.
        uint32_t ReferenceCount;

        /// @brief Instance of the data.
        T Data;
    };

public:
    /// --------------------------------------------------------
    /// Constructors & Destructor
    /// --------------------------------------------------------

    /// @breif Default constructor, null pointer.
    SharedPointer() : mData(nullptr) {}

    /// @brief Copy constructor, increments the reference count.
    /// @param other the other shared pointer.
    SharedPointer(const SharedPointer& other)
    {
        mData = other.mData;
        AddReference();
    }

    /// @brief Move constructor, moves the data from the other shared pointer.
    /// @param other the other shared pointer.
    SharedPointer(SharedPointer&& other) noexcept
    {
        mData = other.mData;
        other.mData = nullptr;
    }

    /// @brief Constructor with nullptr.
    SharedPointer(std::nullptr_t) : mData(nullptr) {}

    /// @brief Destructor, decrements the reference count and deletes the data if the reference count is 0.
    ~SharedPointer() { RemoveReference(); }

    /// --------------------------------------------------------
    /// Operators
    /// --------------------------------------------------------

    /// @brief Copy assignment operator, increments the reference count.
    /// @param other the other shared pointer.
    /// @return reference to this shared pointer.
    SharedPointer& operator=(const SharedPointer& other)
    {
        if (this != &other)
        {
            RemoveReference();

            mData = other.mData;

            AddReference();
        }
        return *this;
    }

    /// @brief Move assignment operator, moves the data from the other shared pointer.
    /// @param other the other shared pointer.
    /// @return reference to this shared pointer.
    SharedPointer& operator=(SharedPointer&& other)
    {
        if (this != &other)
        {
            RemoveReference();

            mData = other.mData;
            other.mData = nullptr;
        }
        return *this;
    }

    /// @brief Copy assignment operator with downcast, increments the reference count.
    /// @param other the other shared pointer.
    /// @return reference to this shared pointer.
    template<typename U>
    SharedPointer& operator=(const SharedPointer<U>& other)
    {
        // Check if the pointers are the same
        if (this != &other)
        {
            RemoveReference();

            mData = other.mData;

            AddReference();
        }
        return *this;
    }

    /// @brief Assignment operator, sets the pointer to null.
    SharedPointer& operator=(std::nullptr_t)
    {
        RemoveReference();
        mData = nullptr;
        return *this;
    }

    /// @brief Access operator.
    /// @return pointer to the data.
    T* operator->() const { return &mData->Data; }

    /// @brief Dereference operator.
    /// @return reference to the data.
    T& operator*() const { return mData->Data; }

    /// @brief bool operator.
    /// @return true if the pointer is not null.
    operator bool() { return mData != nullptr; }

    /// @brief Equality operator.
    /// @param other the other shared pointer.
    /// @return true if the pointers are equal.
    bool operator==(const SharedPointer& other) const { return mData == other.mData; }

    bool operator==(std::nullptr_t) const { return mData == nullptr; }

    /// --------------------------------------------------------
    /// Methods
    /// --------------------------------------------------------

    /// @brief Get the reference count.
    /// @return the reference count.
    T* get() const { return &mData->Data; }

    /// @brief Get the reference count.
    /// @return the reference count.
    uint32_t use_count() { return mData->ReferenceCount; }

    /// @brief Check if the pointer is unique.
    /// @return true if the reference count is 1.
    bool unique() { return mData->ReferenceCount == 1; }

    /// @brief Cast the shared pointer to a shared pointer of another type, this checks if the cast is valid and if it
    /// is invalid it returns a null shared pointer.
    /// @tparam U the type to cast to.
    /// @return the down casted shared pointer, or a null shared pointer if the cast is invalid.
    template<typename U>
    SharedPointer<U> dyn_cast()
    {
        U* casted = dynamic_cast<U*>(&mData->Data);

        // Check if the cast is invalid.
        if (casted == nullptr)
        {
            return SharedPointer<U>(nullptr);
        }

        // Add reference to the data, because the new shared pointer will have a reference to it.
        AddReference();

        return SharedPointer<U>(reinterpret_cast<SharedPointer<U>::InternalData*>(mData));
    }

    /// @brief Cast the shared pointer to a shared pointer of another type, this doesn't check if the cast is valid so
    /// in case of an invalid cast it is undefined behavior.
    /// @tparam U the type to cast to.
    /// @return the casted shared pointer.
    template<typename U>
    SharedPointer<U> cast()
    {
        // Add reference to the data, because the new shared pointer will have a reference to it.
        AddReference();
        return SharedPointer<U>(mData);
    }

    /// @brief Initialize the shared pointer with a pointer to the internal data.
    /// @param data the pointer to the internal data.
    SharedPointer(InternalData* data) : mData(data) {}

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

    /// @brief Internal data structure for the shared pointer.
    InternalData* mData;
};

/// @brief Create a shared pointer from template arguments.
/// @tparam T the type of the shared pointer.
/// @tparam Args the arguments to pass to the constructor of the underlying type.
/// @param args the arguments to pass to the constructor of the underlying type.
template<typename T, typename... Args>
static constexpr SharedPointer<T> CreateSharedPointer(Args&&... args)
{
    return SharedPointer<T>(new SharedPointer<T>::InternalData(std::forward<Args>(args)...));
}

/// @brief Create a shared pointer with default constructor of the underlying type
template<typename T>
static constexpr SharedPointer<T> CreateSharedPointer()
{
    return SharedPointer<T>(new SharedPointer<T>::InternalData());
}
