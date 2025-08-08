// AVEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <windows.h>
#include <fltuser.h>
#include <atlbase.h>
#include <stdio.h>
#include <vector>
#include <thread>
#pragma comment(lib, "FltLib.lib")
#include "VinGuardUMessageDefs.h"


/*
* Message always begins with a [Filter_Message_Header]->[User_Data_Buffer]->[Overlapped_structure]
*/
struct MESSAGE_WITH_HEADER_OVERLAPPED {
    FILTER_MESSAGE_HEADER header;
    WCHAR buffer[1024];
    OVERLAPPED overlapped;
};

bool g_stop = false;

void worker_thread(int thread_id, HANDLE port, HANDLE completion_port)
{
    while (!g_stop) {

        LPOVERLAPPED overlapped = nullptr;
        std::unique_ptr<MESSAGE_WITH_HEADER_OVERLAPPED> non_overlapped_buffer(new MESSAGE_WITH_HEADER_OVERLAPPED());
        ZeroMemory(non_overlapped_buffer.get(), sizeof(MESSAGE_WITH_HEADER_OVERLAPPED));

        HRESULT hr = FilterGetMessage(
            port,
            (PFILTER_MESSAGE_HEADER)&(non_overlapped_buffer->header),
            sizeof(MESSAGE_WITH_HEADER_OVERLAPPED),
            (LPOVERLAPPED)&(non_overlapped_buffer->overlapped));

        if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
            printf("FilterGetMessage failed. Error: %8x\n", hr);
            continue;
        }

        ULONG_PTR key = 0;
        DWORD bytes_tx = 0;
        BOOL result = GetQueuedCompletionStatus(
            completion_port,
            &bytes_tx,
            &key,
            &overlapped,
            INFINITE);

        VinGuard::filter_message re_constructed_msg;
        VinGuard::MEMORY_STREAM buffer;
        buffer.buffer = (LPBYTE)(non_overlapped_buffer->buffer);
        buffer.capacity = 1024;
        re_constructed_msg.deserialize(&buffer);



        wprintf(L"Thread : %i Scan requested for file: %u - %u - %s\n",
            thread_id, 
            re_constructed_msg.m_file_action,
            re_constructed_msg.m_filter_verdict,
            re_constructed_msg.m_filename);
    }
}

int main() {
    HANDLE port;
    HANDLE completion_port;
    HRESULT hr;
    DWORD bytesReturned;

    printf("Connecting to filter...\n");

    hr = FilterConnectCommunicationPort(
        L"\\c7fe3b44-4759-4a07-a37a-ef89a9316096",
        0,
        NULL,
        0,
        NULL,
        &port
    );

    if (FAILED(hr)) {
        printf("Failed to connect to filter. Error: 0x%x\n", hr);
        return 1;
    }

    completion_port = CreateIoCompletionPort(port, nullptr, 0, 0);
    if (!completion_port) {
        DWORD le = ::GetLastError();
        printf("Failed creating IO Port - %u", le);
        CloseHandle(port);
        return 1;
    }

    printf("Connected to filter successfully!\n");
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {

        workers.emplace_back(worker_thread, i, port, completion_port);

    }


    for (auto& t : workers)
        t.join();

    g_stop = true;


    CloseHandle(port);
    CloseHandle(completion_port);

    return 0;
}
