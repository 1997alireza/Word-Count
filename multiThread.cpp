#include <iostream>
#include <thread>
#include <malloc.h>
#include "usageDeterminer/cpuUtilization.h"
#include "usageDeterminer/memoryUtilization.h"

using namespace std;

void count(short thread_number, long start_byte, long end_byte);
char * buffer;
int* wordCount;

int main()
{
    const short THREAD_NUMBER = 8;
    FILE *fp;
    thread t[THREAD_NUMBER];
    struct timespec tstart={0,0}, tend={0,0};
    wordCount = new int[THREAD_NUMBER];

    clock_gettime(CLOCK_MONOTONIC, &tstart);
    double cpuUsage;
    int memUsage;
    CpuUtilization *cpuUtil = new CpuUtilization();
    cpuUtil->init();

    fp = fopen("../test.txt", "r");

    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    if (fileSize == -1) {fputs ("Reading error",stderr); exit (1);}
    rewind(fp);
    buffer = (char*) malloc (sizeof(char)*fileSize);
    if (buffer == NULL) {fputs ("Memory error",stderr); exit (1);}

    size_t result = fread (buffer, 1, fileSize, fp);

    if (result != fileSize) {fputs ("Reading error",stderr); exit (1);}
    fclose(fp);

    long eachSecSize = fileSize / THREAD_NUMBER;
    for (int i = 0; i < THREAD_NUMBER-1; ++i)
    {
        t[i] = thread(count, i, eachSecSize * i, eachSecSize * (i+1) - 1 );
    }
    t[THREAD_NUMBER-1] = thread(count, THREAD_NUMBER-1, eachSecSize * (THREAD_NUMBER-1), fileSize-1);

    int wordcount = 0;
    for (int i = 0; i < THREAD_NUMBER; ++i)
    {
        t[i].join();
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
    return 0;
}

void count(short thread_number, long start_byte, long end_byte)
{
    int wordcount = 0;
    char ch;
    for(long i = start_byte; i <= end_byte; i++)
    {
        ch = buffer[i];
        if (ch == ' ' || ch == '\n')  ++wordcount;
    }

    wordCount[thread_number] = wordcount;
};
