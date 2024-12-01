#pragma once
#include "rocket/net/fd_event.h"
namespace rocket {

class WakeUpFdEvent : public FdEvent{
public:
    WakeUpFdEvent(int fd);
    ~WakeUpFdEvent() = default;

    void wakeup();

private:

};

}