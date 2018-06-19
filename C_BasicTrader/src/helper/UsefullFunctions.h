#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace functions
{
    using namespace std;

    extern string binDir;

    int openOutputFile(ofstream & outputFileStream, string path, string fileName, std::ios_base::openmode mode = std::ofstream::out);

    int openInputFile(ifstream & inputFileStream, string path, string fileName);

    string getPathName(const string& s);

    void init(int argc, const char *argv[]);
}
#endif