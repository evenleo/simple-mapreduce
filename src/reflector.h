/**
 * @brief C++反射器
 */ 

#ifndef _REFLECTOR_H_
#define _REFLECTOR_H_

#include <iostream>
#include <unordered_map>
#include <memory>
#include <functional>

namespace lmr {

class Reflector {
public:
    typedef  std::function<void*()> Func;
    void* CreateObject(const std::string& name)
    {
        if (mapFunction_.count(name))
            return mapFunction_[name]();
        return nullptr;
    }
    void Regist(const std::string& name, Func&& func)
    {
        mapFunction_[name] = func;
    }
    static Reflector& Instance()
    {
        static Reflector instance;
        return instance; 
    }

private:
    Reflector() {}
    std::unordered_map<std::string, Func> mapFunction_;
};

class Register {
public:
    typedef std::function<void*()> Func;
    Register(const std::string& name, Func&& func)
    {
        Reflector::Instance().Regist(name, std::forward<Func>(func));
    }
};

/**
 * 抽象基类不能注册
 */ 
#define REGISTER(class_name) \
Register g_register_##class_name(#class_name, []() { \
    return new class_name(); \
});

}

#endif