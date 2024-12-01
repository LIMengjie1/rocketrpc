#pragma once
#include "god.h"
#include <cstdint>
#include <functional>
#include <memory>

namespace rocket {

class TimerEvent {
public:
    using s_ptr = std::shared_ptr<TimerEvent>;

    TimerEvent(int, bool, std::function<void()> cb);
    void resetArriveTime();

    int64_t getArriveTime() const {
        return m_arrive_time;
    }

    void setCancler(bool value) {
        m_is_cancled = value;
    }

    bool isCancled() {
        return m_is_cancled;
    }

    bool isRepeated() {
        return m_is_repeated;
    }

    std::function<void()> getCallBack() {
        return m_task;
    }

private:
    int64_t m_arrive_time;
    int64_t m_interval;
    bool m_is_repeated = false;
    bool m_is_cancled = false;

    std::function<void()> m_task;

};

}