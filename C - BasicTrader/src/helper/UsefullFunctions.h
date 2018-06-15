#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

namespace functions
{
    using namespace std;

    string binDir;


    ofstream openOutputFile(string path, string fileName)
    {
        ofstream result(binDir + "/" + path + "/" + fileName);
        if(!result.good())
        {
            cout << "Could not open " + binDir + "/" + path + "/" + fileName + "! Exit now!\n";
            exit(EXIT_FAILURE);
        }

        return result;
    }

    ifstream openInputFile(string path, string fileName)
    {
        ifstream result(binDir + "/" + path + "/" + fileName);
        if(!result.good())
        {
            cout << "Could not open " + binDir + "/" + path + "/" + fileName + "! Exit now!\n";
            exit(EXIT_FAILURE);
        }

        return result;
    }

    using std::string;

    string getPathName(const string& s) {
        char sep = '/';
#ifdef _WIN32
        sep = '\\';
#endif

        size_t i = s.rfind(sep, s.length());
        if (i != string::npos) 
        {
            return(s.substr(0, i));
        }

        return("");
    }

    void init(int argc, const char *argv[])
    {
        binDir = argv[0];
        binDir = getPathName(binDir);
    }
}
#endif