//%2005////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
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
// Author: Mike Day (mdday@us.ibm.com)
//
// Modified By: Amit K Arora, IBM (amita@in.ibm.com) for PEP#101
//              Alagaraja Ramasubramanian (alags_raj@in.ibm.com) for Bug#1090
//              Amit K Arora, IBM (amita@in.ibm.com) for Bug#2322
//              David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_DQueue_h
#define Pegasus_DQueue_h

#include <Pegasus/Common/IPC.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_NAMESPACE_BEGIN

template<class L> class DQueue : public internal_dq
{
public:
    static void *operator new(size_t size);
    static void operator delete(void *dead, size_t size);

private:
    AutoPtr<Mutex> _mutex; //PEP101
    AutoPtr<AtomicInt> _actual_count;
    DQueue *_dq_next;
    static DQueue<L> *_headOfFreeList;
    static const int BLOCK_SIZE;
    static Mutex _alloc_mut;

public:
    typedef internal_dq Base;
    DQueue(void);
    DQueue(Boolean head);

    virtual ~DQueue();

    void lock(void) throw(IPCException);
    void unlock(void) throw(IPCException);
    void try_lock() throw(IPCException);

    void insert_first_no_lock(L *element) throw(IPCException);
    void insert_first(L *element) throw(IPCException);
    void insert_last_no_lock(L *element) throw(IPCException);
    void insert_last(L *element) throw(IPCException);
    void empty_list( void ) throw(IPCException);
    L *remove_first ( void ) throw(IPCException);
    L *remove_last ( void ) throw(IPCException);
    L *remove_no_lock(const void *key) throw(IPCException);
    L *remove_no_lock(const L *key) throw(IPCException);
    L *remove(const void *key) throw(IPCException);
    L *remove(const L *key) throw(IPCException);
    L *reference(const void *key) throw(IPCException);
    L *reference(const L *key);
    L *next( const void * ref) throw(IPCException);
    L *prev( const void *ref) throw(IPCException);
    Boolean exists(const void *key) throw(IPCException);
    Boolean exists(const L *key) throw(IPCException);

    Uint32 count(void) { return _actual_count->value() ; }
    Uint32 size(void) { return _actual_count->value() ; }
} ;



template<class L> class AsyncDQueue: public internal_dq
{

public:
    static void * operator new(size_t size);
    static void operator delete(void *, size_t);

private: // asyncdqueue
    AutoPtr<Mutex> _cond;
    AutoPtr<Condition> _slot;
    AutoPtr<Condition> _node;
    AutoPtr<AtomicInt> _actual_count;
    AutoPtr<AtomicInt> _disallow;
    AutoPtr<AtomicInt> _capacity;
    AsyncDQueue *_dq_next;

    static AsyncDQueue *_headOfFreeList;
    static const int BLOCK_SIZE;
     static Mutex _alloc_mut;

    void _insert_prep(void) throw(IPCException);
    void _insert_recover(void) throw(IPCException);
    void _unlink_prep(void) throw(IPCException);
    void _unlink_recover(void) throw(IPCException);
    L *_remove_no_lock(const void *key) throw(IPCException);
    L *_remove_no_lock(const L *key) throw(IPCException);

public:
    typedef internal_dq Base;

    AsyncDQueue(void) ;
    AsyncDQueue(Boolean head, Uint32 capacity );
    virtual ~AsyncDQueue(void);
    void shutdown_queue(void);
    Boolean is_full(void);
    Boolean is_empty(void);
    Boolean is_shutdown(void);
    void try_lock(PEGASUS_THREAD_TYPE myself) throw(IPCException);
    void lock(PEGASUS_THREAD_TYPE myself) throw(IPCException);
    void unlock(void);
    void signal_slot(void) throw(IPCException);
    void signal_node(void) throw(IPCException);
    Condition *get_node_cond(void);
    Condition *get_slot_cond(void);
    void wait_for_node(void) throw(IPCException);
    void set_capacity(Uint32 capacity) throw(IPCException);
    Uint32 get_capacity(void);
    void insert_first(L *element) throw(IPCException);
    void insert_first_wait(L *element) throw(IPCException);
    void insert_last(L *element) throw(IPCException);
    void insert_last_wait(L *element) throw(IPCException);
    void empty_list(void) throw(IPCException);
    L *remove_first(void) throw(IPCException);
    L *remove_first_wait(void) throw(IPCException);
    L *remove_last(void) throw(IPCException);
    L *remove_last_wait(void) throw(IPCException);
    L *remove(const void *key) throw(IPCException);
    L *remove(const L *key) throw(IPCException);
    L *remove_no_lock(const void *key) throw(IPCException);
    L *remove_no_lock(const L *key) throw(IPCException);
    L *remove_wait(const void *key) throw(IPCException);
    L *next(const L *ref) throw(IPCException);
    L *prev(const L *ref) throw(IPCException);
    L *reference(const void *key) throw(IPCException);
    L *reference(const L *key) throw(IPCException);
    Uint32 count(void) ;
    Uint32 size(void) ;
};

