#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "rocket/common/config.h"



namespace rocket{

static Logger* g_logger = nullptr;
Logger* Logger::GetGlobalLogger() {
    if (g_logger) {
        return g_logger;
    }
    LogLevel loglevel = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
    g_logger = new Logger(loglevel);
    return g_logger;
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
        <<"[" << time_str << "]\t" << "[" << string(__FILE__) << ":"<< __LINE__ << "]\t";

    return ss.str();
}

void Logger::pushLog(const string& msg) {
   cout << "pushed log\n";
    m_buffer.emplace(msg);
    cout << "mbuffer size:" << m_buffer.size() << endl;
    //if (event.getLogLevel() < m_set_level) {
    //    return;
    //}
}

void Logger::log() {
    cout << "m buffer size:" << m_buffer.size() << endl;
    while (!m_buffer.empty())  {
        string msg = m_buffer.front();
        m_buffer.pop();
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