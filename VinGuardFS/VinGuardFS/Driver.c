#pragma once 
#include "VinGuardFilterRegistration.h"
#include "VinGuardFilterCallback.h"
#include "VinGuardFilterKernelToUserComm.h"

extern "C"
NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS status;

    status = VinGuard::filter_registration::load_filter(DriverObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = FltStartFiltering(VinGuard::filter_registration::s_filter_handle);
    if (!NT_SUCCESS(status)) {
        FltUnregisterFilter(VinGuard::filter_registration::s_filter_handle);
    }

    return status;
}
