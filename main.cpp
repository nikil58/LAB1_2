#include <iostream>
#include <iostream>
#include <windows.h>
#include <functional>

using namespace std;


size_t callback_count = 0;

VOID CALLBACK FileIOCompletionRoutine(DWORD error, DWORD number_of_bytes, LPOVERLAPPED p_over) { ++callback_count; }

LONGLONG time_calculation(const LARGE_INTEGER& start, const LARGE_INTEGER& end, const LARGE_INTEGER& freq) {
    LARGE_INTEGER res;
    res.QuadPart = end.QuadPart - start.QuadPart;
    res.QuadPart *= 1000000;
    res.QuadPart /= freq.QuadPart;
    return res.QuadPart;
}

int main()
{
    cout << "Enter the path of the file which need to be copied" << endl;
    char path_s[MAX_PATH];
    cin.getline(path_s, MAX_PATH);
    HANDLE opener_s = CreateFile(path_s, GENERIC_READ, 0, nullptr,
                                 OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, nullptr);
    if (opener_s == INVALID_HANDLE_VALUE)
    {
        std::cout << "There is some error Error code: " << GetLastError() << endl;
        return -1;
    }
    cout << "Enter the path of the place where file need to be copied" << endl;
    char path_f[MAX_PATH];
    cin.getline(path_f, MAX_PATH);
    HANDLE opener_f = CreateFile(path_f, GENERIC_WRITE, 0, nullptr,
                                 CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
    if (opener_f == INVALID_HANDLE_VALUE)
    {
        std::cout << "There is some error Error code: " << GetLastError() << endl;
        CloseHandle(opener_s);
        return -1;
    }
    size_t cluster;
    cout << "Enter a size of the cluster" << endl;
    cin >> cluster;
    cluster *= 4096;

    size_t operationCounter;
    cout << "Enter count of operations" << endl;
    cin >> operationCounter;

    int l_source_size = 0;
    DWORD h_source_size = 0;
    l_source_size = GetFileSize(opener_s, &h_source_size);
    BYTE **buffer = new BYTE *[operationCounter];
    for (size_t i = 0; i < operationCounter; ++i)
        buffer[i] = new BYTE[cluster];
    OVERLAPPED *over_read = new OVERLAPPED[operationCounter];
    OVERLAPPED *over_write = new OVERLAPPED[operationCounter];
    for (size_t i = 0; i < operationCounter; ++i)
    {
        over_write[i].Offset = i * cluster;
        over_write[i].OffsetHigh = i * h_source_size;
        over_read[i].Offset = over_write[i].Offset;
        over_read[i].OffsetHigh = over_write[i].OffsetHigh;
    }

        LARGE_INTEGER start_time, end_time, frequency;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start_time);
        size_t current_size = 0;
        int count = 0;
        do {
            size_t opCount = 0;
            for (size_t i = 0; i < operationCounter; ++i) {
                if (l_source_size > 0) {
                    ++opCount;
                    ReadFileEx(opener_s, buffer[i], cluster, &over_read[i], FileIOCompletionRoutine);
                }
            }
            while (callback_count < opCount)
                SleepEx(SIZE_MAX, true);
            for (size_t i = 0; i < operationCounter; i++)
                over_read[i].Offset += cluster * operationCounter;
            callback_count = 0;
            opCount = 0;
            for (size_t i = 0; i < operationCounter; ++i) {
                if (l_source_size > 0) {
                    ++opCount;
                    WriteFileEx(opener_f, buffer[i], cluster, &over_write[i], FileIOCompletionRoutine);
                }
            }
            while (callback_count < opCount)
                SleepEx(SIZE_MAX, true);
            for (size_t i = 0; i < operationCounter; i++)
                over_write[i].Offset += cluster * operationCounter;
            callback_count = 0;
            count++;
            current_size += cluster * count;
        } while (current_size < l_source_size);
        //copy_file(opener_s, opener_f, cluster, operationCounter, l_source_size, buffer, over_read, over_write);

        QueryPerformanceCounter(&end_time);

        std::cout << "Time: " << time_calculation(start_time, end_time, frequency) << " microseconds\n";
    system("pause");

        SetFilePointer(opener_f, l_source_size, nullptr, FILE_BEGIN);
        SetEndOfFile(opener_f);
        CloseHandle(opener_s);
        CloseHandle(opener_f);

        for (size_t i = 0; i < operationCounter; ++i)
            delete[] buffer[i];
        delete[] buffer;
        delete[] over_read;
        delete[] over_write;
        return 0;
    }
