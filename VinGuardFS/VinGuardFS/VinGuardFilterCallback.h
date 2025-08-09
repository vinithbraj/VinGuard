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

            /*
            * IRP = what the applications wants to do with this file
            * IO_STACK_LOCATION is where you'll find the context for this specific 
            * filter to perform an action and is morphed/updated based on the 
            * registrations requested by the filter. 
            * FilterManager takes care of crafting the specific IRP view for 
            * this filter and is provied in Iopb structure below 
            * IO_STACK_LOCATION is more of a legacy structure.
            */
            if (Data->Iopb && 
                Data->Iopb->TargetFileObject && 
                Data->Iopb->TargetFileObject->FileName.Buffer) 
            {
                DbgPrint("VinGuardFS: File open: %wZ\n", &Data->Iopb->TargetFileObject->FileName);

                if (!g_filter_comm) {
                    de("filter com object is nullptr");
                }
                else {
                   // PMEMORY_STREAM out = VinGuard::memory::allocate_memory_buffer(1000);
                    
                   // filter_message _msg;
                    //_msg.copy_into(Data);
                    //_msg.serialize(out);
                    
                    if (!g_filter_comm)
                    {
                        de("invalid ptr = g_filter_comm, unable to communicate with user mode");
                    }
                    else 
                    {
                       // g_filter_comm->send_message_sync(out->buffer, out->offset, nullptr, NULL, 100);

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