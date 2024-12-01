#include <cerrno>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <functional>
#include <mutex>
#include <sys/timerfd.h>
#include <vector>
#include "rocket/net/timer.h"
#include "fd_event.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "timer_event.h"

namespace rocket {
Timer::Timer() : FdEvent() {
    m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    DEBUGLOG("timer fd=%d", m_fd);
    listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));
}

Timer::~Timer() {
}

void Timer::onTimer() {
    char buf[8];
    while(1) {
        if (read(m_fd, buf, 8) == -1 && errno == EAGAIN) {
            break;
        }
    }

    int64_t now = getNowMs();

    std::vector<TimerEvent::s_ptr> tmps;
    vector<std::pair<int64_t, std::function<void()>>> tasks;
    std::unique_lock<mutex> lk{m_mutex};
    
    auto it = m_pending_events.begin();
    for (it = m_pending_events.begin(); it != m_pending_events.end(); it++) {
        if (it->first <= now ) {
            if (!(it->second->isCancled())) {
                tmps.push_back(it->second);
                tasks.push_back(std::make_pair(it->second->getArriveTime(), it->second->getCallBack()));
            }
        } else {
            break;
        }
    }

    m_pending_events.erase(m_pending_events.begin(), it);
    lk.unlock();

    for (auto it = tmps.begin(); it != tmps.end(); it++) {
        if ((*it)->isRepeated()) {
            (*it)->resetArriveTime();
            addTimerEvent(*it);
        }
    }
    resetArriveTime();

    for (auto i : tasks) {
        if (i.second) i.second();
    }
}

void Timer::addTimerEvent(TimerEvent::s_ptr event) {
    bool is_reset = false;

    {
        std::lock_guard<mutex> lk{m_mutex};
        if (m_pending_events.empty()) {
            is_reset = true;
        } else {
            auto it = m_pending_events.begin();
            if ((*it).second->getArriveTime() > event->getArriveTime()) {
                is_reset = true;
            }
        }
        DEBUGLOG("add timer event");
        m_pending_events.emplace(event->getArriveTime(), event);
    }

    if (is_reset) {
        DEBUGLOG("reset time, pending size:%d", m_pending_events.size());
        resetArriveTime();
    }
}

void Timer::resetArriveTime() {
   std::unique_lock<mutex> lk{m_mutex};
   auto tmp = m_pending_events;
   lk.unlock();

   if(!tmp.size()) {
    DEBUGLOG("returned");
    return;
   }
   int64_t now = getNowMs();
   DEBUGLOG("now:%lld", now);

   auto it = tmp.begin();
   int64_t inteval = 0;

   if (it->second->getArriveTime() > now) {
    inteval = it->second->getArriveTime() - now;
   } else {
    inteval = 100;
   }

   timespec ts;
   memset(&ts, 0, sizeof(ts));
   ts.tv_sec = inteval / 1000;
   ts.tv_nsec = (inteval % 1000) * 1000000;

   itimerspec value;
   memset(&value, 0, sizeof(value));
   value.it_value = ts;

    int rt = timerfd_settime(m_fd, 0, &value, nullptr);
    if (rt != 0) {
        ERRORLOG("timerfd settime roor, errno=%d, error%s", errno, strerror(errno));
    }
    DEBUGLOG("timer reset to %lld", now + inteval);
}

void Timer::deleteTimerEvent(TimerEvent::s_ptr event) {
    event->setCancler(true);

    std::unique_lock<mutex> lk{m_mutex};

    auto begin = m_pending_events.lower_bound(event->getArriveTime());
    auto end = m_pending_events.upper_bound(event->getArriveTime());

    auto it = begin;
    for (; it != end; it++) {
        if (it->second == event) {
            break;
        }
    }
    if (it != end) {
        m_pending_events.erase(it);
    }
    lk.unlock();

    DEBUGLOG("succ del timerevent at arrive time %lld", event->getArriveTime());
}

}