//%2006////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; Symantec Corporation; The Open Group.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//==============================================================================
//
//%/////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include "Buffer.h"
#include "Pegasus/Common/InternalException.h"

PEGASUS_NAMESPACE_BEGIN

//
// Note: _empty_rep is the only BufferRep object that may have a zero capacity.
// So "_rep->cap == 0" implies "_rep == _empty_rep". But some platforms produce
// more than one instance of _empty_rep (strangely). Therefore, it is safer to
// use the former test rather than the latter.
//
BufferRep Buffer::_empty_rep =
{
    0, /* size */
    0, /* cap (zero implies it is the _empty_rep) */
    {0} /* data[0] */
};

static const Uint32 MIN_CAPACITY = 2048;

static Uint32 _next_pow_2(Uint32 x)
{
    // Check for potential overflow in x.
    PEGASUS_CHECK_CAPACITY_OVERFLOW(x);

    if (x < MIN_CAPACITY)
        return MIN_CAPACITY;

    x--;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x++;

    return x;
}

static inline BufferRep* _allocate(Uint32 cap)
{
    if (cap < MIN_CAPACITY)
        cap = MIN_CAPACITY;

    // Allocate an extra byte for null-termination performed by getData().
    BufferRep* rep = (BufferRep*)malloc(sizeof(BufferRep) + cap + 1);

    if (!rep)
    {
        throw PEGASUS_STD(bad_alloc)();
    }
    rep->cap = cap;
    return rep;
}

static inline BufferRep* _reallocate(BufferRep* rep, Uint32 cap)
{
    // Allocate an extra byte for null-termination performed by getData().
    rep = (BufferRep*)realloc(rep, sizeof(BufferRep) + cap + 1);

    if (!rep)
    {
        throw PEGASUS_STD(bad_alloc)();
    }
    rep->cap = cap;
    return rep;
}

Buffer::Buffer(const Buffer& x)
{
    _rep = _allocate(x._rep->cap);
    memcpy(_rep->data, x._rep->data, x._rep->size);
    _rep->size = x._rep->size;
}

Buffer::Buffer(const char* data, Uint32 size)
{
    _rep = _allocate(size);
    _rep->size = size;
    memcpy(_rep->data, data, size);
}

Buffer& Buffer::operator=(const Buffer& x)
{
    if (&x != this)
    {
        if (x._rep->size > _rep->cap)
        {
            if (_rep->cap != 0)
                free(_rep);

            _rep = _allocate(x._rep->cap);
        }

        memcpy(_rep->data, x._rep->data, x._rep->size);
        _rep->size = x._rep->size;
    }
    return *this;
}

void Buffer::_reserve_aux(Uint32 cap)
{
    if (_rep->cap == 0)
    {
        _rep = _allocate(cap);
        _rep->size = 0;
    }
    else
        _rep = _reallocate(_rep, _next_pow_2(cap));
}

void Buffer::_append_char_aux()
{
    if (_rep->cap == 0)
    {
        _rep = _allocate(MIN_CAPACITY);
        _rep->size = 0;
    }
    else
    {
        // Check for potential overflow.
        PEGASUS_CHECK_CAPACITY_OVERFLOW(_rep->cap);
        _rep = _reallocate(_rep, _rep->cap ? (2 * _rep->cap) : MIN_CAPACITY);
    }
}

void Buffer::insert(Uint32 pos, const char* data, Uint32 size)
{
    if (pos > _rep->size)
        return;

    Uint32 cap = _rep->size + size;
    Uint32 rem = _rep->size - pos;

    if (cap > _rep->cap)
    {
        BufferRep* rep = _allocate(cap);
        rep->size = cap;

        memcpy(rep->data, _rep->data, pos);
        memcpy(rep->data + pos, data, size);
        memcpy(rep->data + pos + size, _rep->data + pos, rem);

        if (_rep->cap != 0)
            free(_rep);

        _rep = rep;
    }
    else
    {
        memmove(_rep->data + pos + size, _rep->data + pos, rem);
        memcpy(_rep->data + pos, data, size);
        _rep->size += size;
    }
}

void Buffer::remove(Uint32 pos, Uint32 size)
{
    if (pos + size > _rep->size)
        return;

    Uint32 rem = _rep->size - (pos + size);

    if (rem)
        memmove(_rep->data + pos, _rep->data + pos + size, rem);

    _rep->size -= size;
}

PEGASUS_NAMESPACE_END
