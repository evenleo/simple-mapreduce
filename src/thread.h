#ifndef __EVEN_THREAD_H__
#define __EVEN_THREAD_H__

#include "mutex.h"

namespace even {

class Thread : Noncopyable {
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> c, const std::string &name = "")
        : m_cb(cb)
    {
        int rt = pthread_create(&thread_, nullptr, &Thread::run, this);
        if (rt)
        {
            throw std::logic_error("pthread_create error");
        }
    }
    ~Thread()
    {
        if (thread_)
            pthread_detach(thread_);
    }
    void join()
    {
        if (thread_)
        {
            int rt = pthread_join(thread_, nullptr);
            if (rt)
            {
                throw std::logic_error("thread_join error");
            }
            thread_ = 0;
        }
    }

private:
    static void* run(void* arg)
    {
        
    }

private:
    pid_t pid_ = -1;
    pthread_t thread_ = 0;
    std::function<void()> cb_;
};

}

#endif