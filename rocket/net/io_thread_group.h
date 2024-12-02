#pragma once
#include "god.h"
#include "io_thread.h"
#include "log.h"
#include <vector>

namespace rocket {
class IOThreadGroup {
public:
    IOThreadGroup(int size);
    
    ~IOThreadGroup();

    void start();

    void join() {
        for (auto& v : m_io_thread_groups) {
            v->join();
        }
    }

    IOThread* getIOThread();

private:
    int m_size = -1;
    std::vector<IOThread*> m_io_thread_groups;
    uint32_t m_index = 0;
};
}