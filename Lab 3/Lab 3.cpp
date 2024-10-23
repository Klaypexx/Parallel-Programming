#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <mutex>
#include <mmsystem.h> // Для timeGetTime()
#include <fstream>
#include <sstream>

#pragma comment(lib, "winmm.lib")

std::mutex mtx;
const int operationsPerThread = 21;
DWORD startTime;
int numThreads = 2;

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    // Получаем номер потока
    unsigned int threadNumber = *(unsigned int*)lpParam;

    std::ostringstream filename;
    filename << "thread_" << threadNumber << ".txt";
    std::ofstream outFile(filename.str());
    std::ostringstream output;
    
    if (!outFile.is_open()) {
        std::cerr << "Error: unable to open file for thread " << threadNumber << std::endl;
        return 1;
    }

    for (int i = 0; i < operationsPerThread; ++i) {
        for (int j = 0; j < 1'000'000; ++j) {
            for (int k = 0; k < 1'000; ++k) {
            }
        }
        DWORD currentTime = timeGetTime();
        output << threadNumber << "|" << currentTime - startTime << std::endl;
    }

    outFile << output.str();

    outFile.close();
    return 0;
}

int main()
{
    HANDLE* handles = new HANDLE[numThreads];
    unsigned int* threadNumbers = new unsigned int[numThreads];

    // Создаем потоки
    for (int i = 0; i < numThreads; ++i)
    {
        threadNumbers[i] = i + 1; // Нумерация потоков начинается с 1
        handles[i] = CreateThread(NULL, 0, &ThreadProc, &threadNumbers[i], CREATE_SUSPENDED, NULL);
        if (handles[i] == NULL)
        {
            std::cerr << "Flow error " << i + 1 << std::endl;
            return 1;
        }

        SetThreadPriority(handles[i], THREAD_PRIORITY_HIGHEST);
    }

    std::cout << "Press Enter... ";
    std::cin.get();

    startTime = timeGetTime();

    for (int i = 0; i < numThreads; ++i)
    {
        ResumeThread(handles[i]);
    }

    // Ожидаем завершения всех потоков
    WaitForMultipleObjects(numThreads, handles, true, INFINITE);

    for (int i = 0; i < numThreads; ++i)
    {
        CloseHandle(handles[i]);
    }

    delete[] handles;
    delete[] threadNumbers;

    return 0;
}