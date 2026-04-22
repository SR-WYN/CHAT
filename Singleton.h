#pragma once
#include "global.h"
#include <iostream>

template <typename T>
class Singleton
{
protected:
    // 保护构造：允许子类构造，禁止外部构造
    Singleton() = default;
    virtual ~Singleton()
    {
        std::cout << "this is singleton destruct" << std::endl;
    }

    // 禁止拷贝和赋值
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;

public:
    static T &getInstance()
    {
        // C++11 起，局部静态变量初始化是线程安全的
        static T instance;
        return instance;
    }

    static T *getInstancePtr()
    {
        return &getInstance();
    }

    void printAddress()
    {
        // 直接取 instance 的地址
        std::cout << &getInstance() << std::endl;
    }
};