#ifndef LMR_MAPPER_H
#define LMR_MAPPER_H

#include <string>
#include <vector>
#include <set>
#include <chrono>
#include <fstream>
#include <algorithm>
#include "mapinput.h"
#include "hash.h"
#include "reflector.h"

namespace lmr {

using namespace std;

class Mapper {
public:
    virtual void init() {}
    virtual void Map(const string& key, const string& value) = 0;
    virtual void combine() {}
    virtual ~Mapper() {}

    void set_mapinput(MapInput* mapinput) { mapinput_ = mapinput; }
    void set_hashfunc(HashFunction hashfunc) { hashfunc_ = hashfunc; }
    void set_outputfile(const string& outputfile) { outputfile_ = outputfile; }
    void set_numreducer(int num);

    void mapwork();

protected:
    void emit(string key, string value);

private:
    void output();

    HashFunction hashfunc_ = JSHash;
    int num_reducer_ = 0;
    string outputfile_;
    MapInput* mapinput_ = nullptr;
    vector<multiset<pair<string, string>>> out_;
};

#define MAPPER_CREATE(name) reinterpret_cast<Mapper*>(Reflector::Instance().CreateObject(name))

}

#endif //LMR_MAPPER_H