template<class L> DQueue<L> * DQueue<L>::_headOfFreeList = 0;
template<class L> const int DQueue<L>::BLOCK_SIZE = 200;
template<class L> Mutex DQueue<L>::_alloc_mut;

template<class L> void *DQueue<L>::operator new(size_t size)
{
    if (size != sizeof(DQueue<L>))
        return ::operator new(size);

    AutoMutex autoMut(_alloc_mut);

    DQueue<L> *p = _headOfFreeList;
    if(p)
        _headOfFreeList = p->_dq_next;
    else
    {
        DQueue<L> * newBlock =
        static_cast<DQueue<L> *>(::operator new(BLOCK_SIZE * sizeof(DQueue<L>)));
        int i;
        for( i = 1; i < BLOCK_SIZE - 1; ++i)
            newBlock[i]._dq_next = &newBlock[i + 1];
        newBlock[BLOCK_SIZE - 1]._dq_next = 0;

        p = newBlock;
        _headOfFreeList = &newBlock[1];
    }


    return p;
}

template<class L> void DQueue<L>::operator delete(void *dead, size_t size)
{
    if(dead == 0)
        return;
    if(size != sizeof(DQueue<L>))
    {
        ::operator delete(dead);
        return;
    }
    DQueue<L> *p = static_cast<DQueue<L> *>(dead);
    AutoMutex autoMut(_alloc_mut);
    p->_dq_next = _headOfFreeList;
    _headOfFreeList = p;
}

template<class L> AsyncDQueue<L> * AsyncDQueue<L>::_headOfFreeList =0;
template<class L> const int AsyncDQueue<L>::BLOCK_SIZE = 20;
template<class L> Mutex AsyncDQueue<L>::_alloc_mut;

template<class L> void * AsyncDQueue<L>::operator new(size_t size)
{
    if (size != sizeof(AsyncDQueue<L>))
        return ::operator new(size);

    AutoMutex autoMut(_alloc_mut);

    AsyncDQueue<L> *p = _headOfFreeList;
    if(p)
        _headOfFreeList = p->_dq_next;
    else
    {
        AsyncDQueue<L> * newBlock =
        static_cast<AsyncDQueue<L> *>(::operator new(BLOCK_SIZE * sizeof(AsyncDQueue<L>)));
        int i;
        for( i = 1; i < BLOCK_SIZE - 1; ++i)
            newBlock[i]._dq_next = &newBlock[i + 1];
        newBlock[BLOCK_SIZE - 1]._dq_next = 0;

        p = newBlock;
        _headOfFreeList = &newBlock[1];
    }

    return p;
}

template<class L> void AsyncDQueue<L>::operator delete(void *deadObject, size_t size)
{
    if(deadObject == 0)
        return;
    if(size != sizeof(AsyncDQueue<L>))
    {
        ::operator delete(deadObject);
        return;
    }
    AsyncDQueue<L> *p = static_cast<AsyncDQueue<L> *>(deadObject);
    AutoMutex autoMut(_alloc_mut);
    p->_dq_next = _headOfFreeList;
    _headOfFreeList = p;
}

