#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "rocket/common/util.h"

namespace rocket
{
static int g_pid = 0;
static thread_local int g_thread_id = 0;

pid_t getPid() {
    if (g_pid != 0) {
        return g_pid;
    }
    g_pid = getpid();
    return getpid();
}

pid_t getThreadId() {
    if (g_thread_id) {
        return g_thread_id;
    }
    g_thread_id = syscall(SYS_gettid);
    return g_thread_id;
}
} // namespace rocket