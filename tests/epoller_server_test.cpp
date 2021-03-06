#include "epoller.h"
#include <iostream>
#include <unistd.h>


namespace lmr {

class ClientCallback: public Callback {
public:
	ClientCallback(Epoller::ptr epoll) : epoll_(epoll) {}
	virtual void doEvent(struct epoll_event* event) {
        int clientfd = event->data.fd;
        if (event->events & EPOLLIN)
        {
            int n = ::recv(clientfd, buff, 1024, 0);
			if (n <= 0) {
				std::cout << "close " << clientfd << std::endl;
				::close(clientfd);
				epoll_->removeEvent(clientfd);
				return;
			}
			buff[n] = '\0';
			std::cout << "fd: "<< clientfd << ",read " << buff << std::endl;
			::send(clientfd, buff, n, 0);
        }
    }
private:
	Epoller::ptr epoll_;
	char buff[1024];
};

class TcpServer: public Callback, public std::enable_shared_from_this<TcpServer> {
public:
	typedef std::shared_ptr<TcpServer> ptr;
	TcpServer(int port) : port_(port)
	{
		epoll_ = std::make_shared<Epoller>();
	}
	void run() 
	{
		epoll_->initServer(port_, shared_from_this());
		epoll_->start(10, -1);
	}
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
		Callback::ptr client = std::make_shared<ClientCallback>(epoll_);
		epoll_->addEvent(clientfd, EPOLLIN | EPOLLET, client);
	}
private:
	int port_;
	Epoller::ptr epoll_;
};

}

using namespace lmr;

int main(int argc, char** argv)
{
    TcpServer::ptr server = std::make_shared<TcpServer>(7778);
	server->run();

	getchar();
   
    return 0;
}

