#include "io_thread.h"
#include "log.h"
#include "util.h"
#include <cassert>
#include <cstddef>
#include <semaphore.h>

namespace rocket {
IOThread::IOThread() {
    int rt = sem_init(&m_init_semaphore, 0, 0);
    assert(rt == 0);

    rt = sem_init(&m_start_semaphore, 0, 0);
    assert(rt == 0);

    m_thread = thread(IOThread::Main, this);
    sem_wait(&m_init_semaphore);
    DEBUGLOG("IOThread %d create success", m_thread_id);
}

IOThread::~IOThread() {
    m_event_loop->stop();
    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);

    m_thread.join();
    delete m_event_loop;
    m_event_loop = nullptr;
}

void* IOThread::Main(void *arg) {
    IOThread* thread = static_cast<IOThread*>(arg);
    thread->m_event_loop = new EventLoop();
    thread->m_thread_id = getThreadId();

    sem_post(&thread->m_init_semaphore);
    DEBUGLOG("io thread %d create succ", thread->m_thread_id);

    sem_wait(&thread->m_start_semaphore);
    DEBUGLOG("io thread %d start to loop", thread->m_thread_id);
    thread->m_event_loop->loop();

    return nullptr;
}

void IOThread::start() {
    sem_post(&m_start_semaphore);
    DEBUGLOG("io thread %d launched start", m_thread_id);
}
}