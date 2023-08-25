
/// @brief A thread safe vector.
/// @tparam T The type of the vector.
template<typename T>
class TSVector : public Vector<T>
{
public:
    /// --------------------------------------------------------
    /// Constructors & Destructor
    /// --------------------------------------------------------

    /// @brief Construct a new TSVector object
    TSVector() = default;

    /// Deleted Constructors

    TSVector(const TSVector<T>& other) = delete;

    TSVector(TSVector<T>&& other) = delete;

    TSVector<T>& operator=(const TSVector<T>& other) = delete;

    TSVector<T>& operator=(TSVector<T>&& other) = delete;

    /// --------------------------------------------------------
    /// Synchronized functions
    /// --------------------------------------------------------

    /// @brief Start of reading from the vector.
    void start_write() { mWriteLock.lock(); }

    /// @brief End of writing to the vector.
    void end_write() { mWriteLock.unlock(); }

    /// @brief Start of reading from the vector.
    void start_read() { mWriteLock.lock(); }

    /// @brief End of reading from the vector.
    void end_read() { mWriteLock.unlock(); }

private:
    std::mutex mWriteLock;
};
