#include "mapreduce.h"
#include <memory>
#include <mutex>

namespace lmr {

MapReduce* instance = nullptr;
static std::mutex s_mutex;
int number_checkin = 0, real_total = -1;

void cb(header* h, char* data, netcomm* net_)
{
    string input_format, output_format;
    vector<int> input_index, finished_indices;
    switch (h->type)
    {
        case netcomm_type::LMR_CHECKIN:
            {
                std::lock_guard<std::mutex> lk(s_mutex);
                ++number_checkin;
                if (number_checkin % (real_total - 1) == 0)
                    instance->isready_ = true;
            }
            cout << "===netcomm_type::LMR_CHECKIN" << endl;

            break;
        case netcomm_type::LMR_CLOSE:
            instance->stopflag_ = true;
            break;
        case netcomm_type::LMR_ASSIGN_MAPPER:
            cout << "===netcomm_type::LMR_ASSIGN_MAPPER" << endl;
            parse_assign_mapper(data, output_format, input_index);
            instance->assign_mapper(output_format, input_index);
            break;
        case netcomm_type::LMR_ASSIGN_REDUCER:
            cout << "===netcomm_type::LMR_ASSIGN_REDUCER" << endl;
            parse_assign_reducer(data, input_format);
            instance->assign_reducer(input_format);
            break;
        case netcomm_type::LMR_MAPPER_DONE:
            cout << "===netcomm_type::LMR_MAPPER_DONE" << endl;
            parse_mapper_done(data, finished_indices);
            instance->mapper_done(h->src, finished_indices);
            break;
        case netcomm_type::LMR_REDUCER_DONE:
            cout << "===netcomm_type::LMR_REDUCER_DONE" << endl;
            instance->reducer_done(h->src);
            break;
        default:
            break;
    }
}

inline int MapReduce::net_mapper_index(int i)
{
    return i - 1;
}

inline int MapReduce::net_reducer_index(int i)
{
    return i - 1 - spec_->num_mappers;
}

inline int MapReduce::mapper_net_index(int i)
{
    return i + 1;
}

inline int MapReduce::reducer_net_index(int i)
{
    return i + 1 + spec_->num_mappers;
}

void MapReduce::assign_reducer(const string& input_format)
{
    char tmp[1024];
    ReduceInput ri;
    std::shared_ptr<Reducer> reducer(REDUCER_CREATE(spec_->reducer_class));
    for (int i = 0; i < spec_->num_inputs; ++i)
    {
        sprintf(tmp, input_format.c_str(), i);
        ri.add_file(tmp);
    }

    char tmp2[1024];
    strcpy(tmp2, spec_->output_format.c_str());
    sprintf(tmp, "mkdir -p %s", dirname(tmp2));
    system(tmp); // make output directory

    string outputfile = spec_->output_format;
    sprintf(tmp, outputfile.c_str(), net_reducer_index(index_));

    reducer->set_reduceinput(&ri);
    reducer->set_outputfile(tmp);
    reducer->reducework();

    net_->send(0, netcomm_type::LMR_REDUCER_DONE, nullptr, 0);
}

void MapReduce::reducer_done(int net_index)
{
    bool finished = false;
    {
        std::lock_guard<std::mutex> lk(s_mutex);
        if (++reducer_finished_cnt_ == spec_->num_reducers)
            finished = true;
    }    
    if (finished)
    {
        fprintf(stderr, "Reduce Time: %fs.\n", duration_cast<duration<double>>(high_resolution_clock::now() - time_cnt_).count());
        fprintf(stderr, "ALL WORK DONE!\n");
        for (int i = 1; i < real_total; ++i)
            net_->send(i, netcomm_type::LMR_CLOSE, nullptr, 0);
        system("rm -rf tmp/");
        stopflag_ = true;
    }
}

void MapReduce::mapper_done(int net_index, const vector<int>& finished_index)
{
    int job_index = -1;

    {
        std::lock_guard<std::mutex> lk(s_mutex);
        if (!jobs_.empty())
        {
            job_index = jobs_.front();
            jobs_.pop();
        }
        mapper_finished_cnt_ += finished_index.size();
        if (mapper_finished_cnt_ == spec_->num_inputs) // all the mappers finished.
        {
            fprintf(stderr, "ALL MAPPER DONE!\n");
            fprintf(stderr, "Map Time: %fs.\n", duration_cast<duration<double>>(high_resolution_clock::now() - time_cnt_).count());
            time_cnt_ = high_resolution_clock::now();
            for (int i = 0; i < spec_->num_reducers; ++i)
                net_->send(reducer_net_index(i), netcomm_type::LMR_ASSIGN_REDUCER,
                        form_assign_reducer(string("tmp/tmp_%d_") + to_string(i) +".txt"));
        }
    }

    if (job_index >= 0)
        net_->send(net_index, netcomm_type::LMR_ASSIGN_MAPPER,
                    form_assign_mapper("tmp/tmp_%d_%d.txt", {job_index}));
}

void MapReduce::assign_mapper(const string& output_format, const vector<int>& input_index)
{
    char *tmp = new char[spec_->input_format.size() + 1024];
    std::shared_ptr<char> tmpIt(tmp, [](char* p) { delete[] p; });
    MapInput mi;
    std::shared_ptr<Mapper> mapper(MAPPER_CREATE(spec_->mapper_class));
    for (int i : input_index)
    {
        sprintf(tmp, spec_->input_format.c_str(), i);
        mi.add_file(tmp);
    }
    string outputfile = output_format;
    outputfile.replace(outputfile.find("%d"), 2, to_string(input_index[0]));

    mapper->set_mapinput(&mi);
    mapper->set_numreducer(spec_->num_reducers);
    mapper->set_outputfile(outputfile);
    mapper->mapwork();

    net_->send(0, netcomm_type::LMR_MAPPER_DONE, form_mapper_done(input_index));
}

MapReduce::MapReduce(MapReduceSpecification* spec)
{
    instance = this;
    setbuf(stdout, nullptr);
    std::lock_guard<std::mutex> lk(s_mutex);
    set_spec(spec);
}

void MapReduce::set_spec(MapReduceSpecification *spec)
{
    if (!spec) return;
    spec_ = spec;

    total_ = spec_->num_mappers + spec_->num_reducers + 1;
    if (firstspec_)
    {
        firstspec_ = false;
        real_total = total_;
        index_ = spec_->index;
        net_ = new netcomm(spec_->config_file, spec_->index, cb);
    }

    if (total_ > net_->gettotalnum())
    {
        fprintf(stderr, "Too many mappers and reducers. Please add workers in configuration file.\n");
        exit(1);
    }

    if (spec_->num_mappers < 1 || spec_->num_reducers < 1)
    {
        fprintf(stderr, "Number of both mappers and reducers must be at least one.\n");
        exit(1);
    }
}

void MapReduce::start_work()
{
    printf("Start word from %d.\n", index_);
    if (!spec_)
    {
        fprintf(stderr, "No specification.\n");
        exit(1);
    }

    reducer_finished_cnt_ = mapper_finished_cnt_ = 0;

    if (index_ > 0)
    {
        printf("Checkin in %d.\n", index_);
        firstrun_ = false;
        net_->send(0, netcomm_type::LMR_CHECKIN, nullptr, 0);
    }
    else
    {
        printf("Wait for checkin in %d.\n", index_);
        if (firstrun_ && !dist_run_files())
        {
            fprintf(stderr, "distribution error. cannot run workers.\n");
            net_->wait();
            exit(1);
        }
        firstrun_ = false;

        for (int i = 0; i < spec_->num_inputs; ++i)
            jobs_.push(i);
        system("rm -rf tmp/ && mkdir tmp");

        while (!isready_)
            sleep_us(1000);
        isready_ = false;

        printf("All checked in.\n");
        time_cnt_ = high_resolution_clock::now();
        std::lock_guard<std::mutex> lk(s_mutex);
        cout << "===========spec_->num_mappers:" << spec_->num_mappers << endl;
        for (int i = 0; i < spec_->num_mappers; ++i)
        {
            if (jobs_.empty())
                break;
            else
            {
                cout << "===========i=" << i << ", jobs_.size()=" << jobs_.size() << endl;

                net_->send(mapper_net_index(i), netcomm_type::LMR_ASSIGN_MAPPER,
                            form_assign_mapper("tmp/tmp_%d_%d.txt", {jobs_.front()}));
                jobs_.pop();
            }
        }
    }
}

MapReduce::~MapReduce()
{
    if (net_)
    {
        net_->wait();
        delete net_;
    }
}

int MapReduce::work(MapReduceResult& result)
{
    time_point<chrono::high_resolution_clock> start = high_resolution_clock::now();
    start_work();
    while (!stopflag_)
        sleep_us(1000);
    stopflag_ = false;
    result.timeelapsed = duration_cast<duration<double>>(high_resolution_clock::now() - start).count();
    return 0;
}

bool run_sshcmd(const string& ip, const string& username, const string& password, const string& cmd)
{
    ssh_session my_ssh_session;
    ssh_channel channel;
    int rc = 0;
    my_ssh_session = ssh_new();
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, ip.c_str());
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username.c_str());
    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error connecting to localhost: %s\n", ssh_get_error(my_ssh_session));
        ssh_free(my_ssh_session);
        return false;
    }
    if (password.empty())
        rc = ssh_userauth_publickey_auto(my_ssh_session, nullptr, nullptr);
    else
        rc = ssh_userauth_password(my_ssh_session, nullptr, password.c_str());
    if (rc != SSH_AUTH_SUCCESS)
    {
        fprintf(stderr, "Authentication failure.\n");
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return false;
    }
    channel = ssh_channel_new(my_ssh_session);
    ssh_channel_open_session(channel);
    ssh_channel_request_exec(channel, cmd.c_str());

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return true;
}

