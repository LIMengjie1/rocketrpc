#pragma once
#include "eventloop.h"
#include "god.h"
#include <cstddef>
#include <sched.h>
#include <semaphore.h>


namespace rocket {
class IOThread {
public:
    IOThread();

    ~IOThread();

    void start();

    void join() {
        m_thread.join();
    }

    EventLoop* getEventLoop() {
        return m_event_loop;
    }

    static void* Main(void* arg);
private:
    pid_t m_thread_id = -1;
    thread m_thread;
    EventLoop* m_event_loop = nullptr;

    sem_t m_init_semaphore;
    sem_t m_start_semaphore;

};
}