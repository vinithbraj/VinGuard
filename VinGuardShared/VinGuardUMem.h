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

#include <Windows.h>
#include <string>
#include <memory>
#include <cstring>

namespace VinGuard {

    typedef unsigned char* LPBYTE;
    typedef struct _MEMORY_STREAM {
        LPBYTE buffer;
        ULONG  capacity;
        ULONG  offset;

        _MEMORY_STREAM() : buffer(nullptr), capacity(0), offset(0) {}

    } MEMORY_STREAM, * PMEMORY_STREAM;

    namespace memory
    {
        PMEMORY_STREAM allocate_memory_buffer(ULONG size) {
            auto ms = new MEMORY_STREAM();
            ms->buffer = new UCHAR[size];
            ms->capacity = size;
            ms->offset = 0;
            return ms;
        }
        void free_memory_buffer(PMEMORY_STREAM ms) {
            if (ms) {
                delete[] ms->buffer;
                delete ms;
            }
        }
        void zero_memory_buffer(PMEMORY_STREAM ms) {
            if (ms && ms->buffer) 
            {
                std::memset(ms->buffer, 0, ms->capacity);
                ms->offset = 0;
            }
        }
    }
}