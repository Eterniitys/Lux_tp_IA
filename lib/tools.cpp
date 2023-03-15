#include "../lux/kit.hpp"
#include "../lux/define.cpp"
#include <string.h>
#include <vector>
#include <set>
#include <stdio.h>

using namespace std;
using namespace lux;

void WriteLog(string log, bool reset = false)
{
    FILE *pFile;
    if (reset)
        pFile = fopen("logFile.txt", "w");
    else
        pFile = fopen("logFile.txt", "a");
    fprintf(pFile, "%s\n", log.c_str());
    fclose(pFile);
}
