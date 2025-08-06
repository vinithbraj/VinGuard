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

ULONG tag = 'VGFS';

// Placement new declaration for kernel-mode
// In kernel - mode C++ (especially with the Windows kernel and WDK), 
// the default operator new() does not support placement new unless you explicitly declare it — 
// because <new> and standard C++ runtime support are not always present or usable in the kernel.
// basically telling the compiler - I'll give you memory - just run the constructor of the object 
// at this location
//void* operator new(size_t size) noexcept;
//void operator delete(void* p);
 void* __cdecl operator new(size_t size) noexcept {
    // create a blob of memory 
    void* ptr = ExAllocatePool2(POOL_FLAG_NON_PAGED, size, tag);
    if (ptr)
        RtlZeroMemory(ptr, size);

    return ptr;
}

 void __cdecl operator delete(void* p, size_t size) noexcept {
    UNREFERENCED_PARAMETER(size);
    if (p)
        ExFreePoolWithTag(p, tag);
}

namespace VinGuard {

    typedef struct _MEMORY_STREAM {
        PUCHAR buffer;     // Write buffer
        ULONG  capacity;   // Total size of the buffer
        ULONG  offset;     // Current write position

        _MEMORY_STREAM() :buffer(nullptr), capacity(NULL), offset(NULL) {}
    } MEMORY_STREAM, * PMEMORY_STREAM;


    class memory
    {
    public :
        template <typename T>
        static T* allocate_simple_pod_kernel_object()
        {
            T* ptr = new T();
            return ptr;
        }

        template <typename T>
        static void free_kernel_object(T* ptr)
        {
            delete ptr;
        }

        static PMEMORY_STREAM allocate_memory_buffer(ULONG capacity)
        {
            PMEMORY_STREAM stream = (PMEMORY_STREAM)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(MEMORY_STREAM), tag);
            if (!stream)
                return nullptr;

            stream->buffer = (PUCHAR)ExAllocatePool2(POOL_FLAG_NON_PAGED, capacity, tag);
            if (!stream->buffer) {
                ExFreePoolWithTag(stream, tag);
                return nullptr;
            }

            stream->capacity = capacity;
            stream->offset = 0;
            return stream;
        }

        static void free_memory_buffer(PMEMORY_STREAM stream)
        {
            if (!stream)
                return;

            if (stream->buffer) {
                ExFreePoolWithTag(stream->buffer, tag); // Use same tag as allocation
                stream->buffer = nullptr;
            }

            ExFreePoolWithTag(stream, tag);
        }

        static void zero_memory_buffer(PMEMORY_STREAM stream)
        {
            if (!stream)
                return;
            RtlZeroMemory(stream->buffer, stream->capacity);
        }
    };

    struct auto_unicode_string
    {
        ~auto_unicode_string() {
            release_str();
        }
        auto_unicode_string() {
            RtlZeroMemory(&_str, sizeof(_str));
        }
        UNICODE_STRING _str;
        void release_str() {
            if (_str.Buffer && _str.Length && _str.MaximumLength)
            {
                RtlFreeUnicodeString(&_str);
                RtlZeroMemory(&_str, sizeof(_str));
            }

        }
        void copy_in(const PUNICODE_STRING _in_str) {
            if (_in_str) {
                release_str();
                NTSTATUS status = RtlDuplicateUnicodeString(
                    RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                    _in_str,
                    &_str);

                if (!NT_SUCCESS(status)) {
                    RtlZeroMemory(&_str, sizeof(_str)); 
                }
            }
        }

        auto_unicode_string& operator=(const PUNICODE_STRING in_str) {
            copy_in(in_str);
            return *this;
        }
    };
}