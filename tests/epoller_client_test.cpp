#include "epoller.h"
#include <iostream>
#include <unistd.h>

using namespace lmr;

namespace lmr {

class TcpClient: public EpollCallback {
public:
	typedef std::shared_ptr<TcpClient> Ptr;
	
	TcpClient(Epoller& epoll) : EpollCallback(),
			epoll_(epoll) {

	}

	virtual void doEvent(struct epoll_event* event) {
		std::cout << "do-----" << std::endl;
        int clientfd = event->data.fd;
        if (event->events & EPOLLIN)
        {
            int n = ::read(clientfd, buff, 1024);
			if (n <= 0) {
				std::cout << "close " << clientfd << std::endl;
				::close(clientfd);
				epoll_.removeEvent(clientfd);
				event->data.ptr=NULL;
				return;
			}
			buff[n] = '\0';
			std::cout << "fd:" << clientfd << ", read " << buff << std::endl;
			sleep(3);
            Epoller::send(clientfd, buff, n);
        }
    }

private:
	Epoller& epoll_;
	char buff[1024];
};

}

int main(int argc, char** argv)
{
    Epoller epoller;
    TcpClient::Ptr client = std::make_shared<TcpClient>(epoller);
    int clientfd = epoller.connectServer("127.0.0.1", 7778, client.get());
    char buffer[] = "hello";
    Epoller::send(clientfd, buffer, 6);
    epoller.working(10, -1);

    return 0;
}