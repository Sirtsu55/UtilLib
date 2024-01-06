#pragma once

template<typename T>
class SharedPointer
{
public:
    /// --------------------------------------------------------
    /// Constructors & Destructor
    /// --------------------------------------------------------

    /// @breif Default constructor, null pointer.
    SharedPointer() : mData(nullptr) {}

    /// @brief Constructor for an existing data pointer. DON'T USE THIS TO CREATE A SHARED POINTER, USE THE
    /// CreateSharedPointer FUNCTION INSTEAD. Does not increment the reference count because it is assumed that the
    /// reference count is handled externally.
    /// @param data the data pointer.
    explicit SharedPointer(uint8_t* data) : mData(data) {}

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

    /// @brief Assignment operator, sets the pointer to null.
    SharedPointer& operator=(std::nullptr_t)
    {
        RemoveReference();
        mData = nullptr;
        return *this;
    }

    /// @brief Implicit conversion operator to a shared pointer of a base class type.
    /// @tparam U the base class type.
    /// @param other the other shared pointer.
    template<typename U>
    operator SharedPointer<U>() const
    {
        static_assert(std::is_base_of<U, T>::value, "U must be a base class of T");

        // Add reference to the data, because the new shared pointer will have a reference to it.
        AddReference();

        return SharedPointer<U>(mData);
    }

    /// @brief Access operator.
    /// @return pointer to the data.
    T* operator->() const { return GetData(); }

    /// @brief Dereference operator.
    /// @return reference to the data.
    T& operator*() const { return *GetData(); }

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
    T* get() const { return GetData(); }

    /// @brief Get the reference count.
    /// @return the reference count.
    uint32_t use_count() { return *GetReferenceData(); }

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
        U* casted = dynamic_cast<U*>(GetData());

        // Check if the cast is invalid.
        if (casted == nullptr)
        {
            return SharedPointer<U>(nullptr);
        }

        // Add reference to the data, because the new shared pointer will have a reference to it.
        AddReference();

        return SharedPointer<U>(mData);
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

private:
    inline T* GetData() const { return reinterpret_cast<T*>(mData + sizeof(uint32_t)); }

    inline uint32_t* GetReferenceData() const { return reinterpret_cast<uint32_t*>(mData); }

    inline void AddReference()
    {
        if (mData == nullptr)
            return;

        uint32_t& referenceCount = *GetReferenceData();

        referenceCount++;
    }

    inline void RemoveReference()
    {
        uint32_t& referenceCount = *GetReferenceData();

        if (mData == nullptr)
            return;

        referenceCount--;

        if (referenceCount == 0)
        {
            GetData()->~T(); // Call the destructor of the data.
            delete mData;
        }
    }

    /// @brief Internal data structure for the shared pointer.
    uint8_t* mData;

    // Friend functions that create shared pointers.
    template<typename U, typename... Args>
    constexpr friend SharedPointer<U> CreateSharedPointer(Args&&... args);

    template<typename U>
    constexpr friend SharedPointer<U> CreateSharedPointer();
};

/// @brief Create a shared pointer from template arguments.
/// @tparam T the type of the shared pointer.
/// @tparam Args the arguments to pass to the constructor of the underlying type.
/// @param args the arguments to pass to the constructor of the underlying type.
template<typename T, typename... Args>
constexpr SharedPointer<T> CreateSharedPointer(Args&&... args)
{
    uint8_t* data = new uint8_t[sizeof(uint32_t) + sizeof(T)]; // Allocate memory for the data and the reference count.

    // First four bytes of the data is the reference count. Set it to 1 because the shared pointer will have a
    // reference to it.
    *reinterpret_cast<uint32_t*>(data) = 1;

    // Construct new object in the allocated memory.
    T* object = new (data + sizeof(uint32_t)) T(std::forward<Args>(args)...);

    return SharedPointer<T>(data);
}

/// @brief Create a shared pointer with default constructor of the underlying type
template<typename T>
constexpr SharedPointer<T> CreateSharedPointer()
{
    uint8_t* data = new uint8_t[sizeof(uint32_t) + sizeof(T)]; // Allocate memory for the data and the reference count.

    // First four bytes of the data is the reference count. Set it to 1 because the shared pointer will have a
    // reference to it.
    *reinterpret_cast<uint32_t*>(data) = 1;

    // Construct new object in the allocated memory.
    T* object = new (data + sizeof(uint32_t)) T();

    return SharedPointer<T>(data);
}
