#include "../../src/reflector.h"

using namespace lmr;

class Base {
public:
    explicit Base() = default;
    virtual void Print()
    {
        std::cout << "Base" << std::endl;
    }
};
REGISTER(Base);  // 注册

class DeriveA : public Base {
public:
    void Print() override
    {
        std::cout << "DeriveA" << std::endl;
    }
};
REGISTER(DeriveA);  // 注册

class DeriveB : public Base {
public:
    void Print() override
    {
        std::cout << "DeriveB" << std::endl;
    }
};
REGISTER(DeriveB);  // 注册

/**
 * 实例创建宏，返回基类指针，相当于new的作用，需要自己回收
 */ 
#define BASE_CREATE(name) reinterpret_cast<Base*>(Reflector::Instance().CreateObject(name))  

int main()
{
    std::shared_ptr<Base> p1(BASE_CREATE("Base"));
    p1->Print();
    std::shared_ptr<Base> p2(BASE_CREATE("DeriveA"));
    p2->Print();
    std::shared_ptr<Base> p3(BASE_CREATE("DeriveB"));
    p3->Print();
    return 0;
}