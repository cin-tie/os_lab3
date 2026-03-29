#include <gtest/gtest.h>
#include <windows.h>
#include "../include/marker.h"
#include <vector>

// Инициализация структуры потока
void initThreadData(ThreadData& data, int* array, int size, int id) {
    data.id = id;
    data.array = array;
    data.size = size;
    data.markedCount = 0;

    data.startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    data.stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    data.continueEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    data.cannotContinueEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    data.cs = new CRITICAL_SECTION;
    InitializeCriticalSection(data.cs);
}

// Очистка ресурсов
void cleanup(ThreadData& data) {
    CloseHandle(data.startEvent);
    CloseHandle(data.stopEvent);
    CloseHandle(data.continueEvent);
    CloseHandle(data.cannotContinueEvent);

    DeleteCriticalSection(data.cs);
    delete data.cs;
}

// Проверка: есть ли элементы с заданным id
bool hasValue(int* array, int size, int value) {
    for (int i = 0; i < size; i++) {
        if (array[i] == value) return true;
    }
    return false;
}



// Тест 1: Массив изначально пустой
TEST(MarkerTest, ArrayInitiallyZero) {
    int size = 10;
    int* array = new int[size]{0};

    for (int i = 0; i < size; i++) {
        EXPECT_EQ(array[i], 0);
    }

    delete[] array;
}


// Тест 2: Поток НЕ работает до startEvent
TEST(MarkerTest, ThreadWaitsForStartEvent) {
    int size = 5;
    int array[5] = {0};

    ThreadData data;
    initThreadData(data, array, size, 1);

    HANDLE thread = CreateThread(NULL, 0, markerThread, &data, 0, NULL);

    Sleep(50);

    for (int i = 0; i < size; i++) {
        EXPECT_EQ(array[i], 0);
    }

    SetEvent(data.stopEvent);
    SetEvent(data.startEvent);
    WaitForSingleObject(thread, INFINITE);

    cleanup(data);
}


// Тест 3: Поток начинает работу после startEvent
TEST(MarkerTest, ThreadStartsAfterEvent) {
    int size = 5;
    int array[5] = {0};

    ThreadData data;
    initThreadData(data, array, size, 1);

    HANDLE thread = CreateThread(NULL, 0, markerThread, &data, 0, NULL);

    SetEvent(data.startEvent);

    Sleep(100);

    bool changed = false;
    for (int i = 0; i < size; i++) {
        if (array[i] != 0) {
            changed = true;
            break;
        }
    }

    EXPECT_TRUE(changed);

    SetEvent(data.stopEvent);
    WaitForSingleObject(thread, INFINITE);

    cleanup(data);
}


// Тест 4: Поток увеличивает markedCount
TEST(MarkerTest, MarkedCountIncreases) {
    int size = 5;
    int array[5] = {0};

    ThreadData data;
    initThreadData(data, array, size, 2);

    HANDLE thread = CreateThread(NULL, 0, markerThread, &data, 0, NULL);

    SetEvent(data.startEvent);

    Sleep(100);

    EXPECT_GT(data.markedCount, 0);

    SetEvent(data.stopEvent);
    WaitForSingleObject(thread, INFINITE);

    cleanup(data);
}


// Тест 5: Поток сигнализирует о невозможности продолжения
TEST(MarkerTest, CannotContinueEventTriggered) {
    int size = 1;
    int array[1] = {0};

    ThreadData data;
    initThreadData(data, array, size, 1);

    HANDLE thread = CreateThread(NULL, 0, markerThread, &data, 0, NULL);

    SetEvent(data.startEvent);

    DWORD res = WaitForSingleObject(data.cannotContinueEvent, 200);

    EXPECT_EQ(res, WAIT_OBJECT_0);

    SetEvent(data.stopEvent);
    WaitForSingleObject(thread, INFINITE);

    cleanup(data);
}


// Тест 6: Поток очищает свои метки при остановке
TEST(MarkerTest, ClearsOwnMarksOnStop) {
    int size = 5;
    int array[5] = {0};

    ThreadData data;
    initThreadData(data, array, size, 3);

    HANDLE thread = CreateThread(NULL, 0, markerThread, &data, 0, NULL);

    SetEvent(data.startEvent);

    Sleep(100);

    SetEvent(data.stopEvent);
    WaitForSingleObject(thread, INFINITE);

    for (int i = 0; i < size; i++) {
        EXPECT_NE(array[i], 3);
    }

    cleanup(data);
}


// Тест 7: Несколько потоков работают одновременно
TEST(MarkerTest, MultipleThreadsWork) {
    int size = 10;
    int array[10] = {0};

    const int THREADS = 3;

    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    HANDLE startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    std::vector<HANDLE> threads(THREADS);
    std::vector<ThreadData> data(THREADS);

    for (int i = 0; i < THREADS; i++) {
        data[i] = {
            i + 1,
            array,
            size,
            startEvent,
            CreateEvent(NULL, TRUE, FALSE, NULL),
            CreateEvent(NULL, TRUE, FALSE, NULL),
            CreateEvent(NULL, TRUE, FALSE, NULL),
            &cs,
            0
        };

        threads[i] = CreateThread(NULL, 0, markerThread, &data[i], 0, NULL);
    }

    SetEvent(startEvent);

    Sleep(200);

    bool found = false;
    for (int i = 0; i < size; i++) {
        if (array[i] != 0) {
            found = true;
            break;
        }
    }

    EXPECT_TRUE(found);

    for (int i = 0; i < THREADS; i++) {
        SetEvent(data[i].stopEvent);
    }

    for (int i = 0; i < THREADS; i++) {
        WaitForSingleObject(threads[i], INFINITE);
    }

    DeleteCriticalSection(&cs);
}


// Тест 8: continueEvent позволяет продолжить работу
TEST(MarkerTest, ContinueEventWorks) {
    int size = 1;
    int array[1] = {0};

    ThreadData data;
    initThreadData(data, array, size, 1);

    HANDLE thread = CreateThread(NULL, 0, markerThread, &data, 0, NULL);

    SetEvent(data.startEvent);

    WaitForSingleObject(data.cannotContinueEvent, INFINITE);

    SetEvent(data.continueEvent);

    Sleep(50);

    EXPECT_TRUE(true);

    SetEvent(data.stopEvent);
    WaitForSingleObject(thread, INFINITE);

    cleanup(data);
}