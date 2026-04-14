#pragma once

#include "global.h"
#include <memory>

template <typename T>
class Singleton
{
protected:
    Singleton() = default;
    Singleton(const Singleton<T> &) = delete;
    Singleton &operator=(const Singleton<T> &) = delete;
    static std::shared_ptr<T> _instance;
public:
    static T& GetInstance() // magic static
    {
        static T instance;
        return instance;
    }

    void PrintAddress()
    {
        std::cout << _instance().get() << std::endl;
    }

    ~Singleton() 
    {
        std::cout << "this is singleton destruct" << std::endl;
    }

};

//类内声明，类外初始化
template <typename T>
std::shared_ptr <T> Singleton<T>::_instance = nullptr;