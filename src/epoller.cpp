#include "epoller.h"
#include <iostream>


namespace lmr {

Epoller::Epoller()
{
    epfd_ = epoll_create1(0);
	if (epfd_ == -1) {
        std::cout << "epoll_create1 error" << std::endl;
        exit(0);
    }
}

int Epoller::initServer(int port, EpollCallback* cb)
{
    int fd;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "Failed to create server socket" << std::endl;
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        std::cout << "Failed to bind server socket" << std::endl;
        exit(1);
    }

    if (listen(fd, 10) < 0)
    {
        std::cout << "Failed to listen on server socket" << std::endl;
        exit(1);
    }

    setnonblocking(fd);
    addEvent(fd, EPOLLIN | EPOLLET, cb);
    return fd;
}

/**
 * 事件监听loop ，核心部分
 */ 
void Epoller::working(int maxEvents, int timeout) {
	struct epoll_event* events = (epoll_event*) calloc(maxEvents, sizeof(struct epoll_event));
	while (true) {
		int nfds = epoll_wait(epfd_, events, maxEvents, timeout);
		if (nfds == -1) {
            std::cout << "epoll_wait error" << std::endl;
        }
        for (int i = 0; i < nfds; ++i) {
			if (events[i].data.ptr) {
                EpollCallback* cb = (EpollCallback*)events[i].data.ptr;
                cb->doEvent(&events[i]);
            }
        }
	}
	free(events);
}

int Epoller::connectServer(std::string ip, int port, EpollCallback* cb)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	int rt = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (rt != 0) {
        std::cout << "connect error" << std::endl;
        return rt;
    }
    addEvent(sockfd, EPOLLIN, cb);
	return sockfd;
}

void Epoller::setnonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int Epoller::send(int fd, const char* data, int size) 
{
    return ::send(fd, data, size, 0);
}

void Epoller::addEvent(int fd, int events, EpollCallback* cb)
{
    epoll_event event;
    event.events = events;
    event.data.ptr = cb;
    event.data.fd = fd;
    epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
}

void Epoller::removeEvent(int fd)
{
    epoll_event event;
    event.data.fd = fd;
    epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, &event);
}

void Epoller::modifyEvent(int fd, int events, EpollCallback* cb)
{
    epoll_event event;
    event.events = events;
    event.data.ptr = cb;
    event.data.fd = fd;
    epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
}


}