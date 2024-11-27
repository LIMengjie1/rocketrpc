#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include <iostream>
using namespace std;

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    DEBUGLOG("test debug log %s", "fuck");
    DEBUGLOG("test info log %s", "fuck");
//     std::cout << "ads\n";
//     string msg = (new rocket::LogEvent(rocket::LogLevel::Debug))->toString() + rocket::formatString("%s", "fuck"); 
//        std::cout << "ads\n";
//     cout << "msg:" << msg<<endl;
//     rocket::Logger::GetGlobalLogger()->pushLog(msg); 
//        std::cout << "ads\n";
//     rocket::Logger::GetGlobalLogger()->log();
//    std::cout << "ads\n";
    return 0;
}