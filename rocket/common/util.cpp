#include <cstring>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#include "rocket/common/util.h"
#include "log.h"

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

int64_t getNowMs() {
  timeval val;
  gettimeofday(&val, NULL);

  auto ret = val.tv_sec * 1000 + val.tv_usec / 1000;
  return ret;
}

int32_t getIntFromNetByte(const char *buf) {
    int32_t ret;
    memcpy(&ret, buf, sizeof(ret));

    return ntohl(ret);
}

} // namespace rocket
