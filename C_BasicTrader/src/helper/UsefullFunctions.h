#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>


namespace functions
{

     using namespace std;

     extern std::string* binDir;

//     inline static int openOutputFile(ofstream & outputFileStream, string path, string fileName, std::ios_base::openmode mode = ofstream::out)
//     {
//         outputFileStream.open(binDir + "/" + path + "/" + fileName, mode);
//         if(!outputFileStream.good())
//         {
//             cout << "Could not open " + binDir + "/" + path + "/" + fileName + "! Exit now!\n";
//             return false;
//         }

//         return true;
//     }

//     inline static int openInputFile(ifstream & inputFileStream, string path, string fileName)
//     {
//         inputFileStream.open(binDir + "/" + path + "/" + fileName);
//         if(!inputFileStream.good())
//         {
//             cout << "Could not open " + binDir + "/" + path + "/" + fileName + "! Exit now!\n";
//             return false;
//         }

//         return true;
//     }

//     using std::string;

//     static string getPathName(const string& s) {
//         char sep = '/';
// #ifdef _WIN32
//         sep = '\\';
// #endif

//         size_t i = s.rfind(sep, s.length());
//         if (i != string::npos) 
//         {
//             return(s.substr(0, i));
//         }

//         return("");
//     }

//     static void init(int argc, const char *argv[])
//     {
//         binDir = argv[0];
//         binDir = getPathName(binDir);
//     }


    using namespace std;

    int openOutputFile(ofstream & outputFileStream, string path, string fileName, std::ios_base::openmode mode = std::ofstream::out);

    int openInputFile(ifstream & inputFileStream, string path, string fileName);

    using std::string;

    string getPathName(const string& s);

    void init(int argc, const char *argv[]);
}
#endif