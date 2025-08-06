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
#include "VinGuardUMem.h"

#define SERIALIZE_INIT() generate_blocks();
#define SERIALIZE_MAP_BEGIN() virtual void generate_blocks() override {
#define SERIALIZE_MAP_ENTRY_POD(_type,_data) \
    { \
        VinGuard::block _block( \
            (PVOID)&_data, \
            0, \
            &VinGuard::functors::out::_s_pod<_type>, \
            &VinGuard::functors::in::_d_pod<_type>); \
        _blocks.push(_block); \
    }
#define SERIALIZE_MAP_ENTRY_BYTE(_data, _size) \
    { \
        VinGuard::block _block( \
            (PVOID)_data, \
            _size, \
            VinGuard::functors::out::_s_byte, \
            VinGuard::functors::in::_d_byte); \
        _blocks.push(_block); \
    }

#define SERIALIZE_MAP_ENTRY_CHAR(_data, _size )\
{\
	VinGuard::block _block(\
            (PVOID)_data, \
            _size,\
            VinGuard::functors::out::_s_char,\
            VinGuard::functors::in::_d_char);\
	        _blocks.push(_block);\
}

#define SERIALIZE_MAP_END() }

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

namespace config
{
    const ULONG heap_size = 1024;
}

#define MAX_BLOCKS 10

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

namespace VinGuard {
    bool memory_stream_writer(PMEMORY_STREAM stream, const PVOID in_data, ULONG size)
    {
        if (!stream || !in_data || stream->offset + size > stream->capacity)
            return false;
        
        std::memcpy(stream->buffer + stream->offset, in_data, size);
        stream->offset += size;
        return true;
    }

    bool memory_stream_reader(PMEMORY_STREAM stream, PVOID out_data, ULONG size) {
        if (!stream || !out_data || stream->offset + size > stream->capacity)
            return false;
        
        std::memcpy(out_data, stream->buffer + stream->offset, size);
        stream->offset += size;
        return true;
    }

    namespace functors {
        struct out {
            static void _s_char
                (PMEMORY_STREAM stream, PVOID p, ULONG size)
            {
                size = (ULONG)strlen((char*)p);
                memory_stream_writer(stream, &size, sizeof(ULONG));
                memory_stream_writer(stream, p, size);
            }

            template<typename pod>
            static void _s_pod(PMEMORY_STREAM stream, PVOID p, ULONG size)
            {
                memory_stream_writer(stream, p, sizeof(pod));
            }

            static void _s_byte(PMEMORY_STREAM stream, PVOID p, ULONG size)
            {
                memory_stream_writer(stream, p, size);
            }

            typedef void (*callback)(PMEMORY_STREAM, PVOID, ULONG);
        };

        struct in {
            static void _d_char(PMEMORY_STREAM stream, PVOID p, PULONG size, PMEMORY_STREAM _heap)
            {
                *size = 0;
                if (!memory_stream_reader(stream, size, sizeof(ULONG)))
                {
                    return;
                }

                wprintf(L"stringlen = %u\n", *size);

                if (*size > _heap->capacity)
                {
                    return;
                }
                
                memory::zero_memory_buffer(_heap);

                if (!memory_stream_reader(stream, _heap->buffer, (*size)*sizeof(wchar_t)))
                {
                    return;
                }
                                
                std::memcpy(p, _heap->buffer, *size);
            }

            static void _d_byte(PMEMORY_STREAM stream, PVOID p, PULONG size, PMEMORY_STREAM _heap)
            {
                (void)_heap;
                memory_stream_reader(stream, p, *size);
            }

            template<typename pod>
            static void _d_pod(PMEMORY_STREAM stream, PVOID p, PULONG size, PMEMORY_STREAM _heap)
            {
                (void)size;
                (void)_heap;
                memory_stream_reader(stream, p, sizeof(pod));
            }

            typedef void (*callback)(PMEMORY_STREAM, PVOID, PULONG, PMEMORY_STREAM);
        };
    }

    struct block {
        block() : _p(nullptr), _size(0), _s(nullptr), _d(nullptr) {}
        block(PVOID p, ULONG size, functors::out::callback s, functors::in::callback d)
            : _p(p), _size(size), _s(s), _d(d) {
        }
        PVOID _p;
        ULONG _size;
        functors::out::callback _s;
        functors::in::callback  _d;
    };

    typedef struct _BLOCK_LIST {
        block items[MAX_BLOCKS];
        ULONG count;
        void push(block item) {
            if (count < MAX_BLOCKS)
                items[count++] = item;
        }
        _BLOCK_LIST() : count(0) {}
    } BLOCK_LIST;


    namespace functors
    {
        struct exec
        {
            static void out(block b, PMEMORY_STREAM stream)
            {
                b._s(stream, b._p, b._size);
            }

            static void in(block b, PMEMORY_STREAM stream, PMEMORY_STREAM _heap)
            {
                if (stream->offset < stream->capacity)
                    b._d(stream, b._p, &b._size, _heap);
            }
        };
    }

    class serializer {
    public:
        serializer() 
        {
            _heap = memory::allocate_memory_buffer(config::heap_size);
        }
        
        virtual ~serializer()
        {
            memory::free_memory_buffer(_heap);
        }

        virtual void generate_blocks() = 0;

        bool serialize(PMEMORY_STREAM stream)
        {
            if (!stream || stream->capacity == 0 || _blocks.count == 0)
                return false;
            
            for (ULONG i = 0; i < _blocks.count; ++i)
                functors::exec::out(_blocks.items[i], stream);
            
            return true;
        }

        bool deserialize(PMEMORY_STREAM stream)
        {
            if (_blocks.count == 0)
            {
                return false;
            }

            stream->offset = 0;
            
            for (ULONG i = 0; i < _blocks.count; ++i)
            {
                functors::exec::in(_blocks.items[i], stream, _heap);
            }
            
            return true;
        }

    protected:
        PMEMORY_STREAM _heap;
        BLOCK_LIST _blocks;
    };
}
