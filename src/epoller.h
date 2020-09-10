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

class EpollCallback {
public:
	typedef std::shared_ptr<EpollCallback> Ptr;
	EpollCallback() {};
	virtual ~EpollCallback() {};
	virtual void doEvent(struct epoll_event*) {};
};


class Epoller : public Noncopyable {
public:
    typedef std::shared_ptr<Epoller> Ptr;

    Epoller();
    ~Epoller() {}

    int initServer(int port, EpollCallback* cb);
    void working(int maxEvents, int timeout);
    int connectServer(std::string ip, int port, EpollCallback* cb);

    void addEvent(int fd, int events, EpollCallback* cb);
    void removeEvent(int fd);
    void modifyEvent(int fd, int events, EpollCallback* cb);

    static int send(int fd, const char* data, int size);
private:
    void setnonblocking(int fd);

private:
    int epfd_ = -1;
};

}


#endif