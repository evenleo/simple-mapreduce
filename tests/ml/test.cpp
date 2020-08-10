#include "../../src/common.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    std::string s = "even,evenleo,leo,kobe";
    std::vector<std::string> vec = string_split(s, ',');
    for (auto& i : vec)
    {
        std::cout << i << std::endl;
    }
    
    cout << "-------------" << endl;

    split_file_ascii("input.txt", "lr/input_%d.txt", 3);

    

    return 0;
}