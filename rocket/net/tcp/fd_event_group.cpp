#include "fd_event_group.h"
#include "fd_event.h"
#include <cstddef>
#include <mutex>

namespace rocket {

static FdEventGroup* g_fd_event_group = nullptr;

FdEventGroup* FdEventGroup::GetFdEventGroup() {
   if (g_fd_event_group) return g_fd_event_group; 

   g_fd_event_group = new FdEventGroup(128);
}

FdEventGroup::FdEventGroup(int size) {
    m_fd_group.resize(size);
    for (int i = 0; i < size; i++) {
        m_fd_group.emplace_back(new FdEvent(i));
    }
}

FdEventGroup::~FdEventGroup() {
    for (auto& v : m_fd_group) {
        delete v;
    }
}

FdEvent* FdEventGroup::getFdEvent(int fd) {
    std::lock_guard<std::mutex> lk{m_mutex};

    if (fd < m_fd_group.size()) return m_fd_group[fd];

    int new_size = fd*1.5;

    for (int i = m_fd_group.size(); i < new_size; i++) {
        m_fd_group.emplace_back(new FdEvent(i));
    }

    return  m_fd_group[fd];
}
}