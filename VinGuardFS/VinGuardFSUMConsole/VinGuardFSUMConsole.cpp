// AVEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <windows.h>
#include <fltuser.h>
#include <atlbase.h>
#include <stdio.h>
#pragma comment(lib, "FltLib.lib")

#include "VinGuardUMessageDefs.h"


typedef struct MESSAGE {
    WCHAR buffer[1024]; 
} MESSAGE;
struct SCANNER_MESSAGE : public FILTER_MESSAGE_HEADER, public MESSAGE {};

int main() {
    HANDLE hPort = INVALID_HANDLE_VALUE;
    HRESULT hr;
    DWORD bytesReturned;

    printf("Connecting to filter...\n");

    hr = FilterConnectCommunicationPort(
        L"\\c7fe3b44-4759-4a07-a37a-ef89a9316096",
        0,
        NULL,
        0,
        NULL,
        &hPort
    );

    if (FAILED(hr)) {
        printf("Failed to connect to filter. Error: 0x%x\n", hr);
        return 1;
    }

    printf("Connected to filter successfully!\n");
    
    while (true) {
        SCANNER_MESSAGE message = {};
        memset(&message, 0, sizeof(SCANNER_MESSAGE));

        FILTER_REPLY_HEADER reply = { 0 };
        OVERLAPPED overlapped = { 0 };

        printf("Waiting for scan request from driver...\n");

        HRESULT hr  = FilterGetMessage(
            hPort,
            (PFILTER_MESSAGE_HEADER)&message,
            sizeof(message),
           NULL
        );


        if (FAILED(hr)) {
            DWORD err = GetLastError();
            if (err != ERROR_IO_PENDING) {
                printf("FilterGetMessage failed. Error: %d\n", err);
                break;
            }
            // Wait for I/O to complete
            GetOverlappedResult(hPort, &overlapped, &bytesReturned, TRUE);
        }
        
        VinGuard::filter_message re_constructed_msg;
        VinGuard::MEMORY_STREAM buffer;
        buffer.buffer = (LPBYTE) message.buffer;
        buffer.capacity = 1024;
        re_constructed_msg.deserialize(&buffer);


       
        wprintf(L"Scan requested for file: %u - %u - %s\n", 
            re_constructed_msg.m_file_action, 
            re_constructed_msg.m_filter_verdict, 
            re_constructed_msg.m_filename);

        // Simulate scanning logic

        reply.MessageId = 1;

        hr = FilterReplyMessage(hPort, &reply, sizeof(reply));

        if (FAILED(hr)) {
            printf("FilterReplyMessage failed. Error: 0x%x\n", hr);
        }
        else {
            printf("Replied to driver. Verdict: %s\n", reply.Status ? "Allow" : "Block");
        }

        Sleep(100);  // Throttle loop
    }

    CloseHandle(hPort);
    return 0;
}
