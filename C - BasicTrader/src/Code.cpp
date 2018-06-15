#include "helper/Macros.h"
#include "helper/UsefullFunctions.h"
#include "helper/ConfigManager.h"

using namespace std;
using namespace config;

string ingridPath, outgridPath;

int main(int argc, const char *argv[]) 
{
    // if(argc != 3)
    // {
    //     cout << "Aufrufen mit: " << argv[0] << " INGRID.vtk OUTGRID.vtk\n";
    //     exit(EXIT_FAILURE);
    // }

    functions::init(argc, argv);
    config::loadConfig();

    
}