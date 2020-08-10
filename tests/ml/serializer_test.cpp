#include <iostream>
#include "../../src/serializer.h"

using namespace std;
using namespace even;

struct S {
    S() {}
    S(int i, float f) : a(i), b(f) {}
    int a;
    float b;
    friend ostream &operator<<(ostream &os, const S &s);
};

ostream &operator<<(ostream &os, const S &s)
{
    return os << s.a << ", " << s.b;
}

int main()
{
    Serializer serializer;

    string str = "abcdef";
    int n = 10;
    S s(7, 11.2);

    serializer.input_type(str);
    serializer.input_type(n);
    serializer.input_type(s);

    string got_str;
    int got_n;
    S got_s;
    serializer.output_type(got_str);
    serializer.output_type(got_n);
    serializer.output_type(got_s);
    cout << "got_str: " << got_str << endl;
    cout << "got_n: " << got_n << endl;
    cout << "got_s: " << got_s << endl;
}