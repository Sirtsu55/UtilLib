#pragma once

#include <utility>
#include <cassert>
#include "SharedPointer.h"

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

    /// @brief Constructs a function from a member function, but with a shared pointer. This stores no reference
    /// to the shared pointer, so it is up to the user to ensure the shared pointer is valid for the lifetime of
    /// the function.
    /// @tparam C Type of the class/struct.
    /// @param instance The shared pointer to the instance of the class.
    /// @param function The member function to bind.
    template<typename C>
    Function(const SharedPointer<C>& instance, FunctionPtrMember<C> function)
        : mMemberFunction(reinterpret_cast<FunctionPtrMember<Instance>>(function)),
          mInstance(reinterpret_cast<Instance*>(instance.get()))
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
        {
            assert(mInstance != nullptr);
            return (mInstance->*mMemberFunction)(std::forward<Args>(args)...);
        }
        else
        {
            assert(mFunction != nullptr);
            return mFunction(std::forward<Args>(args)...);
        }
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

// Code below is based on
// https://www.codeproject.com/Articles/313312/Cplusplus11-Lambda-Storage-Without-libcplusplus
// and code style modified to match the rest of the project

template<typename T>
class LambdaExecutor;

template<typename Out, typename... In>
class LambdaExecutor<Out(In...)>
{
public:
    Out operator()(In... in)
    {
        assert(lambda != nullptr);
        return ExecuteLambda(lambda, in...);
    }

protected:
    LambdaExecutor(void*& lambda) : lambda(lambda) {}

    ~LambdaExecutor() {}

    template<typename T>
    void GenerateExecutor(T const& lambda)
    {
        ExecuteLambda = [](void* lambda, In... arguments) -> Out { return ((T*)lambda)->operator()(arguments...); };
    }

    void ReceiveExecutor(LambdaExecutor<Out(In...)> const& other) { ExecuteLambda = other.ExecuteLambda; }

private:
    void*& lambda;
    Out (*ExecuteLambda)(void*, In...);
};

template<typename... In>
class LambdaExecutor<void(In...)>
{
public:
    void operator()(In... in)
    {
        assert(lambda != nullptr);
        ExecuteLambda(lambda, in...);
    }

protected:
    LambdaExecutor(void*& lambda) : lambda(lambda) {}

    ~LambdaExecutor() {}

    template<typename T>
    void GenerateExecutor(T const& lambda)
    {
        ExecuteLambda = [](void* lambda, In... arguments) { return ((T*)lambda)->operator()(arguments...); };
    }

    void ReceiveExecutor(LambdaExecutor<void(In...)> const& other) { ExecuteLambda = other.ExecuteLambda; }

private:
    void*& lambda;
    void (*ExecuteLambda)(void*, In...);
};

template<typename T>
class LambdaFunction;

template<typename Out, typename... In>
class LambdaFunction<Out(In...)> : public LambdaExecutor<Out(In...)>
{
public:
    LambdaFunction() : LambdaExecutor<Out(In...)>(lambda), lambda(nullptr), DeleteLambda(nullptr), CopyLambda(nullptr)
    {
    }

    LambdaFunction(LambdaFunction<Out(In...)> const& other)
        : LambdaExecutor<Out(In...)>(lambda), lambda(other.CopyLambda ? other.CopyLambda(other.lambda) : nullptr),
          DeleteLambda(other.DeleteLambda), CopyLambda(other.CopyLambda)
    {
        this->ReceiveExecutor(other);
    }

    template<typename T>
    LambdaFunction(T const& lambda) : LambdaExecutor<Out(In...)>(this->lambda), lambda(nullptr)
    {
        Copy(lambda);
    }

    ~LambdaFunction()
    {
        if (DeleteLambda != nullptr)
            DeleteLambda(lambda);
    }

    LambdaFunction<Out(In...)>& operator=(LambdaFunction<Out(In...)> const& other)
    {
        this->lambda = other.CopyLambda ? other.CopyLambda(other.lambda) : nullptr;
        this->ReceiveExecutor(other);
        this->DeleteLambda = other.DeleteLambda;
        this->CopyLambda = other.CopyLambda;
        return *this;
    }

    template<typename T>
    LambdaFunction<Out(In...)>& operator=(T const& lambda)
    {
        Copy(lambda);
        return *this;
    }

    operator bool() { return lambda != nullptr; }

private:
    template<typename T>
    void Copy(T const& lambda)
    {
        if (this->lambda != nullptr)
            this->DeleteLambda(this->lambda);
        this->lambda = new T(lambda);

        this->GenerateExecutor(lambda);

        this->DeleteLambda = [](void* lambda) { delete (T*)lambda; };

        this->CopyLambda = [](void* lambda) -> void* { return lambda ? new T(*(T*)lambda) : nullptr; };
    }

    void* lambda;
    void (*DeleteLambda)(void*);
    void* (*CopyLambda)(void*);
};
