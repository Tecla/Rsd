////////////
//
// File:      Memory.h
// Module:    RSD
// Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
// Content:   Aligned memory support
//
////////////

#ifndef __RSD_Memory_h__
#define __RSD_Memory_h__

#include <new>
#include <cstdlib>
#include <malloc.h>

#include <boost/noncopyable.hpp>
#include <boost/atomic.hpp>

#include <Rsd/Platform.h>


namespace RenderSpud
{


inline void* aligned_malloc(size_t size, size_t alignment)
{
#ifdef __GNUC__
#if _WIN32
    void *pResult = ::memalign(alignment, size);
    if (pResult == nullptr)
#else
    void *pResult = nullptr;
    if (::posix_memalign(&pResult, alignment, size) != 0)
#endif
    {
        pResult = ::malloc(size);
    }
    return pResult;
#else
    return _aligned_malloc(size, alignment);
#endif
}

inline void aligned_free(void *pPtr)
{
    if (pPtr == nullptr)
        return;

#ifdef __GNUC__
    free(pPtr);
#else
    _aligned_free(pPtr);
#endif
}

template<typename T, size_t kAlignment>
inline T *alignedAlloc(size_t numInstances)
{
    return reinterpret_cast<T*>(aligned_malloc(sizeof(T) * numInstances, kAlignment));
}

template<typename T>
inline void alignedFree(T* data)
{
    aligned_free(data);
}


template<size_t kAlignment>
class AlignBase
{
public:
    void* operator new(size_t size)
    {
        return aligned_malloc(size, kAlignment);
    }

    void* operator new[](size_t size)
    {
        return aligned_malloc(size, kAlignment);
    }

    void operator delete(void *pPtr)
    {
        aligned_free(pPtr);
    }

    void operator delete[](void *pPtr)
    {
        aligned_free(pPtr);
    }
};


#if RS_SIMD
typedef AlignBase<Core::kNativeSimdAlignment> SimdAlign;
#else
class FakeAlignBase { };
typedef FakeAlignBase SimdAlign;
#endif


// Base class for reference counted objects.  Utilizes atomics to ensure thread-
// safe reference counting behavior.  You should not delete one of these
// directly, but instead use Ptr<ClassName> (or boost::intrusive_ptr<ClassName>)
// and let it go out of scope.  The last smart pointer dying deletes the object.
class ReferenceCounted : private boost::noncopyable
{
public:
    ReferenceCounted() : m_refCount(0) { }

    void incrementReference() const
    {
        // Adding a reference lazily should be safe from any thread, as it can
        // only happen when an existing reference lives in the same thread.
        m_refCount.fetch_add(1, boost::memory_order_relaxed);
    }

    void decrementReference() const
    {
        // Clearing a reference requires a sync on decrement, so we get the
        // count right.  We only have to do a barrier for starting subsequent
        // operations when we are going to delete it, otherwise the other
        // threads can move along.
        if (m_refCount.fetch_sub(1, boost::memory_order_release) == 1)
        {
            boost::atomic_thread_fence(boost::memory_order_acquire);
            delete this;
        }
    }

    unsigned int referenceCount() const
    {
        return m_refCount.load(boost::memory_order_consume);
    }

    // Don't use this unless you know what you're doing (or you'll get leaks)
    void decrementReferenceNoDestroy() const
    {
        if (referenceCount() > 0)
        {
            m_refCount.fetch_sub(1, boost::memory_order_release);
        }
    }

protected:
    ReferenceCounted(const ReferenceCounted&) : m_refCount(0) { }

    // Must be virtual or we get slicing crashes
    virtual ~ReferenceCounted() { }

    ReferenceCounted& operator =(const ReferenceCounted&)
    {
        return *this;
    }


private:
    mutable boost::atomic<unsigned int> m_refCount;
};


} // namespace RenderSpud


namespace boost
{
    // Required to use ReferenceCounted with boost::intrusive_ptr<T>

    inline void intrusive_ptr_add_ref(RenderSpud::ReferenceCounted *pRefCounted)
    {
        pRefCounted->incrementReference();
    }

    inline void intrusive_ptr_add_ref(const RenderSpud::ReferenceCounted *pRefCounted)
    {
        pRefCounted->incrementReference();
    }

    inline void intrusive_ptr_release(RenderSpud::ReferenceCounted *pRefCounted)
    {
        pRefCounted->decrementReference();
    }

    inline void intrusive_ptr_release(const RenderSpud::ReferenceCounted *pRefCounted)
    {
        pRefCounted->decrementReference();
    }
}


#include <boost/smart_ptr/intrusive_ptr.hpp>


namespace RenderSpud
{


template<class T>
class Ptr : public boost::intrusive_ptr<T>
{
public:
    Ptr() : boost::intrusive_ptr<T>() { }

    Ptr(T *p, bool add_ref = true ): boost::intrusive_ptr<T>(p, add_ref) { }

    template<class U>
    Ptr(boost::intrusive_ptr<U> const &rhs,
        typename boost::detail::sp_enable_if_convertible<U, T>::type = boost::detail::sp_empty() )
        : boost::intrusive_ptr<T>(rhs) { }

    Ptr(Ptr const &rhs) : boost::intrusive_ptr<T>(rhs) { }

    template<class U>
    Ptr<T>& operator =(boost::intrusive_ptr<U> const &rhs)
    {
        Ptr<T>(rhs).swap(*this);
        return *this;
    }

    Ptr(Ptr<T> &&rhs) : boost::intrusive_ptr<T>(rhs) { }

    Ptr<T>& operator =(Ptr<T> &&rhs)
    {
        boost::intrusive_ptr<T>::operator =(rhs);
        return *this;
    }

    Ptr<T>& operator =(Ptr<T> const &rhs)
    {
        boost::intrusive_ptr<T>::operator =(rhs);
        return *this;
    }

    Ptr<T>& operator=(T *rhs)
    {
        boost::intrusive_ptr<T>::operator =(rhs);
        return *this;
    }
};

    
} // namespace RenderSpud


#endif // __RSD_Memory_h__
