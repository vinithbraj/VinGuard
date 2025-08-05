//$------------------------------------------------------------------------
//$-Slacksoft Copyright header. 2007
//$------------------------------------------------------------------------
//$-Product : Slacksoft binary serializer.
//$-Date:
//$-Author:
//$-Description:
//$------------------------------------------------------------------------
//$-You are not allowed to modify this source code without prior written 
//$-permission from the author. 
//$------------------------------------------------------------------------


#pragma once

#include <fstream>
#include <vector>
#include <sstream>
#include <bitset>


#define SERIALIZE_MAP_BEGIN() virtual void generate_blocks() \
{\
	_blocks.clear();

#define SERIALIZE_MAP_ENTRY_ATL_CSTRING(_data)\
{\
	VinGuard::block _block((LPVOID)&_data,0,&VinGuard::functors::out::_s_cstring,&VinGuard::functors::in::_d_cstring);\
	_blocks.push_back(_block);\
}

#define SERIALIZE_MAP_ENTRY_ATL_CSTRING_VECTOR(_data)\
{\
	VinGuard::block _block((LPVOID)&_data,0,&VinGuard::functors::out::_s_cstring_vector,&VinGuard::functors::in::_d_cstring_vector);\
	_blocks.push_back(_block);\
}

#define SERIALIZE_MAP_ENTRY_STD_STRING(_data)\
{\
	VinGuard::block _block((LPVOID)&_data,0,&VinGuard::functors::out::_s_stdstring,&VinGuard::functors::in::_d_stdstring);\
	_blocks.push_back(_block);\
}

#define SERIALIZE_MAP_ENTRY_POD(_type,_data)\
{\
	VinGuard::block _block((LPVOID)&_data,0,&VinGuard::functors::out::_s_pod<_type>,&VinGuard::functors::in::_d_pod<_type>);\
	_blocks.push_back(_block);\
}

#define SERIALIZE_MAP_ENTRY_BYTE( _type, _data, _size )\
{\
	VinGuard::block _block((LPVOID)&_data, _type, _size,VinGuard::functors::out::_s_byte,VinGuard::functors::out::_d_byte);\
	_blocks.push_back(_block);\
}

#define SERIALIZE_MAP_END() \
    _num_blocks = _blocks.size();\
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

namespace bitmask
{
    const UINT MASK_SIZE = 16;
    enum { initialized = 0 };
}

namespace config
{
    const UINT heap_size = 1024;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

namespace VinGuard
{

    namespace functors
    {
        struct out
        {
            static void _s_char
            (std::iostream& stream, LPVOID p, UINT size)
            {
                ATLASSERT(size);
                stream.write((const char*)&size, sizeof(UINT) / sizeof(char));
                stream.write((const char*)p, size);
            }

            static void _s_cstring
            (std::iostream& stream, LPVOID p, UINT size)
            {
                ATL::CString* _pc = (ATL::CString*)p;
                ATLASSERT(_pc);
                ATL::CStringA _ca = ATL::CT2A(_pc->GetString(), CP_UTF8);
                size = _ca.GetLength();
                _s_char(stream, (LPVOID)(_ca.GetString()), size);
            }

            static void _s_cstring_vector
            (std::iostream& stream, LPVOID p, UINT size)
            {
                std::vector<ATL::CString>* _pc = (std::vector<ATL::CString>*) p;
                ATLASSERT(_pc);
                std::for_each(std::begin(*_pc), std::end(*_pc), std::bind(&functors::out::_s_cstring, std::ref(stream), &(std::placeholders::_1), 0));
            }

            static void _s_stdstring
            (std::iostream& stream, LPVOID p, UINT size)
            {
                std::string* _pc = (std::string*)p;
                ATLASSERT(_pc);
                size = _pc->length();
                _s_char(stream, (LPVOID)(_pc->c_str()), size);
            }

            template<typename pod>
            static void _s_pod
            (std::iostream& stream, LPVOID p, UINT size)
            {
                pod* _pt = (pod*)p;
                ATLASSERT(_pt);
                stream.write((const char*)_pt, sizeof(pod) / sizeof(char));
            }

            static void _s_byte
            (std::iostream& stream, LPVOID p, UINT size)
            {
                ATLASSERT(p);
                stream.write((LPCSTR)p, size);
            }

            typedef void (*callback)(std::iostream&, LPVOID, UINT);
        };

        struct in
        {
            static char* _d_char
            (std::iostream& stream, LPVOID p, UINT size, LPVOID _heap, UINT _heap_size)
            {
                size = 0;
                stream.read((CHAR*)&size, sizeof(int) / sizeof(CHAR));
                ATLASSERT(_heap_size > size);

                char* _pc = NULL;

                if (_heap_size > size) {
                    ZeroMemory(_heap, _heap_size);
                    stream.read((char*)_heap, size);
                    _pc = (char*)_heap;
                }

                return _pc;
            }

            static void _d_cstring
            (std::iostream& stream, LPVOID p, UINT size, LPBYTE _heap, UINT _heap_size)
            {
                ATLASSERT(p);
                char* _pc = _d_char(stream, 0, 0, _heap, _heap_size);
                if (_pc) {
                    *((CString*)(p)) = ATL::CA2T(_pc);
                }
            }

            static void _d_cstring_vector
            (std::iostream& stream, LPVOID p, UINT size, LPBYTE _heap, UINT _heap_size)
            {
                std::vector<ATL::CString>* _pc = (std::vector<ATL::CString>*) p;
                ATLASSERT(_pc);
                std::for_each(std::begin(*_pc), std::end(*_pc), std::bind(&functors::in::_d_cstring, std::ref(stream), &(std::placeholders::_1), 0, _heap, _heap_size));
            }

            static void _d_stdstring
            (std::iostream& stream, LPVOID p, UINT size, LPBYTE _heap, UINT _heap_size)
            {
                ATLASSERT(p);
                char* _pc = _d_char(stream, 0, 0, _heap, _heap_size);
                if (_pc) {
                    *((std::string*)(p)) = _pc;
                }
            }

            static void _d_byte
            (std::iostream& stream, LPVOID p, UINT size, LPBYTE _heap, UINT _heap_size)
            {
                ATLASSERT(p);
                stream.read((char*)p, size);
            }

            template<typename pod>
            static void _d_pod
            (std::iostream& stream, LPVOID p, UINT size, LPBYTE _heap, UINT _heap_size)
            {
                ATLASSERT(p);
                stream.read((char*)(p), sizeof(pod) / sizeof(char));
            }

            typedef void (*callback)(std::iostream&, LPVOID, UINT, LPBYTE, UINT);
        };
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    struct block
    {
    public:
        block() :_p(0), _size(0), _s(0), _d(0) {}
        block(
            LPVOID _ip,
            DWORD _isize,
            functors::out::callback _is,
            functors::in::callback _id) :
            _p(_ip), _size(_isize), _s(_is), _d(_id) {
        }

        LPVOID _p;
        DWORD _size;
        functors::out::callback _s;
        functors::in::callback  _d;
    };

    typedef std::vector<block> blocks;

    namespace functors
    {
        struct exec
        {
            static void out(block b, std::iostream& stream) {
                b._s(stream, b._p, b._size);
            }

            static void in(block b, std::iostream& stream, LPBYTE _heap, UINT _heap_size) {
                if (!stream.eof())
                    b._d(stream, b._p, b._size, _heap, _heap_size);
            }
        };
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    class serializer
    {
    public:

        //allocation in ctor, probably not so good...
        serializer() {
            _heap = (LPBYTE)malloc(config::heap_size);
        }

        virtual ~serializer() {
            if (_heap)
                free(_heap);
        }

        virtual void generate_blocks() = 0;

        bool serialize(LPCTSTR file)
        {
            bool r = false;
            if (_num_blocks) {
                std::fstream stream;
                stream.open(file, std::ios_base::out | std::ios_base::app | std::ios_base::binary);
                r = stream.is_open();
                ATLASSERT(r);
                if (r) {
                    std::for_each(std::begin(_blocks), std::end(_blocks),
                        std::bind(&functors::exec::out, std::placeholders::_1, std::ref(stream)));
                    stream.close();
                }
            }
            return r;
        }

        bool serialize(std::iostream& stream)
        {
            bool r = false;
            if (_num_blocks) {
                std::for_each(std::begin(_blocks), std::end(_blocks),
                    std::bind(&functors::exec::out, std::placeholders::_1, std::ref(stream)));
                r = true;
            }
            return r;
        }

        UINT serialize(LPBYTE buffer, UINT size)
        {
            bool r = false;
            UINT c = 0;
            ATLASSERT(buffer);
            ATLASSERT(size);

            if (buffer && size) {
                std::stringbuf stm;
                std::iostream iostm(&stm);
                r = serialize(iostm);
                ATLASSERT(r);

                if (r) {
                    const UINT stream_size = stm.str().length();
                    ATLASSERT(size > stream_size);
                    if (stream_size > size)
                        r = false;
                    else {
                        ZeroMemory(buffer, sizeof(BYTE) * size);
                        memcpy_s(buffer, size, stm.str().c_str(), stream_size);
                        c = stream_size;
                    }
                }
            }

            return c;
        }

        bool deserialize(LPBYTE buffer, UINT size)
        {
            bool r = false;
            ATLASSERT(buffer);
            ATLASSERT(size);

            if (buffer && size) {
                std::stringbuf stm;
                std::iostream iostm(&stm);
                iostm.write((char*)buffer, size);
                r = deserialize(iostm);
            }

            return r;
        }

        bool deserialize(LPCTSTR file)
        {
            bool r = false;
            if (_num_blocks) {
                if (0xFFFFFFFF == ::GetFileAttributes(file)) {
                    ATLASSERT(r);
                }
                else
                {
                    std::fstream stream;
                    stream.open(file, std::ios_base::in | std::ios_base::binary);
                    r = stream.is_open();
                    ATLASSERT(r);
                    if (r) {
                        std::for_each(std::begin(_blocks), std::end(_blocks),
                            std::bind(&functors::exec::in, std::placeholders::_1, std::ref(stream), _heap, config::heap_size));
                        r = true;
                    }
                }
            }
            return r;
        }

        bool deserialize(std::iostream& stream)
        {
            bool r = false;
            if (_num_blocks) {
                std::for_each(std::begin(_blocks), std::end(_blocks),
                    std::bind(&functors::exec::in, std::placeholders::_1, std::ref(stream), _heap, config::heap_size));
                r = true;
            }
            return r;
        }

        void clear(LPCTSTR file) {
            ::DeleteFile(file);
        }

        bool stream_out(LPCTSTR file, std::fstream& stream) {
            return _stream<std::ios_base::out | std::ios_base::app | std::ios_base::binary>(file, stream);
        }

        bool stream_in(LPCTSTR file, std::fstream& stream) {
            return _stream<std::ios_base::in | std::ios_base::binary>(file, stream);
        }

    protected:

        template<int t> bool _stream(LPCTSTR file, std::fstream& stream)
        {
            bool r = false;
            ATLASSERT(file);

            if (file) {
                stream.open(file, t);
                r = stream.is_open();
            }
            return r;
        }

        LPBYTE _heap;
        blocks _blocks;
        UINT _num_blocks;
    };
}