#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <map>
#include <string>

typedef std::map<std::string, std::string> ConfigInfo;

namespace config
{
    extern ConfigInfo configValues;

    void loadConfig();
}
#endif