template<class L> DQueue<L>::DQueue(void)
    : Base(false)
{

}

template<class L> DQueue<L>::DQueue(Boolean head)
    :  Base(head)
{
    if(head == true)
    {
        _mutex.reset(new Mutex());
        _actual_count.reset(new AtomicInt(0));
    }
}


template<class L> DQueue<L>::~DQueue()
{

}


template<class L> void DQueue<L>::lock(void) throw(IPCException)
{
    _mutex->lock(pegasus_thread_self());
}

template<class L> void DQueue<L>::unlock(void) throw(IPCException)
{
    _mutex->unlock();
}

template<class L> void DQueue<L>::try_lock() throw(IPCException)
{
    _mutex->try_lock(pegasus_thread_self());
}

template<class L> void DQueue<L>::insert_first_no_lock(L *element) throw(IPCException)
{
    if( pegasus_thread_self() != _mutex->get_owner())
        throw Permission(pegasus_thread_self());
    Base::insert_first(static_cast<void *>(element));
    (*_actual_count.get())++;
}

template<class L> void DQueue<L>::insert_first(L *element) throw(IPCException)
{
    if(element == 0)
        return;
    AutoMutex autoMut(*_mutex.get());
    Base::insert_first(static_cast<void *>(element));
    (*_actual_count.get())++;
}

template<class L> void DQueue<L>::insert_last_no_lock(L *element) throw(IPCException)
{
    if( pegasus_thread_self() != _mutex->get_owner())
        throw Permission(pegasus_thread_self());
    Base::insert_last(static_cast<void *>(element));
    (*_actual_count.get())++;
}


template<class L> void DQueue<L>::insert_last(L *element) throw(IPCException)
{
    if(element == 0)
        return;
    AutoMutex autoMut(*_mutex.get());
    Base::insert_last(static_cast<void *>(element));
    (*_actual_count.get())++;
}


template<class L> void DQueue<L>::empty_list( void ) throw(IPCException)
{
    if( Base::count() > 0)
    {
        AutoMutex autoMut(*_mutex.get());
        Base::empty_list();
        (*_actual_count.get()) = 0;
    }
}


template<class L> L * DQueue<L>::remove_first ( void ) throw(IPCException)
{
    L *ret = 0;

    if( _actual_count->value() )
    {
        AutoMutex autoMut(*_mutex.get());
        ret = static_cast<L *>(Base::remove_first());
        if( ret != 0 )
            (*_actual_count.get())--;
    }

    return ret;
}

template<class L> L *DQueue<L>::remove_last ( void ) throw(IPCException)
{
    L * ret = 0;
    if( _actual_count->value() )
    {
        AutoMutex autoMut(*_mutex.get());
        ret = static_cast<L *>(Base::remove_last());
        if( ret != 0 )
            (*_actual_count.get())--;
    }

    return ret;
}

template<class L> L *DQueue<L>::remove_no_lock(const void *key) throw(IPCException)
{
    if(key == 0 )
        return 0;
    if( pegasus_thread_self() != _mutex->get_owner())
        throw Permission(pegasus_thread_self());

    if (_actual_count->value() )
    {
        L *ret = static_cast<L *>(Base::next(0));
        while( ret != 0 )
        {
            if (ret->operator==(key))
            {
                ret = static_cast<L *>(Base::remove(ret));
                if( ret != 0 )
                    (*_actual_count.get())--;
                return ret;
            }

            ret = static_cast<L *>(Base::next(static_cast<const void *>(ret)));
        }
    }

    return 0 ;
}

