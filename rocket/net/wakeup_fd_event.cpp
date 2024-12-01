#include <cerrno>
#include <unistd.h>
#include "wakeup_fd_event.h"
#include "fd_event.h"
#include "log.h"

namespace rocket {
    WakeUpFdEvent::WakeUpFdEvent(int fd) : FdEvent(fd) {
    }

    void WakeUpFdEvent::wakeup() {
        char buf[8] = {'a'};
        int rt = write(m_fd, buf, sizeof(buf));
        if (rt != sizeof(buf)) {
            ERRORLOG("write to wake up fd is %d bytes.", rt);
        }
        DEBUGLOG("write wake up fd");
    }



}