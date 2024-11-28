#pragma once
#include <string>
#include <queue>
#include <memory>
#include <iostream>
#include <mutex>
#include "rocket/common/config.h"
using std::cout;
using std::endl;

using std::string;
using std::queue;

namespace rocket {

enum LogLevel {
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
};

template<typename... Args>
string formatString(const char* str, Args&&... args) {
    int size = snprintf(nullptr, 0, str, args...);

    string res;
    if (size > 0) {
        res.resize(size);
        snprintf(&res[0], size + 1, str, args...);
    }
    return res;
}

#define DEBUGLOG(str, ...)\
   if (rocket::Logger::GetGlobalLogger()->getLogLevel() >= rocket::Debug) \
   { \
   rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Debug))->toString() +"[" + string(__FILE__) + ":" +  to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
   rocket::Logger::GetGlobalLogger()->log(); \
   } \

#define INFOLOG(str, ...)\
   if (rocket::Logger::GetGlobalLogger()->getLogLevel() >= rocket::Info) \
   { \
   rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Info))->toString() + "[" + string(__FILE__) + ":" +  to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
   rocket::Logger::GetGlobalLogger()->log(); \
   }\

#define ERRORLOG(str, ...)\
   if (rocket::Loggger::GetGlobalLogger()->getLogLevel() >= rocket::Error) \
   { \
   rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Error))->toString() +"[" + string(__FILE__) + ":" +  to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
   rocket::Logger::GetGlobalLogger()->log(); \
   }\

class Logger {
 public:
    using s_ptr = std::shared_ptr<Logger>;
    void pushLog(const string& log);
    void log();
    Logger(LogLevel level) : m_set_level(level){}
    LogLevel getLogLevel() {return m_set_level;}
    static Logger* GetGlobalLogger();
    static void InitGlobalLogger();
 private:
    std::mutex m_mutex;
    LogLevel m_set_level;
    queue<string> m_buffer;
};

string logLevelToString(LogLevel level);

LogLevel StringToLogLevel(const string& str);

class LogEvent {
public:
 
 LogEvent(LogLevel level) : m_level(level) {}
 string getFileName () const {
    return m_file_name;
 }
 
 LogLevel getLogLevel () const {
    return m_level;
 }
 
 string toString();

private:
 string m_file_name;
 string m_file_line;
 int m_pid; // pid_t?
 int m_thread_id;
 string m_time;

 LogLevel m_level;
 //`Logger*  
};

}