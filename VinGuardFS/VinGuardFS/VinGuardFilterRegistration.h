#pragma once

extern "C"
{
    #include <fltKernel.h>
    #pragma comment(lib, "fltmgr.lib")
}

#include "VinGuardFilterCallback.h"
namespace VinGuard {
    class filter_registration {

    public:

        static PFLT_FILTER s_filter_handle;


        static NTSTATUS unload_filter(FLT_FILTER_UNLOAD_FLAGS Flags)
        {
            UNREFERENCED_PARAMETER(Flags);
            FltUnregisterFilter(s_filter_handle);
            return STATUS_SUCCESS;
        }

        static NTSTATUS load_filter(PDRIVER_OBJECT DriverObject)
        {
             static const FLT_REGISTRATION FilterRegistration = {
                sizeof(FLT_REGISTRATION),
                FLT_REGISTRATION_VERSION,
                0, // Flags
                NULL, // Contexts
                VinGuardFilterCallback::Callbacks,
                unload_filter,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL
            };

            auto status = FltRegisterFilter(DriverObject, &FilterRegistration, &s_filter_handle);
            return status;
        }

    };
}


PFLT_FILTER VinGuard::filter_registration::s_filter_handle = nullptr;