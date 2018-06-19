#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include "UsefullFunctions.h"
#include "Macros.h"

namespace functions
{
    using namespace std;

    string binDir;

    int openOutputFile(ofstream & outputFileStream, string path, string fileName, std::ios_base::openmode mode)
    {
        outputFileStream.open(binDir + "/" + path + "/" + fileName, mode);
        if(!outputFileStream.good())
        {
            cout << "Could not open " + binDir + "/" + path + "/" + fileName + "! Exit now!" << std::endl;
            return false;
        }

        return true;
    }

    int openInputFile(ifstream & inputFileStream, string path, string fileName)
    {
        inputFileStream.open(binDir + "/" + path + "/" + fileName);
        if(!inputFileStream.good())
        {
            cout << "Could not open " + binDir + "/" + path + "/" + fileName + "! Exit now!" << std::endl;
            return false;
        }

        return true;
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
        UNUSED(argc);

        binDir = string(argv[0]);
        binDir = std::string(getPathName(binDir));
    }
}