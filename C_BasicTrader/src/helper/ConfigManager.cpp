#include <iostream>
#include <sstream>
#include <map>
#include <fstream>
#include <cstdlib>
#include <string>
#include "ConfigManager.h"
#include "UsefullFunctions.h"

namespace config
{
    using namespace std;

    ConfigInfo configValues;

    void loadConfig()
    {
        configValues = *(new ConfigInfo());

        string line;

        ifstream fileStream;
        functions::openInputFile(fileStream, ".", "config.txt");

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