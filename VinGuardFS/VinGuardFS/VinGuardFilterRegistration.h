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
    #pragma comment(lib, "fltmgr.lib")
}

#include "VinGuardMem.h"
#include "VinGuardDebugMacros.h"
#include "VinGuardFilterCallback.h"
#include "VinGuardFilterKernelToUserComm.h"

// STATIC objects with CTORS will require .CRT section 
// Not permitted or unsafe in Kernel code. 
VinGuard::kernel_user_com* g_filter_comm = nullptr;

namespace VinGuard {
    class filter_registration {

    public:

        static PFLT_FILTER s_filter_handle;


        static NTSTATUS unload_filter(FLT_FILTER_UNLOAD_FLAGS Flags)
        {
            di("unload_load_filter - enter");
            UNREFERENCED_PARAMETER(Flags);

            if (g_filter_comm) {
                g_filter_comm->shutdown();
                VinGuard::free_kernel_object(g_filter_comm);
            }

            FltUnregisterFilter(s_filter_handle);
          
            di("unlaod_load_filter - exit");
            return STATUS_SUCCESS;
        }

        static NTSTATUS load_filter(PDRIVER_OBJECT DriverObject)
        {
            di("load_filter - enter");
             static const FLT_REGISTRATION FilterRegistration = {
                sizeof(FLT_REGISTRATION),
                FLT_REGISTRATION_VERSION,
                0, // Flags
                NULL, // Contexts
                VinGuard::filter_callback::Callbacks,
                unload_filter,
                NULL, //InstanceSetupCallback - Called when attached to a volume
                NULL, //InstanceQueryTearDownCallback - Called when an instance is about to be torn down
                NULL, //InstanceQueryTearDownStartCallback - Called just before teardown starts - optional
                NULL, //InstanceTeardownCompleteCallback - Called when an instance teardown is complete 
                NULL, //GenerateFileNameCallback - Only used for tunnelling or obfuscation to generate alternate filenames
                NULL, //NormalizeNameComponentCallback - Only use if names are normalized
                NULL  //NormalizeContextCleanupCallback - Cleans up Conect allocated above 
            };
             
             NTSTATUS status = FltRegisterFilter(DriverObject, &FilterRegistration, &s_filter_handle);
             if (!NT_SUCCESS(status))
             {
                 de("Failed registering filter with error : %ul", status);
                 return status;
             }

             //establish ports and callback infrastructure
             di("establish driver ports");
             g_filter_comm = VinGuard::allocate_simple_pod_kernel_object<VinGuard::kernel_user_com>();
             if (!g_filter_comm)
             {
                 de("Failed allocating memory for filter comms object");
                 return STATUS_INSUFFICIENT_RESOURCES;
             }

             status = g_filter_comm->initialize(VinGuard::filter_registration::s_filter_handle);
             if (!NT_SUCCESS(status))
             {
                 de("Failed to init filter coms: %ul", status);
                 return status;
             }

             //start filtering
             di("start filtering");
             status = FltStartFiltering(VinGuard::filter_registration::s_filter_handle);
             if (!NT_SUCCESS(status)) {
                 de("Failed to start filtering : %ul", status);
                 FltUnregisterFilter(VinGuard::filter_registration::s_filter_handle);
                 return status;
             }

             return status;
        }
    };
}


PFLT_FILTER VinGuard::filter_registration::s_filter_handle = nullptr;