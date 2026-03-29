#include <windows.h>
#include <iostream>
#include <vector>
#include "marker.h"

void printArray(int* array, int size){
    for(int i = 0; i < size; ++i){
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}

int main(){
    int size;
    std::cout << "Array size: ";
    std::cin >> size;

    if(size <= 0){
        std::cerr << "Invalid size\n";
        return 1;
    }

    int* array = new int[size]{0};
    
    int threadCount;
    std::cout << "Threads count: ";
    std::cin >> threadCount;

    if(threadCount < 0){
        std::cerr << "Invalid threads count\n";
        delete[] array;
        return 1;
    }

    if(threadCount == 0){
        std::cout << "Nothing to do. Finishing...\n";
        delete[] array;
        return 0;
    }

    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    HANDLE startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    std::vector<HANDLE> threads(threadCount);
    std::vector<ThreadData> data(threadCount);

    std::vector<HANDLE> cannotContinueEvents(threadCount);
    std::vector<HANDLE> stopEvents(threadCount);
    std::vector<HANDLE> continueEvents(threadCount);

    for(int i = 0; i < threadCount; ++i){
        cannotContinueEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        stopEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        continueEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);

        data[i] = {
            i + 1,
            array,
            size,
            startEvent,
            stopEvents[i],
            continueEvents[i],
            cannotContinueEvents[i],
            &cs,
            0
        };

        threads[i] = CreateThread(NULL, 0, markerThread, &data[i], 0, NULL);

        if (!threads[i]) {
            std::cerr << "Thread creation failed: " << GetLastError() << std::endl;
            return 1;
        }
    }

    SetEvent(startEvent);

    int activeThreads = threadCount;

    while (activeThreads > 0) {
        WaitForMultipleObjects(activeThreads, cannotContinueEvents.data(), TRUE, INFINITE);

        std::cout << "Array:\n";
        printArray(array, size);

        int id;
        std::cout << "Stop thread id: ";
        std::cin >> id;

        while(id < 1 || id > threadCount){
            std::cerr << "Invalid thread id. Try again\n\n";
            std::cout << "Stop thread id: ";
            std::cin >> id;
        }

        SetEvent(stopEvents[id - 1]);

        WaitForSingleObject(stopEvents[id - 1], INFINITE);

        std::cout << "After stopping:\n";
        printArray(array, size);

        activeThreads--;

        for(int i = 0; i < threadCount; ++i){
            if(i != id - 1){
                SetEvent(continueEvents[i]);
            }
        }
    }

    DeleteCriticalSection(&cs);
    delete[] array;
    return 0;
}