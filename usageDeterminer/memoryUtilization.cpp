#include "memoryUtilization.h"
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif defined unix || __unix__ || __unix || linux || __linux__ || __linux
#define LINUX
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int parseLine(char* line){
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}
#endif


int MemoryUtilization::getCurrentlyUsedRAM(){
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    return (int)(pmc.WorkingSetSize / 1000);
#elif defined LINUX
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
#endif
}
