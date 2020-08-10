#ifndef LMR_SPEC_H
#define LMR_SPEC_H

#include <string>
#include "mapinput.h"
#include "hash.h"

namespace lmr {

using namespace std;

struct MapReduceSpecification
{
    HashFunction hashfunc = JSHash;
    string input_format, output_format, mapper_class, reducer_class, config_file, program_file;
    int num_inputs = 0, num_mappers = 0, num_reducers = 0, index = 0;
};

}

#endif //LMR_SPEC_H