template<class L> L * DQueue<L>::remove_no_lock(const L *key) throw(IPCException)
{
    if(key == 0 )
        return 0;
    if( pegasus_thread_self() != _mutex->get_owner())
        throw Permission(pegasus_thread_self());

    if (_actual_count->value() )
    {
        L *ret = static_cast<L *>(Base::next(0));
        while( ret != 0 )
        {
            if (ret->operator==(*key))
            {
                ret = static_cast<L *>(Base::remove(static_cast<const void *>(ret)));
                if( ret != 0 )
                    (*_actual_count.get())--;
                return ret;
            }

            ret = static_cast<L *>(Base::next(static_cast<const void *>(ret)));
        }
    }

    return 0 ;
}


template<class L> L * DQueue<L>::remove(const void *key) throw(IPCException)
{
    L *ret = 0;

    if( _actual_count->value() > 0 )
    {
        AutoMutex autoMut(*_mutex.get());
        ret = DQueue<L>::remove_no_lock(key);
    }

    return(ret);
}

template<class L>L *DQueue<L>::remove(const L *key) throw(IPCException)
{
    L *ret = 0;

    if( _actual_count->value() > 0 )
    {
        AutoMutex autoMut(*_mutex.get());
        ret = DQueue<L>::remove_no_lock(key);
    }

     return(ret);
}

template<class L> L *DQueue<L>::reference(const L *key)
{
    if(key == 0)
        return 0;

    if( pegasus_thread_self() != _mutex->get_owner())
        throw Permission(pegasus_thread_self());

    if( _actual_count->value() )
    {
        L *ret = static_cast<L *>(Base::next(0));
        while(ret != 0)
        {
            if(ret->operator==(*key))
                return ret;
            ret = static_cast<L *>(Base::next(static_cast<const void *>(ret)));
        }
    }

    return(0);
}

template<class L> L *DQueue<L>::reference(const void *key) throw(IPCException)
{
    if(key == 0)
        return 0;

    if( pegasus_thread_self() != _mutex->get_owner())
        throw Permission(pegasus_thread_self());

    if( _actual_count->value() )
    {
        L *ret = static_cast<L *>(Base::next(0));
        while(ret != 0)
        {
            if(ret->operator==(key))
                return ret;
            ret = static_cast<L *>(Base::next(static_cast<const void *>(ret)));
        }
    }

    return(0);
}

template<class L> L * DQueue<L>::next( const void * ref) throw(IPCException)
{
    if (_mutex->get_owner() != pegasus_thread_self())
        throw Permission(pegasus_thread_self()) ;

    return static_cast<L *>(Base::next(ref));
}

template<class L> L *DQueue<L>::prev( const void *ref) throw(IPCException)
{
    if (_mutex->get_owner() != pegasus_thread_self())
        throw Permission(pegasus_thread_self());

    return  static_cast<L *>(Base::prev(ref));
}

template<class L> Boolean DQueue<L>::exists(const void *key) throw(IPCException)
{
    if(key == 0)
        return false;

    Boolean ret = false;
    if(_actual_count->value() > 0)
    {
        AutoMutex autoMut(*_mutex.get());
        ret = (DQueue<L>::reference(key) != 0);
    }

    return(ret);
}

template<class L> Boolean DQueue<L>::exists(const L *key) throw(IPCException)
{
    if(key == 0)
        return false;

    Boolean ret = false;
    if(_actual_count->value() > 0)
    {
        AutoMutex autoMut(*_mutex.get());
        ret = (DQueue<L>::reference(key) != 0);
    }

    return(ret);
}

template<class L> void AsyncDQueue<L>::_insert_prep(void) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
        unlock();
        throw ListClosed();
    }

    _slot->lock_object(pegasus_thread_self());
    while(true == is_full())
    {
        _slot->unlocked_wait(pegasus_thread_self());
        if(_disallow->value() > 0)
        {
            unlock();
            throw ListClosed();
        }
    }
}

template<class L> void AsyncDQueue<L>::_insert_recover(void) throw(IPCException)
{
    _node->unlocked_signal(pegasus_thread_self());
    (*_actual_count.get())++;
    unlock();
}

