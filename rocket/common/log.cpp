#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include "rocket/common/util.h"
#include "rocket/common/log.h"
#include "rocket/common/config.h"



namespace rocket{

static Logger* g_logger = nullptr;

Logger* Logger::GetGlobalLogger() {
  return g_logger;   
}

void Logger::InitGlobalLogger() {
    LogLevel loglevel = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
    g_logger = new Logger(loglevel);
}

string logLevelToString(LogLevel l) {
    switch (l)
    {
    case Debug:
        return "Debug";
    
    case Info:
        return "Info";

    case Error:
        return "Error";
    default:
        break;
    }
    return "";
}

string LogEvent::toString() {
    struct timeval now_time;
    gettimeofday(&now_time, nullptr);
    
    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t);

    char buf[128]= {0};
    strftime(buf, 128, "%y-%m-%d %H:%M:%S", &now_time_t);
    string time_str(buf);

    int ms = now_time.tv_usec / 1000;
    time_str += std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();

    std::stringstream ss;
    ss << "[" << logLevelToString(m_level) << "]\t" <<  "[" << time_str << "]\t"
       << "[" <<m_pid << ":" << m_thread_id << "]\t" 
        <<"[" << time_str << "]\t";

    return ss.str();
}

void Logger::pushLog(const string& msg) {
   std::lock_guard<std::mutex> guard(m_mutex);
    m_buffer.emplace(msg);
    //if (event.getLogLevel() < m_set_level) {
    //    return;
    //}
}

void Logger::log() {
  decltype(m_buffer) tmp;
  {
    std::lock_guard<std::mutex> guard(m_mutex);
    tmp.swap(m_buffer);
  }
    while (!tmp.empty())  {
        string msg = tmp.front();
        tmp.pop();
        printf(msg.c_str());
    }
}

std::string LogLevelToString(LogLevel level) {
  switch (level) {
  case Debug:
    return "DEBUG";

  case Info:
    return "INFO";

  case Error:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}

LogLevel StringToLogLevel(const string& log_level) {
  if (log_level == "DEBUG") {
    return Debug;
  } else if (log_level == "INFO") {
    return Info;
  } else if (log_level == "ERROR") {
    return Error;
  } else {
    return Unknown;
  }
}

}