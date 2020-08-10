#ifndef LMR_MAPREDUCE_H
#define LMR_MAPREDUCE_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <chrono>
#include <termios.h>
#include "libgen.h"
#include "libssh/libssh.h"
#include "unistd.h"
#include "spec.h"
#include "netcomm.h"
#include "netpacket.h"
#include "mapper.h"
#include "reducer.h"

namespace lmr
{
    using namespace std;
    using namespace std::chrono;

    typedef struct
    {
        double timeelapsed;
    } MapReduceResult;

    class MapReduce
    {
        friend void cb(header* h, char* data, netcomm* net);
    public:
        MapReduce(MapReduceSpecification* spec = nullptr);
        ~MapReduce();
        void set_spec(MapReduceSpecification* spec);
        int work(MapReduceResult& result);
        MapReduceSpecification* get_spec() { return spec_; }

    private:
        bool dist_run_files();
        void start_work();
        inline int net_mapper_index(int i);
        inline int net_reducer_index(int i);
        inline int mapper_net_index(int i);
        inline int reducer_net_index(int i);

        void assign_mapper(const string& output_format, const vector<int>& input_index);
        void mapper_done(int net_index, const vector<int>& finished_index);
        void assign_reducer(const string& input_format);
        void reducer_done(int net_index);

        bool stopflag_ = false, isready_ = false, firstrun_ = true, firstspec_ = true;
        int index_, total_, mapper_finished_cnt_ = 0, reducer_finished_cnt_ = 0;
        MapReduceSpecification* spec_ = nullptr;
        time_point<chrono::high_resolution_clock> time_cnt_;
        netcomm *net_ = nullptr;
        queue<int> jobs_;
    };
}

#endif //LMR_MAPREDUCE_H
