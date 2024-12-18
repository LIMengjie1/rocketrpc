#include <tinyxml/tinyxml.h>
#include "rocket/common/config.h"
#include "rocket/common/god.h"


#define READ_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name); \
if (!name##_node) { \
    puts("get node failed"); \
    exit(0); \
} \



#define READ_STR_FROM_XML_NODE(name, parent) \
  TiXmlElement* name##_node = parent->FirstChildElement(#name); \
  if (!name##_node|| !name##_node->GetText()) { \
    printf("Start rocket server error, failed to read config file %s\n", #name); \
    exit(0); \
  } \
  std::string name##_str = std::string(name##_node->GetText()); \

namespace rocket {

static Config* g_config = nullptr;
Config* Config::GetGlobalConfig() {
    return g_config;
}

void Config::SetGlobalConfig(const string& xmlfile) {
    if (g_config == nullptr) {
        g_config = new Config(xmlfile);
    }
}
Config::Config(const string& xmlfile) {
    TiXmlDocument* xml_document = new TiXmlDocument();
    cout << "xmlfile:" << xmlfile << endl;
    bool rt = xml_document->LoadFile(xmlfile.c_str());
    if (!rt) {
        printf("start failed, error:%s\n", xml_document->ErrorDesc());
        exit(0);
    }
    READ_XML_NODE(root, xml_document);
    READ_XML_NODE(log, root_node);
    READ_STR_FROM_XML_NODE(log_level, log_node);
    m_log_level = log_level_str;
    // TiXmlElement* root_node = xml_document->FirstChildElement("root");
    // if (!root_node) {
    //     puts("get node failed");
    //     exit(0);
    // }
    
}
}