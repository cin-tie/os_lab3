#pragma once
#include <windows.h>
#include <vector>

struct ThreadData{
    int id;
    int* array;
    int size;

    HANDLE startEvent;
    HANDLE stopEvent;
    HANDLE continueEvent;
    HANDLE cannotContinueEvent;

    CRITICAL_SECTION* cs;
    int markedCount;
};

DWORD WINAPI markerThread(LPVOID param);