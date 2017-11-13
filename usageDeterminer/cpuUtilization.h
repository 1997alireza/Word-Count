#ifndef CPU_UTILIZATION_H
#define CPU_UTILIZATION_H

#ifdef  _WIN32
#include "windows.h"
static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static HANDLE self;

#elif defined unix || __unix__ || __unix || linux || __linux__ || __linux
#include <sys/times.h>
static clock_t lastCPU, lastSysCPU, lastUserCPU;
#endif

static int numProcessors;

class CpuUtilization {
public :
    void init();
    double getCurrentValue();
};

#endif