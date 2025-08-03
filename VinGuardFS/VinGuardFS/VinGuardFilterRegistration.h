#pragma once

extern "C"
{
    #include <fltKernel.h>
    #pragma comment(lib, "fltmgr.lib")

    #include "VinGuardFilterCallback.h"
}

class VinGuardFSRegistration {

public:

    static PFLT_FILTER gFilterHandle;


    static NTSTATUS UnloadFilter(FLT_FILTER_UNLOAD_FLAGS Flags)
    {
        UNREFERENCED_PARAMETER(Flags);
        FltUnregisterFilter(gFilterHandle);
        return STATUS_SUCCESS;
    }

    static NTSTATUS LoadFilter(PDRIVER_OBJECT DriverObject)
    {
         static const FLT_REGISTRATION FilterRegistration = {
            sizeof(FLT_REGISTRATION),
            FLT_REGISTRATION_VERSION,
            0, // Flags
            NULL, // Contexts
            VinGuardFilterCallback::Callbacks,
            UnloadFilter,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL
        };

        auto status = FltRegisterFilter(DriverObject, &FilterRegistration, &gFilterHandle);
        return status;
    }

};

PFLT_FILTER VinGuardFSRegistration::gFilterHandle = nullptr;