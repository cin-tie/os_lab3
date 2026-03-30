#include "marker.h"
#include <iostream>

DWORD WINAPI markerThread(LPVOID param){
    ThreadData* threadData = static_cast<ThreadData*>(param);

    srand(threadData->id);

    WaitForSingleObject(threadData->startEvent, INFINITE);

    while (true){
        int index = rand() % threadData->size;

        EnterCriticalSection(threadData->cs);

        if(threadData->array[index] == 0){
            Sleep(5);

            threadData->array[index] = threadData->id;
            threadData->markedCount++;

            LeaveCriticalSection(threadData->cs);

            Sleep(5);
        }
        else{
            std::cout << "Thread " << threadData->id 
                        << " marked: " << threadData->markedCount
                        << " cannot mark index: " << index << std::endl;
            
            LeaveCriticalSection(threadData->cs);
        
            SetEvent(threadData->cannotContinueEvent);

            HANDLE events[2] = {threadData->stopEvent, threadData->continueEvent};
            DWORD res = WaitForMultipleObjects(2, events, FALSE, INFINITE);

            if(res == WAIT_OBJECT_0){
                EnterCriticalSection(threadData->cs);

                for(int i = 0; i < threadData->size; ++i){
                    if(threadData->array[i] == threadData->id){
                        threadData->array[i] = 0;
                    }
                }
                
                LeaveCriticalSection(threadData->cs);

                return 0;
            }
            else if(res == WAIT_OBJECT_0 + 1){
                ResetEvent(threadData->cannotContinueEvent);
                ResetEvent(threadData->continueEvent);
                continue;
            }
        }
    }
}
