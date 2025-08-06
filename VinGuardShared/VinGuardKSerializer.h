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
#include "VinGuardDebugMacros.h"

typedef unsigned char* LPBYTE;
            
#define SERIALIZE_INIT() generate_blocks();
#define SERIALIZE_MAP_BEGIN() virtual void generate_blocks() \
{

#define SERIALIZE_MAP_ENTRY_POD(_type,_data)\
{\
	VinGuard::block _block(\
            (PVOID)&_data,\
            0,\
            &VinGuard::functors::out::_s_pod<_type>,\
            &VinGuard::functors::in::_d_pod<_type>);\
	        _blocks.push(_block);\
}

#define SERIALIZE_MAP_ENTRY_BYTE(_data, _size )\
{\
	VinGuard::block _block(\
            (PVOID)_data, \
            _size,\
            VinGuard::functors::out::_s_byte,\
            VinGuard::functors::in::_d_byte);\
	        _blocks.push(_block);\
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

#define SERIALIZE_MAP_END() \
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

namespace config
{
    const ULONG heap_size = 1024;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

namespace VinGuard
{
    bool memory_stream_writer(PMEMORY_STREAM stream, const PVOID in_data, ULONG size)
    {
        if (!stream || !in_data || stream->offset + size > stream->capacity)
        {
            return FALSE;
        }
        RtlCopyMemory(
            stream->buffer + stream->offset, 
            in_data, 
            size);

        stream->offset += size;
        return TRUE;
    }

    bool memory_stream_reader(PMEMORY_STREAM stream, const PVOID out_data, ULONG size)
    {
        if (!stream || !out_data || stream->offset + size > stream->capacity)
            return FALSE;

        RtlCopyMemory(out_data, stream->buffer+stream->offset, size);
        stream->offset += size;
        return TRUE;
    }

namespace functors
{
    struct out
    {
	    static void _s_char          
            (PMEMORY_STREAM stream, PVOID p, ULONG size) 
	    {
            size = reinterpret_cast<PUNICODE_STRING>(p)->Length;
            memory_stream_writer(stream, &size, sizeof(ULONG));
            memory_stream_writer(stream, 
                reinterpret_cast<PUNICODE_STRING>(p)->Buffer, 
                reinterpret_cast<PUNICODE_STRING>(p)->Length);
	    }

        template<typename pod>
	    static void _s_pod           
            (PMEMORY_STREAM stream, PVOID p, ULONG size)
	    {
            UNREFERENCED_PARAMETER(size); 
            memory_stream_writer(stream, p, sizeof(pod));
	    }

	    static void _s_byte          
            (PMEMORY_STREAM stream, PVOID p, ULONG size)
	    {
            memory_stream_writer(stream, p, size);
	    }

        typedef void (*callback)(PMEMORY_STREAM, PVOID, ULONG);
    };

    struct in
    {
	    static void _d_char    
            (PMEMORY_STREAM stream, PVOID p, PULONG size, PMEMORY_STREAM _heap)
        {
            UNREFERENCED_PARAMETER(p);

            *size = 0;

            //read the size first
            if (!memory_stream_reader(stream, size, sizeof(ULONG)))
                return;

            if (*size > _heap->capacity)
                return;

            VinGuard::memory::zero_memory_buffer(_heap);

            if (!memory_stream_reader(stream, _heap->buffer, *size))
                return;

            UNICODE_STRING _ts;
            _ts.Length = (USHORT)(*size);
            _ts.MaximumLength = (USHORT)(*size);
            _ts.Buffer = (PWSTR)_heap->buffer;
            
            NTSTATUS status = RtlDuplicateUnicodeString(
                RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                &_ts,
                reinterpret_cast<PUNICODE_STRING>(p));

            if (!NT_SUCCESS(status))
                return;
	    }

    	static void _d_byte          
            (PMEMORY_STREAM stream, PVOID p, PULONG size, PMEMORY_STREAM _heap)
	    {
            UNREFERENCED_PARAMETER(_heap);
            memory_stream_reader(stream, p, *size);
	    }
	
        template<typename pod>
	    static void _d_pod           
            (PMEMORY_STREAM stream, PVOID p, PULONG size, PMEMORY_STREAM _heap)
	    {
            UNREFERENCED_PARAMETER(size);
            UNREFERENCED_PARAMETER(_heap);
            UNREFERENCED_PARAMETER(_heap_size);

            memory_stream_reader(stream, p, sizeof(pod));
	    }

        typedef void (*callback)(PMEMORY_STREAM, PVOID, PULONG, PMEMORY_STREAM);
    };
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

    struct block
	{
	public:
		block():_p(0),_size(0),_s(0),_d(0){
        
        
        }
        block(
            PVOID _ip,
            ULONG _isize,
            functors::out::callback _is,
            functors::in::callback _id) :
            _p(_ip), _size(_isize), _s(_is), _d(_id) {
        }

		PVOID _p;
        ULONG _size;
        functors::out::callback _s;
        functors::in::callback  _d;
	};

    #define MAX_BLOCKS 10

    typedef struct _BLOCK_LIST {
        block items[MAX_BLOCKS];
        ULONG count;
        void push(block item) {
            if (count < MAX_BLOCKS - 1) {
                items[count++] = item;
            }
        }
        _BLOCK_LIST() :count(0) {}
    } BLOCK_LIST;


    namespace functors
    {
        struct exec
        {
            static void out(block b,PMEMORY_STREAM stream){
                b._s(stream,b._p,b._size);
            }
            
            static void in(block b,PMEMORY_STREAM stream, PMEMORY_STREAM _heap){
                if (stream->offset < stream->capacity)
                {
                    b._d(stream, b._p, &b._size, _heap);
                }
            }
        };
    }

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

	class serializer
	{
	public:

		serializer(){
            _heap = VinGuard::memory::allocate_memory_buffer(config::heap_size);
        }

		virtual ~serializer(){
            VinGuard::memory::free_memory_buffer(_heap);
        }

        virtual void generate_blocks()=0;

        bool serialize(PMEMORY_STREAM stream)
        {
            if (nullptr == stream || stream->capacity == 0)
            {
                return false;
            }

            if (_blocks.count == 0)
            {
                return false;
            }

            for (ULONG i = 0; i < _blocks.count; ++i) {
                
                functors::exec::out(_blocks.items[i], stream);
            }

            return true;
        }

        bool deserialize(PMEMORY_STREAM stream)
        {
            stream->offset = 0;

            if (_blocks.count == 0)
                return false;

            for (ULONG i = 0; i < _blocks.count; ++i) {
                functors::exec::in(_blocks.items[i], stream, _heap);
            }

            return true;
        }

	protected:

        VinGuard::PMEMORY_STREAM _heap;
        BLOCK_LIST _blocks;
	};
}