template<class L> void AsyncDQueue<L>::_unlink_prep(void) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
        unlock();
        throw ListClosed();
    }
    _node->lock_object(pegasus_thread_self());
    while(true == is_empty())
    {
        _node->unlocked_wait(pegasus_thread_self());
       if(_disallow->value() > 0)
       {
           unlock();
           throw ListClosed();
       }
    }
}

template<class L> void AsyncDQueue<L>::_unlink_recover(void) throw(IPCException)
{
    _slot->unlocked_signal(pegasus_thread_self());
    (*_actual_count.get())--;
    unlock();
}

template<class L> L * AsyncDQueue<L>::_remove_no_lock(const void *key) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
        unlock();
        throw ListClosed();
    }
    if( pegasus_thread_self() != _cond->get_owner())
        throw Permission(pegasus_thread_self());
    L *ret = static_cast<L *>(Base::next(0));
    while(ret != 0)
    {
        if(ret->operator==(key))
       {
           return static_cast<L *>(Base::remove(static_cast<const void *>(ret)));
       }

       ret = static_cast<L *>(Base::next(static_cast<const void *>(ret)));
    }

    return 0;
}

template<class L> L *AsyncDQueue<L>::_remove_no_lock(const L *key) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
        unlock();
        throw ListClosed();
    }
    if( pegasus_thread_self() != _cond->get_owner())
        throw Permission(pegasus_thread_self());
    L *ret = static_cast<L *>(Base::next(0));
    while(ret != 0)
    {
       if(ret->operator==(*key))
       {
           return static_cast<L *>(Base::remove(static_cast<const void *>(ret)));
       }

       ret = static_cast<L *>(Base::next(static_cast<const void *>(ret)));
    }

    return 0;
}


template<class L> AsyncDQueue<L>::AsyncDQueue(void)
    : Base(false)
{

}

template<class L> AsyncDQueue<L>::AsyncDQueue(Boolean head, Uint32 capacity )
   : Base(head)
{
    if(head == true)
    {
        _cond.reset(new Mutex());
        _slot.reset(new Condition(*_cond.get()));
        _node.reset(new Condition(*_cond.get()));
        _actual_count.reset(new AtomicInt(0));
        _disallow.reset(new AtomicInt(0));
        _capacity.reset(new AtomicInt(capacity));
    }
}

template<class L> AsyncDQueue<L>::~AsyncDQueue(void)
{

}


template<class L> void AsyncDQueue<L>::shutdown_queue(void)
{
    try
    {
        lock(pegasus_thread_self());
        (*_disallow.get())++;
        _node->disallow();
        _node->unlocked_signal(pegasus_thread_self());
        _node->unlocked_signal(pegasus_thread_self());
        _slot->disallow();
        _slot->unlocked_signal(pegasus_thread_self());
        _slot->unlocked_signal(pegasus_thread_self());
        unlock();
    }
    catch(const ListClosed &)
    {
        (*_disallow.get())++;
    }
}

template<class L> Boolean AsyncDQueue<L>::is_full(void)
{
    return false;


    if( _capacity->value() == 0 )
        return false;

    if(_actual_count->value() >= _capacity->value())
        return true;
    return false;
}

template<class L> Boolean AsyncDQueue<L>::is_empty(void)
{
    if(_actual_count->value() == 0)
       return true;
    return false;
}


template<class L> Boolean AsyncDQueue<L>::is_shutdown(void)
{
    if( _disallow->value() > 0)
        return true;
     return false;
}

template<class L> void AsyncDQueue<L>::try_lock(PEGASUS_THREAD_TYPE myself) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
        throw ListClosed();
    }

    _cond->try_lock(myself);
}

template<class L> void AsyncDQueue<L>::lock(PEGASUS_THREAD_TYPE myself) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
       throw ListClosed();
    }
    _cond->lock(myself);
}

template<class L> void AsyncDQueue<L>::unlock(void)
{
    _cond->unlock();
}

template<class L> void AsyncDQueue<L>::signal_slot(void) throw(IPCException)
{
    AutoMutex autoMut(*_cond.get());
    _slot->unlocked_signal(pegasus_thread_self());
}

