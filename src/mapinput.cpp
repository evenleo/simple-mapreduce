#include "mapinput.h"

namespace lmr {

bool MapInput::get_next(string& key, string& value)
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

}