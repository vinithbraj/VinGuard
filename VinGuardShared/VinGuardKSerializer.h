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

#define SERIALIZE_MAP_BEGIN() virtual void generate_blocks() \
{

#define SERIALIZE_MAP_ENTRY_STD_STRING(_data)\
{\
	VinGuard::block _block((PVOID)&_data,0,&VinGuard::functors::out::_s_stdstring,&VinGuard::functors::in::_d_stdstring);\
	_blocks.push(_block);\
}

#define SERIALIZE_MAP_ENTRY_POD(_type,_data)\
{\
	VinGuard::block _block((PVOID)&_data,0,&VinGuard::functors::out::_s_pod<_type>,&VinGuard::functors::in::_d_pod<_type>);\
	_blocks.push(_block);\
}

#define SERIALIZE_MAP_ENTRY_BYTE( _type, _data, _size )\
{\
	VinGuard::block _block((PVOID)&_data, _type, _size,VinGuard::functors::out::_s_byte,VinGuard::functors::out::_d_byte);\
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

    typedef struct _MEMORY_STREAM {
        PUCHAR buffer;     // Write buffer
        ULONG  capacity;   // Total size of the buffer
        ULONG  offset;     // Current write position
    } MEMORY_STREAM, * PMEMORY_STREAM;


    bool memory_stream_writer(PMEMORY_STREAM stream, const PVOID in_data, ULONG size)
    {
        if (!stream || !in_data || stream->offset + size > stream->capacity)
            return FALSE;

        RtlCopyMemory(stream->buffer + stream->offset, in_data, size);
        stream->offset += size;
        return TRUE;
    }

    bool memory_stream_reader(PMEMORY_STREAM stream, PVOID out_data, ULONG size)
    {
        if (!stream || !out_data || stream->offset + size > stream->capacity)
            return FALSE;

        RtlCopyMemory(out_data, stream->buffer + stream->offset, size);
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
            // Write the size first
            memory_stream_writer(stream, &size, sizeof(ULONG));
            // Write the actual data
            memory_stream_writer(stream, p, size);
	    }

        template<typename pod>
	    static void _s_pod           
            (PMEMORY_STREAM stream, PVOID p, ULONG size)
	    {
            UNREFERENCED_PARAMETER(size); // size not used here
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
	    static char* _d_char         
            (PMEMORY_STREAM stream, PVOID p, ULONG size, PVOID _heap, ULONG _heap_size)
	    {
            UNREFERENCED_PARAMETER(p);
            size = 0;

            //read the size first
            if (!memory_stream_reader(stream, &size, sizeof(ULONG)))
                return NULL;

            if (size > _heap_size)
                return NULL;

            RtlZeroMemory(_heap, _heap_size);

            //read the actual data next
            if (!memory_stream_reader(stream, _heap, size))
                return NULL;

            return (char*)_heap;
	    }

    	static void _d_byte          
            (PMEMORY_STREAM stream, PVOID p, ULONG size, LPBYTE _heap, ULONG _heap_size)
	    {
            UNREFERENCED_PARAMETER(_heap);
            UNREFERENCED_PARAMETER(_heap_size);

            memory_stream_reader(stream, p, size);
	    }
	
        template<typename pod>
	    static void _d_pod           
            (PMEMORY_STREAM stream, PVOID p, ULONG size, LPBYTE _heap, ULONG _heap_size)
	    {
            UNREFERENCED_PARAMETER(size);
            UNREFERENCED_PARAMETER(_heap);
            UNREFERENCED_PARAMETER(_heap_size);

            memory_stream_reader(stream, p, sizeof(pod));
	    }

        typedef void (*callback)(PMEMORY_STREAM, PVOID, ULONG, LPBYTE, ULONG);
    };
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

    struct block
	{
	public:
		block():_p(0),_size(0),_s(0),_d(0){}
        block(
            PVOID _ip, 
            ULONG _isize,
            functors::out::callback _is, 
            functors::in::callback _id):
            _p(_ip),_size(_isize),_s(_is),_d(_id){}

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
            
            static void in(block b,PMEMORY_STREAM stream, LPBYTE _heap, ULONG _heap_size){
                if (stream->offset < stream->capacity)
                {
                    b._d(stream, b._p, b._size, _heap, _heap_size);
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
            _heap = (PUCHAR)ExAllocatePool2(NonPagedPoolNx, config::heap_size, 'vght');
        }

		virtual ~serializer(){
            if(_heap)
                ExFreePoolWithTag(_heap, 'vght');
        }

        virtual void generate_blocks()=0;

        bool serialize(PMEMORY_STREAM stream)
        {
            if (_blocks.count == 0)
                return false;

            for (ULONG i = 0; i < _blocks.count; ++i){
                functors::exec::out(_blocks.items[i], stream);
            }

            return true;
        }

        bool deserialize(PMEMORY_STREAM& stream)
        {
            if (_blocks.count == 0)
            if (_blocks.count == 0)
                return false;

            for (ULONG i = 0; i < _blocks.count; ++i) {
                functors::exec::in(_blocks.items[i], stream, _heap, config::heap_size);
            }

            return true;
        }

	protected:

        LPBYTE _heap;
        BLOCK_LIST _blocks;
	};
}