#pragma once
#include <map>
#include <string>
using std::map;
using std::string;
namespace rocket {
class Config {
 public:
    Config(const string& xmlfile);
    static Config* GetGlobalConfig();
    static void SetGlobalConfig(const string&);

    string m_log_level;
    map<string, string> m_config_values;
};

}