#include <iostream>
#include <malloc.h>
#include <thread>
#include <windows.h>

using namespace std;

LPCSTR count(short process_number, long start_byte, long end_byte);

char * buffer;
int* wordCount;

int main() {
    const short PROCESS_NUMBER = 10;
    FILE *fp;
    HANDLE p[PROCESS_NUMBER];
    struct timespec tstart={0,0}, tend={0,0};
    wordCount = new int[PROCESS_NUMBER];

    clock_gettime(CLOCK_MONOTONIC, &tstart);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

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

    long eachSecSize = fileSize / PROCESS_NUMBER;
    for (int i = 0; i < PROCESS_NUMBER-1; ++i) {
        p[i] = (HANDLE) CreateProcess(count(i, eachSecSize * i, eachSecSize * (i+1) - 1 ), NULL, NULL, NULL, FALSE, 0,
                                      NULL, NULL, &si, &pi);
    }
    p[PROCESS_NUMBER-1] = (HANDLE) CreateProcess(count(PROCESS_NUMBER-1, eachSecSize * (PROCESS_NUMBER-1), fileSize-1),
                                                 NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    int wordcount = 0;
    for (int i = 0; i < PROCESS_NUMBER; ++i) {
        WaitForSingleObject(p[i], INFINITE);
        wordcount += wordCount[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);

    cout << wordcount << endl;

    printf("time(sec): %.5f\n",
           ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
           ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));

    free(buffer);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

LPCSTR count(short process_number, long start_byte, long end_byte){
    int wordcount = 0;
    char ch;
    for(long i = start_byte; i <= end_byte; i++){
        ch = buffer[i];
        if (ch == ' ' || ch == '\n')  ++wordcount;
    }

    wordCount[process_number] = wordcount;

    return 0;
};