template<class L> void AsyncDQueue<L>::signal_node(void) throw(IPCException)
{
    AutoMutex autoMut(*_cond.get());
    _node->unlocked_signal(pegasus_thread_self());
}

template<class L> Condition *AsyncDQueue<L>::get_node_cond(void)
{
    return _node.get() ;
}

template<class L> Condition * AsyncDQueue<L>::get_slot_cond(void)
{
    return _slot.get()
}

template<class L> void AsyncDQueue<L>::wait_for_node(void) throw(IPCException)
{
    _unlink_prep();
}

template<class L> void AsyncDQueue<L>::set_capacity(Uint32 capacity) throw(IPCException)
{
    lock(pegasus_thread_self());
    *_capacity.get() = capacity;
    unlock();
}

template<class L> Uint32 AsyncDQueue<L>::get_capacity(void)
{
    return _capacity->value();
}

template<class L> void AsyncDQueue<L>::insert_first(L *element) throw(IPCException)
{
    if(element == 0)
        return;

    lock(pegasus_thread_self());
    if(true == is_full())
    {
        unlock();
        throw ListFull(_capacity->value());
    }
    Base::insert_first(static_cast<void *>(element));
    _insert_recover();
}

template<class L> void AsyncDQueue<L>::insert_first_wait(L *element) throw(IPCException)
{
    if(element == 0)
       return;

    _insert_prep();
    Base::insert_first(static_cast<void *>(element));
    _insert_recover();
}

template<class L> void AsyncDQueue<L>::insert_last(L *element) throw(IPCException)
{
    if(element == 0)
       return;
    lock(pegasus_thread_self());
    if(true == is_full())
    {
        unlock();
        throw ListFull(_capacity->value());
    }
    Base::insert_last(static_cast<void *>(element));
    _insert_recover();
}

template<class L> void AsyncDQueue<L>::insert_last_wait(L *element) throw(IPCException)
{
    if(element == 0)
        return;
    _insert_prep();
    Base::insert_last(element);
    _insert_recover();
}

template<class L> void AsyncDQueue<L>::empty_list(void) throw(IPCException)
{
    lock(pegasus_thread_self());
    Base::empty_list();
    (*_actual_count.get()) = 0;
    _slot->unlocked_signal(pegasus_thread_self());
    unlock();
}

template<class L> L *AsyncDQueue<L>::remove_first(void) throw(IPCException)
{

    lock(pegasus_thread_self());
    L *ret = static_cast<L *>(Base::remove_first());
    if(ret != 0)
    {
        _slot->unlocked_signal(pegasus_thread_self());
        (*_actual_count.get())--;
    }
    unlock();
    return ret;
}

template<class L> L *AsyncDQueue<L>::remove_first_wait(void) throw(IPCException)
{
    _unlink_prep();
    L *ret = static_cast<L *>(Base::remove_first());
    _unlink_recover();
    return ret;
}

template<class L> L *AsyncDQueue<L>::remove_last(void) throw(IPCException)
{
    lock(pegasus_thread_self());

    L *ret = static_cast<L *>(Base::remove_last());
    if(ret != 0)
    {
        (*_actual_count.get())--;
        _slot->unlocked_signal(pegasus_thread_self());
    }
    unlock();
    return ret;
}

template<class L> L *AsyncDQueue<L>::remove_last_wait(void) throw(IPCException)
{
    _unlink_prep();
    L *ret = static_cast<L *>(Base::remove_last());
    _unlink_recover();
    return ret;
}

template<class L> L *AsyncDQueue<L>::remove(const void *key) throw(IPCException)
{
    if(key == 0)
        return 0;
    lock(pegasus_thread_self());

    L *ret = _remove_no_lock(key);
    if(ret != 0)
    {
        (*_actual_count.get())--;
        _slot->unlocked_signal(pegasus_thread_self());
    }
    unlock();
    return ret;
}

