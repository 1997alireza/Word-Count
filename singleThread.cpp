#include <iostream>
#include <thread>
#include <malloc.h>
#include "usageDeterminer/cpuUtilization.h"

using namespace std;

char * buffer;

int main() {

    FILE *fp;
    int wordCount = 0;
    struct timespec tstart={0,0}, tend={0,0};

    clock_gettime(CLOCK_MONOTONIC, &tstart);

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

    double cpuUsage;
    CpuUtilization *cpuUtil = new CpuUtilization();
    cpuUtil->init();

    if(fp){
        for(long i = 0; i <= fileSize; i++){
            char ch = buffer[i];
            if (ch == ' ' || ch == '\n')  ++wordCount;
        }
        cpuUsage = cpuUtil->getCurrentValue();
    }
    else {
        cout << "Failed!" << endl;
        exit(1);
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);

    cout << wordCount << endl;


    cout << "time(sec): " << ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
                             ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec) << endl;

    cout << "Cpu Usage: " << cpuUsage << endl;
    return 0;
}