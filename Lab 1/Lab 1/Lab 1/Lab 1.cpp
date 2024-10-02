#include <windows.h>
#include <iostream>
#include <string>

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    // Получаем номер потока
    unsigned int threadNumber = *(unsigned int*)lpParam;

    // Выводим сообщение о работе потока
    std::cout << "Flow " << threadNumber << std::endl;

    ExitThread(0); // Завершение потока
}

int main()
{
    int numThreads;     
    std::cout << "Enter flow count: ";
    std::cin >> numThreads;

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
    }

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