template<class L>L *AsyncDQueue<L>::remove(const L *key) throw(IPCException)
{
    if(key == 0)
        return 0;
     lock(pegasus_thread_self());

    L *ret = _remove_no_lock(key);
    if(ret != 0)
    {
        (*_actual_count.get())--;
        _slot->unlocked_signal(pegasus_thread_self());
    }
    unlock();
    return ret;
}

template<class L> L *AsyncDQueue<L>::remove_no_lock(const void *key) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
        unlock();
        throw ListClosed();
    }

    if(key == 0)
        return 0;

    L *ret = 0;

    if(Base::count() > 0 )
    {
        ret = _remove_no_lock(key);
        if(ret != 0)
        {
            (*_actual_count.get())--;
            _slot->unlocked_signal(pegasus_thread_self());
        }
    }

    return ret;
}


template<class L> L *AsyncDQueue<L>::remove_no_lock(const L *key) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
        unlock();
        throw ListClosed();
    }

    if(key == 0)
        return 0;

    L *ret = 0;

    if(Base::count() > 0 )
    {
        ret = _remove_no_lock(key);
        if(ret != 0)
        {
            (*_actual_count.get())--;
            _slot->unlocked_signal(pegasus_thread_self());
       }
    }

    return ret;
}

template<class L> L *AsyncDQueue<L>::remove_wait(const void *key) throw(IPCException)
{

    if(key == 0)
        return 0;

    lock(pegasus_thread_self());

    L *ret = _remove_no_lock(key);
    while( ret == 0 )
    {
        if(_disallow->value() > 0)
        {
            unlock();
            throw ListClosed();
        }
        _node->unlocked_wait(pegasus_thread_self());
        if(_disallow->value() > 0)
       {
          unlock();
          throw ListClosed();
       }
       ret = _remove_no_lock(key);
    }
    if(ret != 0)
    {
        (*_actual_count.get())--;
        _slot->unlocked_signal(pegasus_thread_self());
    }
    unlock();
    return ret;
}

template<class L> L *AsyncDQueue<L>::next(const L *ref) throw(IPCException)
{
   if( pegasus_thread_self() != _cond->get_owner())
        throw Permission(pegasus_thread_self());

    return static_cast<L *>(Base::next( reinterpret_cast<const void *>(ref)));
}

template<class L> L *AsyncDQueue<L>::prev(const L *ref) throw(IPCException)
{
    if( pegasus_thread_self() != _cond->get_owner())
        throw Permission(pegasus_thread_self());

    return static_cast<L *>(Base::prev( reinterpret_cast<const void *>(ref)));
}

template<class L> L *AsyncDQueue<L>::reference(const void *key) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
        unlock();
        throw ListClosed();
    }

    if( key == 0 )
        return 0;

    if( pegasus_thread_self() != _cond->get_owner())
        throw Permission(pegasus_thread_self());

    if(Base::count() > 0 )
    {
        L *ret = static_cast<L *>(Base::next(0));
        while(ret != 0)
        {
            if(ret->operator==(key))
                return ret;
            ret = static_cast<L *>(Base::next(static_cast<const void *>(ret)));
        }
    }

    return(0);
}

template<class L> L *AsyncDQueue<L>::reference(const L *key) throw(IPCException)
{
    if(_disallow->value() > 0)
    {
        unlock();
        throw ListClosed();
    }

    if(key == 0)
       return 0;

    if( pegasus_thread_self() != _cond->get_owner())
        throw Permission(pegasus_thread_self());

    if(Base::count() > 0 )
    {
        L *ret = static_cast<L *>(Base::next(0));
        while(ret != 0)
        {
            if(ret->operator==(*key))
                return ret;
            ret = static_cast<L *>(Base::next(static_cast<const void *>(ret)));
        }
    }

    return(0);
}

template<class L> Uint32 AsyncDQueue<L>::count(void)
{
    return _actual_count->value();
}

template<class L> Uint32 AsyncDQueue<L>::size(void)
{
    return _actual_count->value();
}


PEGASUS_NAMESPACE_END

#endif /* Pegasus_DQueue_h */
