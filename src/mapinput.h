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
    bool get_next(string& key, string& value)
    {
        while (true)
        {
            while (!f_.good())
            {
                if (++file_index_ >= files_.size())
                    return false;
                f_.close();
                f_.open(files_[file_index_]);
            }

            while (getline(f_, value))
            {
                if (value.back() == '\r')
                    value.pop_back();
                if (!value.empty())
                {
                    key = files_[file_index_] + "_" + to_string(line_index_++);
                    return true;
                }
            }
        }
    }

private:
    int file_index_ = -1;
    int line_index_ = 0;
    vector<string> files_;
    ifstream f_;
};

}

#endif //LMR_MAPINPUT_H
