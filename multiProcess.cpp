#if defined unix || __unix__ || __unix || linux || __linux__ || __linux
#define LINUX
#endif
#include <iostream>
#include "usageDeterminer/cpuUtilization.h"
#include "usageDeterminer/memoryUtilization.h"
using namespace std;
#ifdef  _WIN32
#include <malloc.h>
#include <thread>
#include <windows.h>
LPCSTR count(short process_number, long start_byte, long end_byte);
#elif defined LINUX
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
void count(short process_number, long start_byte, long end_byte);
#endif

char * buffer;
int* wordCount;

int main()
{
    const short PROCESS_NUMBER = 8;
    FILE *fp;
    struct timespec tstart={0,0}, tend={0,0};

    clock_gettime(CLOCK_MONOTONIC, &tstart);
    double cpuUsage;
    int memUsage;
    CpuUtilization *cpuUtil = new CpuUtilization();
    cpuUtil->init();

#ifdef _WIN32
    HANDLE p[PROCESS_NUMBER];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
#elif defined LINUX
    __pid_t pid[PROCESS_NUMBER];
#endif

    fp = fopen("../test.txt", "r");
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    if (fileSize == -1) {fputs ("Reading error",stderr); exit (1);}
    rewind(fp);
#ifdef _WIN32
    wordCount = new int[PROCESS_NUMBER];
    buffer = (char*) malloc (sizeof(char)*fileSize);
#elif defined LINUX
    buffer = static_cast<char *>(mmap(NULL, sizeof(char) * fileSize , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1, 0));
    wordCount = static_cast<int *>(mmap(NULL, sizeof(int) * PROCESS_NUMBER , PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1, 0));
#endif
    if (buffer == NULL) {fputs ("Memory error",stderr); exit (1);}

    size_t result = fread (buffer, 1, fileSize, fp);

    if (result != fileSize) {fputs ("Reading error",stderr); exit (1);}
    fclose(fp);

    long eachSecSize = fileSize / PROCESS_NUMBER;
    for (short i = 0; i < PROCESS_NUMBER; ++i)
    {
        long end_byte = (i == PROCESS_NUMBER-1) ?  fileSize-1 : eachSecSize * (i+1) - 1;
#ifdef _WIN32
        p[i] = (HANDLE) CreateProcess(count(i, eachSecSize * i, end_byte), NULL, NULL, NULL, FALSE, 0,
                                      NULL, NULL, &si, &pi);
#elif defined LINUX
        __pid_t id = fork();
        if(id == 0){ // child process
            count(i, eachSecSize * i, end_byte);
            exit(0);
        }
        else if (id > 0){ // parent process
            pid[i] = id;
        }
        else
        { // error on creating child process
            fputs ("Creating process error",stderr);
            exit (1);
        }
#endif
    }

    int wordcount = 0;
    for (int i = 0; i < PROCESS_NUMBER; ++i)
    {
#ifdef _WIN32
        WaitForSingleObject(p[i], INFINITE);
#elif defined LINUX
        waitpid(pid[i], 0, 0);
#endif
        wordcount += wordCount[i];
    }
    cpuUsage = cpuUtil->getCurrentValue();
    MemoryUtilization *mm = new MemoryUtilization();
    memUsage = mm->getCurrentlyUsedRAM();
    clock_gettime(CLOCK_MONOTONIC, &tend);

    cout << wordcount << endl;

    cout << "time(sec): " << ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
                             ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec) << endl;
    cout << "Cpu Usage: " << cpuUsage << endl;
    cout << "RAM Usage: " << memUsage << endl;

    free(buffer);
#ifdef _WIN32
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif
    return 0;
}

#ifdef _WIN32
LPCSTR count(short process_number, long start_byte, long end_byte)
{
    int wordcount = 0;
    char ch;
    for(long i = start_byte; i <= end_byte; i++){
        ch = buffer[i];
        if (ch == ' ' || ch == '\n')  ++wordcount;
    }

    wordCount[process_number] = wordcount;

    return 0;
};
#elif defined LINUX
void count(short process_number, long start_byte, long end_byte)
{
    int wordcount = 0;
    char ch;
    for(long i = start_byte; i <= end_byte; i++)
    {
        ch = buffer[i];
        if (ch == ' ' || ch == '\n')  ++wordcount;
    }
    wordCount[process_number] = wordcount;
};
#endif
