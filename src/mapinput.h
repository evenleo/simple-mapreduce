#ifndef LMR_MAPINPUT_H
#define LMR_MAPINPUT_H

#include <string>
#include <vector>
#include <fstream>

namespace lmr {
    
using namespace std;
class MapInput
{
public:
    void add_file(string filename) { files_.push_back(filename); }
    bool get_next(string& key, string& value);

private:
    int file_index_ = -1;
    int line_index_ = 0;
    vector<string> files_;
    ifstream f_;
};

}

#endif //LMR_MAPINPUT_H
