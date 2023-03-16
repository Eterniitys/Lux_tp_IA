#ifndef tools_cpp
#define tools_cpp

using namespace std;
using namespace lux;

/*
    char ch[128];
    sprintf(ch, "(pointer) ->%p > %d,%d", &(closestCityTile->pos), (closestCityTile->pos).x, (closestCityTile->pos).y);
    WriteLog(ch);
*/

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