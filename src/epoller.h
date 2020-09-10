#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <memory>
#include <functional>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unordered_map>


namespace lmr {

class Noncopyable {
public:
	Noncopyable(const Noncopyable& rhs) = delete;
	Noncopyable& operator=(const Noncopyable& rhs) = delete;

protected:
	Noncopyable() = default;
	~Noncopyable() = default;
};

class Callback {
public:
	typedef std::shared_ptr<Callback> ptr;
	virtual ~Callback() {};
	virtual void doEvent(struct epoll_event*) = 0;
};

class Epoller : public Noncopyable {
public:
    typedef std::shared_ptr<Epoller> ptr;

    Epoller();
    ~Epoller();

    int initServer(int port, Callback::ptr cb);
    void start(int maxEvents, int timeout);
    int connectServer(std::string ip, int port, Callback::ptr cb);

    void addEvent(int fd, int events, Callback::ptr cb);
    void removeEvent(int fd);
    void modifyEvent(int fd, int events, Callback::ptr cb);

private:
    void eventLoop(int maxEvents, int timeout);
    void setnonblocking(int fd);

private:
    int epfd_ = -1;
	std::unordered_map<int, Callback::ptr> callbacks_; 
};

}


#endif