#ifndef LMR_NETCOMM_H
#define LMR_NETCOMM_H

#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <functional>
#include "event2/event.h"
#include "event2/thread.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include "event2/util.h"
#include <arpa/inet.h>

#include "common.h"

namespace lmr {

class netcomm;
using namespace std;
const short CURVERSION = 1;

struct header {
    unsigned short version, type;
    unsigned int length, src, dst;
};

typedef std::function<void(header*, char*, netcomm*)> Func;

/**
 * 事件类型
 */ 
enum netcomm_type {
    LMR_HELLO,               // hello message
    LMR_CHECKIN,             // checkin 签入事件
    LMR_CLOSE,               // close immediately  关闭
    LMR_ASSIGN_MAPPER,       // output format (_%d_%d), input_indices  // mapper
    LMR_MAPPER_DONE,         // finished_indices   // mapper完成
    LMR_ASSIGN_REDUCER,      // assign input format(_%d_(fixed))  // reducer
    LMR_REDUCER_DONE,        // DONE              // reducer完成
};

class netcomm
{
    friend void listener_cb(struct evconnlistener *listener_, evutil_socket_t fd,
                            struct sockaddr* sa, int socklen, void* ctx);
    friend void read_cb(struct bufferevent* bev, void* ctx);
    friend void event_cb(struct bufferevent* bev, short events, void* ctx);
public:
    netcomm(string configfile, int index, Func f);
    ~netcomm();
    void send(int dst, unsigned short type, char* src, int size);
    void send(int dst, unsigned short type, string data);
    int gettotalnum();
    void wait();

    vector<pair<string, uint16_t>> endpoints;

private:
    void readconfig(string& configfile);
    void net_init();
    void net_connect(int index);

    vector<struct bufferevent*> net_buffer_;          // libevent连接缓存
    unordered_map<struct bufferevent*, int> net_um_;
    struct event_base* net_base_ = nullptr;
    struct evconnlistener* listener_ = nullptr;
    int myindex_;
    Func cbfun_;
    pthread_t pid_;

};

}

#endif //LMR_NETCOMM_H
