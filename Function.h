#pragma once

#include <utility>

// Add pragma to disable casting pointer to function to another pointer to function
#pragma pointers_to_members(full_generality, virtual_inheritance)

// Code below is based on
// https://codereview.stackexchange.com/questions/277865/tfunction-stdfunction-replacement-for-event-system

template<typename T>
class Function;

template<typename R, typename... Args>
class Function<R(Args...)>
{
public:
    using FunctionPtrStatic = R (*)(Args...);

    template<typename C>
    using FunctionPtrMember = R (C::*)(Args...);

    /// --------------------------------------------------------
    /// Constructors
    /// --------------------------------------------------------

    /// @brief Constructs an empty function.
    Function() : mMemberFunction(nullptr), mFunction(nullptr), mInstance(nullptr) {}

    /// @brief Constructs a function from a static function.
    /// @param function The static function to bind.
    Function(FunctionPtrStatic function) : mMemberFunction(nullptr), mFunction(function), mInstance(nullptr) {}

    /// @brief Constructs a function from a member function.
    /// @tparam C Type of the class/struct.
    /// @param function The member function to bind.
    /// @param instance pointer to the instance of the class.
    template<typename C>
    Function(C* instance, FunctionPtrMember<C> function)
        : mMemberFunction(reinterpret_cast<FunctionPtrMember<Instance>>(function)),
          mInstance(reinterpret_cast<Instance*>(instance))
    {
    }

    /// --------------------------------------------------------
    /// Bind Functions
    /// --------------------------------------------------------

    /// @brief Binds a static function to the function.
    /// @param function The static function to bind.
    constexpr void Bind(FunctionPtrStatic function)
    {
        mFunction = function;
        mInstance = nullptr;
    }

    /// @brief Binds a member function to the function.
    /// @tparam C Type of the class/struct.
    /// @param function The member function to bind.
    /// @param instance pointer to the instance of the class.
    template<typename C>
    constexpr void Bind(C* instance, FunctionPtrMember<C> function)
    {
        mMemberFunction = reinterpret_cast<FunctionPtrMember<Instance>>(function);
        mInstance = reinterpret_cast<Instance*>(instance);
    }

    /// --------------------------------------------------------
    /// Is... Functions
    /// --------------------------------------------------------

    /// @brief Checks if the function is bound to a member function.
    /// @return True if the function is bound to a member function, false otherwise.
    constexpr bool IsMember() const { return mInstance != nullptr; }

    /// @brief Checks if the function is bound to a static function.
    /// @return True if the function is bound to a static function, false otherwise.
    constexpr bool IsStatic() const { return mInstance == nullptr; }

    /// --------------------------------------------------------
    /// Operators
    /// --------------------------------------------------------

    /// @brief Invokes the function.
    /// @param ...args Arguments to pass to the function.
    /// @return Return value of the function.
    constexpr R operator()(Args... args) const
    {
        if (mInstance)
            return (mInstance->*mMemberFunction)(std::forward<Args>(args)...);
        else
            return mFunction(std::forward<Args>(args)...);
    }

    /// @brief Checks if the function is bound to a function.
    constexpr operator bool() const
    {
        // Instance can be null for static functions, so we
        // check for that after checking for function.
        return mFunction != nullptr && mInstance != nullptr;
    }

private:
    struct Instance; // Dummy struct to allow member functions to be invoked

    FunctionPtrMember<Instance> mMemberFunction;
    FunctionPtrStatic mFunction;

    Instance* mInstance;
};