int getch()
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

void getpass(string& pass)
{
    char c;
    pass.clear();
    printf("please input your password: ");
    while ((c = getch()) != '\n')
        pass.push_back(c);
    putchar('\n');
}

bool MapReduce::dist_run_files()
{
    // get cwd
    char *tmp = getcwd(nullptr, 0);
    string cwd(tmp), temp, username, password;
    free(tmp);

    // get username and password
    ifstream f(spec_->config_file);
    getline(f, temp);
    size_t pos = temp.find(':');
    username = temp.substr(0, pos);
    password = temp.substr(pos + 1);
    if (!password.empty() && password.back() == '\r')
        password.pop_back();
    if (password == "*")
        getpass(password);
    else
        password = base64_decode(password);

    // run workers
    unordered_map<string, vector<pair<int,int>>> um;
    for (int i = 1; i < real_total; ++i)
        um[net_->endpoints[i].first].push_back(make_pair(i, net_->endpoints[i].second));

    for (auto& p : um)
    {
        string cmd = "cd " + cwd + " && mkdir -p output";
        for (auto& p2 : p.second)
        {
            cmd += " && (./" + spec_->program_file + " " + to_string(p2.first) +
                    " >& output/output_" + to_string(p2.first) + ".txt &)";
        }
        if (p.first == "127.0.0.1")
            system(cmd.c_str());
        else if (!run_sshcmd(p.first, username, password, cmd))
            return false;
    }
    return true;
}

}