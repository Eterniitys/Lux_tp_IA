#ifndef tools_cpp
#define tools_cpp

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

#endif