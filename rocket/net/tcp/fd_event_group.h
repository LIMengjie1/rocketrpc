#pragma once
#include "god.h"
#include "fd_event.h"
#include <vector>

namespace rocket {
class FdEventGroup {
public:
    FdEventGroup(int);

    ~FdEventGroup();

    FdEvent* getFdEvent(int);
    
    static FdEventGroup*GetFdEventGroup();
private:
    std::vector<FdEvent*> m_fd_group;
    mutex m_mutex;
};

}