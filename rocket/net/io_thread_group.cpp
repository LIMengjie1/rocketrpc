#include "io_thread_group.h"
#include "io_thread.h"

namespace rocket {
IOThreadGroup::IOThreadGroup(int size) : m_size(size){
    m_io_thread_groups.resize(m_size);
    for (int i = 0; i < m_size; i++) {
        m_io_thread_groups[i] = new IOThread();
    }
}

IOThreadGroup::~IOThreadGroup(){}

void IOThreadGroup::start() {
    for (auto& v : m_io_thread_groups) {
        v->start();
    }
}

IOThread* IOThreadGroup::getIOThread() {
     return m_io_thread_groups[m_index++ % m_io_thread_groups.size()];
}
}