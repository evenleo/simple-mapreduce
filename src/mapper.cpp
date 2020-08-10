#include "mapper.h"

namespace lmr {

void Mapper::set_numreducer(int num)
{
    num_reducer_ = num;
    out_.clear();
    out_.resize(num);
}

void Mapper::emit(string key, string value)
{
    if (num_reducer_ == 1)
        out_[0].insert(make_pair(key, value));
    else
        out_[hashfunc_(key) % num_reducer_].insert(make_pair(key, value));
}

void Mapper::mapwork()
{
    auto start = chrono::high_resolution_clock::now();
    string key, value;
    if (!mapinput_)
    {
        fprintf(stderr, "no assigned mapinput_.\n");
        exit(1);
    }
    init();
    while (mapinput_->get_next(key, value))
        Map(key, value);
    combine();

    output();
    fprintf(stderr, "Map Compute Time: %f.\n", chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - start).count());
}

void Mapper::output()
{
    ofstream f;
    char *tmp = new char[10 + outputfile_.size()];
    for (int i = 0; i < num_reducer_; ++i)
    {
        sprintf(tmp, outputfile_.c_str(), i);
        f.open(tmp);
        for (auto &j : out_[i])
        {
            f << j.first.size() << "\t" << j.first << "\t" << j.second << "\n";
        }
        f.close();
    }
    delete[] tmp;
}

}
