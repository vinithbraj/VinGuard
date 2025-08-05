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
#include "VinGuardFilterRegistration.h"
#include "VinGuardFilterCallback.h"

extern "C"
NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
    di("enter driver main");

    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS status;

    //load and register callback 
    di("load driver filter");
    status = VinGuard::filter_registration::load_filter(DriverObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

   

    return status;
}
