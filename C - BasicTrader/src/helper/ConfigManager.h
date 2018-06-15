#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <iostream>
#include <sstream>
#include <map>
#include <fstream>
#include <cstdlib>
#include <string>
#include "UsefullFunctions.h"

typedef std::map<std::string, std::string> ConfigInfo;

namespace config
{
    using namespace std;

    ConfigInfo configValues;

    void loadConfig()
    {
        string line;

        ifstream fileStream = functions::openInputFile(".", "config.txt");

        while (getline(fileStream, line))
        {
            std::istringstream is_line(line);
            std::string key;
            if (getline(is_line, key, '='))
            {
                std::string value;
                if (key[0] == '#')
                    continue;

                if (getline(is_line, value))
                {
                    configValues[key] = value;
                }
            }
        }
    }

}
#endif