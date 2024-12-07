#pragma once
namespace rocket{
pid_t getPid();
pid_t getThreadId();
int64_t getNowMs();

int32_t getIntFromNetByte(const char* buf);
}