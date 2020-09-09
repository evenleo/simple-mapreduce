#include "../src/reflector.h"
#include <iostream>

using namespace std;
using namespace lmr;

class A
{
public:
    virtual void print(string s)
    {
        cout << "A: " << s << endl;
    }
};

#define A_CREATE(name) reinterpret_cast<A*>(Reflector::Instance().CreateObject(name))

class B : public A
{
public:
    void print(string s)
    {
        cout << "B: " << s << endl;
    }
};


REGISTER(B)

int main()
{
    A* b = A_CREATE("B");
    b->print("abc");
    delete b;
    return 0;
}
