#include "epoller.h"
#include <iostream>
#include <unistd.h>


namespace lmr {

class ClientCallback: public EpollCallback {
public:
	typedef std::shared_ptr<ClientCallback> Ptr;
	
	ClientCallback(Epoller& epoll) : epoll_(epoll) {}

	virtual void doEvent(struct epoll_event* event) {
        int clientfd = event->data.fd;
        if (event->events & EPOLLIN)
        {
            int n = ::recv(clientfd, buff, 1024, 0);
			if (n <= 0) {
				std::cout << "close " << clientfd << std::endl;
				::close(clientfd);
				epoll_.removeEvent(clientfd);
				ClientCallback* cltCallback = (ClientCallback*)event->data.ptr;
				event->data.ptr=NULL;
				delete cltCallback;
				return;
			}
			buff[n] = '\0';
			std::cout << "fd: "<< clientfd << ",read " << buff << std::endl;
			::send(clientfd, buff, n, 0);
        }
    }
private:
	Epoller& epoll_;
	char buff[1024];
};

class TcpServer: public EpollCallback {
public:
	typedef std::shared_ptr<TcpServer> Ptr;

	TcpServer(Epoller& epoll) : epoll_(epoll) {}

	virtual void doEvent(struct epoll_event* event) {
        int listenfd = event->data.fd;
        struct sockaddr_in clientaddr;
        socklen_t clilen;
		int clientfd = accept(listenfd, (sockaddr *)&clientaddr, &clilen);
		if (clientfd < 0) {
			std::cout << clientfd << std::endl;
			return;
		}
		char *str = inet_ntoa(clientaddr.sin_addr);
		std::cout << "accept a connection from " << str << std::endl;
		std::shared_ptr<EpollCallback> client = std::make_shared<ClientCallback>(epoll_);
		clients_[clientfd] = client;
		epoll_.addEvent(clientfd, EPOLLIN | EPOLLET, client.get());
	}
private:
	Epoller& epoll_;
	std::unordered_map<int, EpollCallback::Ptr> clients_; 
};

}

using namespace lmr;

int main(int argc, char** argv)
{

    lmr::Epoller epoller;
    TcpServer::Ptr server = std::make_shared<TcpServer>(epoller);
    epoller.initServer(7778, server.get());
    epoller.working(10, -1);
    return 0;
}

