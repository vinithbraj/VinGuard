#pragma once 
#include "VinGuardFilterRegistration.h"
#include "VinGuardFilterCallback.h"

extern "C"
NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS status;

    status = VinGuardFSRegistration::LoadFilter(DriverObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = FltStartFiltering(VinGuardFSRegistration::gFilterHandle);
    if (!NT_SUCCESS(status)) {
        FltUnregisterFilter(VinGuardFSRegistration::gFilterHandle);
    }

    return status;
}
