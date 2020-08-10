#ifndef LMR_REDUCER_H
#define LMR_REDUCER_H

#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include <map>
#include "register.h"
#include "reduceinput.h"
#include "hash.h"
#include "reflector.h"

namespace lmr {

using namespace std;

class Reducer {
public:
    virtual void init() {}
    virtual void Reduce(const string& key, ReduceInput* reduceInput) = 0;
    virtual ~Reducer(){}
    virtual void combine() {}

    void set_reduceinput(ReduceInput* reduceinput) { reduceinput_ = reduceinput; }
    void set_hashfunc(HashFunction hashfunc) { hashfunc_ = hashfunc; }
    void set_outputfile(const string& outputfile) { outputfile_ = outputfile; }
    void set_nummapper(int num) { num_mapper_ = num; }

    void reducework();
    void output(string key, string value);

private:
    HashFunction hashfunc_ = JSHash;
    int num_mapper_ = 0;
    string outputfile_;
    ofstream out_;
    ReduceInput* reduceinput_ = nullptr;
};

BASE_CLASS_REGISTER(Reducer)


#define REGISTER_REDUCER(reducer_name) \
    CHILD_CLASS_REGISTER(Reducer,reducer_name)

#define CREATE_REDUCER(reducer_name)   \
    CHILD_CLASS_CREATOR(Reducer,reducer_name)

#define REDUCER_CREATE(name) reinterpret_cast<Reducer*>(Reflector::Instance().CreateObject(name))  

}

#endif //LMR_REDUCER_H
