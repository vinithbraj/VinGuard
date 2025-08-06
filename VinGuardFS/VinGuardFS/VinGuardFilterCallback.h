/*
 * Copyright (c) 2025 Vinith Raj. All rights reserved.
 *
 * Permission is hereby granted to view and read this source code
 * for educational or informational purposes only.
 *
 * Redistribution, modification, reproduction, or use in any software—whether
 * commercial, open-source, or personal—is strictly prohibited without prior
 * written permission from the author.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 *
 * Contact: vinithbraj(at)gmail(dot)com
 */

#pragma once

extern "C"
{
#include <fltKernel.h>
#include <ntstrsafe.h>
#pragma comment(lib, "fltmgr.lib")
}

#include "VinGuardFilterKernelToUserComm.h"

namespace VinGuard
{
    class filter_callback {

    public:

        static kernel_user_com* g_filter_comm;
        static const FLT_OPERATION_REGISTRATION Callbacks[];

        static FLT_PREOP_CALLBACK_STATUS PreCreateCallback(
            PFLT_CALLBACK_DATA Data, 
            PCFLT_RELATED_OBJECTS FltObjects, 
            PVOID* CompletionContext)
        {
            UNREFERENCED_PARAMETER(CompletionContext);
            UNREFERENCED_PARAMETER(FltObjects);

            if (Data->Iopb && 
                Data->Iopb->TargetFileObject && 
                Data->Iopb->TargetFileObject->FileName.Buffer) 
            {
                DbgPrint("VinGuardFS: File open: %wZ\n", &Data->Iopb->TargetFileObject->FileName);

                if (!g_filter_comm) {
                    de("filter com object is nullptr");
                }
                else {
                    PMEMORY_STREAM out = VinGuard::memory::allocate_memory_buffer(1000);
                    filter_message _msg;
                    _msg.m_canonical_file_path = &(Data->Iopb->TargetFileObject->FileName);
                    _msg.file_action = 123;
                    _msg.filter_verdict = 456;
                    _msg.serialize(out);


                    if (!g_filter_comm)
                    {
                        de("invalid ptr = g_filter_comm, unable to communicate with user mode");
                    }
                    else 
                    {
                        g_filter_comm->send_message_sync(out->buffer, out->offset, nullptr, NULL, 100);

                    }
                }
            }

            return FLT_PREOP_SUCCESS_NO_CALLBACK;
        }
    };

    // only one callback is registered for now.
    const FLT_OPERATION_REGISTRATION filter_callback::Callbacks[] = {
         { IRP_MJ_CREATE, 0, filter_callback::PreCreateCallback, NULL },
         { IRP_MJ_OPERATION_END }
    };
}

VinGuard::kernel_user_com* VinGuard::filter_callback::g_filter_comm = nullptr;