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
#include <fltKernel.h>

// Placement new declaration for kernel-mode
// In kernel - mode C++ (especially with the Windows kernel and WDK), 
// the default operator new() does not support placement new unless you explicitly declare it — 
// because <new> and standard C++ runtime support are not always present or usable in the kernel.
// basically telling the compiler - I'll give you memory - just run the constructor of the object 
// at this location
inline void* operator new(size_t, void* p) noexcept { return p; }
inline void operator delete(void*, void*) noexcept {} // required by some compilers

namespace VinGuard {

    template <typename T>
    static T* allocate_simple_pod_kernel_object(POOL_FLAGS poolFlags = POOL_FLAG_NON_PAGED, ULONG tag = 'VGFS')
    {
        SIZE_T size = sizeof(T);

        // create a blob of memory 
        T* ptr = reinterpret_cast<T*>(ExAllocatePool2(poolFlags, size, tag));
        if (ptr)
        {
            //clear this memory block
            RtlZeroMemory(ptr, size);

            //place the object using new 
            new (ptr) T();
        }

        return ptr;
    }

    template <typename T>
    static void free_kernel_object(T* ptr, ULONG tag = 'VGFS')
    {
        if (ptr) {
            ptr->~T();                              // Call destructor (for non-POD types)
            ExFreePoolWithTag(ptr, tag);            // Free memory with tag
        }
    }
}