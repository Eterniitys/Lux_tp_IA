#ifndef tools_cpp
#define tools_cpp

using namespace std;
using namespace lux;

/*
    char ch[128];
    sprintf(ch, "(pointer) ->%p > %d,%d", &(closestCityTile->pos), (closestCityTile->pos).x, (closestCityTile->pos).y);
    WriteLog(ch);
*/

// Function to write a log message to a file
void WriteLog(string log, bool reset = false)
{
    // Declare a FILE pointer variable called pFile
    FILE *pFile;

    // If reset is true, open the file "logFile.txt" in write mode ("w") and assign it to pFile
    if (reset)
        pFile = fopen("logFile.txt", "w");

    // If reset is false, open the file "logFile.txt" in append mode ("a") and assign it to pFile
    else
        pFile = fopen("logFile.txt", "a");
    fprintf(pFile, "%s\n", log.c_str());
    fclose(pFile);
}

#endif