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
#include <atomic>
#include <cassert>
#include <malloc.h>

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
// directly, but instead use Ptr<ClassName> (or IntrusivePtr<ClassName>)
// and let it go out of scope.  The last smart pointer dying deletes the object.
class ReferenceCounted
{
public:
    ReferenceCounted() : m_refCount(0) { }

    void incrementReference() const
    {
        // Adding a reference lazily should be safe from any thread, as it can
        // only happen when an existing reference lives in the same thread.
        m_refCount.fetch_add(1, std::memory_order_relaxed);
    }

    void decrementReference() const
    {
        // Clearing a reference requires a sync on decrement, so we get the
        // count right.  We only have to do a barrier for starting subsequent
        // operations when we are going to delete it, otherwise the other
        // threads can move along.
        if (m_refCount.fetch_sub(1, std::memory_order_release) == 1)
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete this;
        }
    }

    unsigned int referenceCount() const
    {
        return m_refCount.load(std::memory_order_consume);
    }

    // Don't use this unless you know what you're doing (or you'll get leaks)
    void decrementReferenceNoDestroy() const
    {
        if (referenceCount() > 0)
        {
            m_refCount.fetch_sub(1, std::memory_order_release);
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
    mutable std::atomic<unsigned int> m_refCount;
};


//
//  IntrusivePtr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on unqualified calls to
//
//      void intrusiveAddRef(T *p);
//      void intrusiveDecRef(T *p);
//
//          (p != nullptr)
//
//  The object is responsible for destroying itself.
//
template<class T>
class IntrusivePtr
{
private:
    typedef IntrusivePtr SelfType;

public:

    class EmptyType { };

    typedef T element_type;

    IntrusivePtr() noexcept : px(nullptr) { }

    IntrusivePtr(T *p, bool addRef = true): px(p)
    {
        if (px != nullptr && addRef)
            intrusiveAddRef(px);
    }

    template<class U>
    IntrusivePtr(IntrusivePtr<U> const& rhs, typename std::enable_if<std::is_convertible<U*,T*>::value, EmptyType>::type = EmptyType())
        : px(rhs.get())
    {
        if (px != nullptr)
            intrusiveAddRef(px);
    }

    IntrusivePtr(IntrusivePtr const& rhs)
        : px(rhs.px)
    {
        if (px != nullptr)
            intrusiveAddRef(px);
    }

    ~IntrusivePtr()
    {
        if (px != nullptr)
            intrusiveDecRef(px);
    }

    template<class U>
    IntrusivePtr& operator =(IntrusivePtr<U> const& rhs)
    {
        SelfType(rhs).swap(*this);
        return *this;
    }

    IntrusivePtr(IntrusivePtr&& rhs) noexcept
        : px(rhs.px)
    {
        rhs.px = nullptr;
    }

    IntrusivePtr& operator =(IntrusivePtr&& rhs) noexcept
    {
        SelfType(static_cast<IntrusivePtr&&>(rhs)).swap(*this);
        return *this;
    }

    IntrusivePtr& operator=(IntrusivePtr const& rhs)
    {
        SelfType(rhs).swap(*this);
        return *this;
    }

    IntrusivePtr& operator=(T *rhs)
    {
        SelfType(rhs).swap(*this);
        return *this;
    }

    void reset() noexcept
    {
        SelfType().swap(*this);
    }

    void reset(T *rhs)
    {
        SelfType(rhs).swap(*this);
    }

    T* get() const noexcept
    {
        return px;
    }

    T& operator *() const
    {
        assert(px != nullptr);
        return *px;
    }

    T* operator ->() const
    {
        assert(px != nullptr);
        return px;
    }

    explicit operator bool() const noexcept
    {
        return px != nullptr;
    }

    // operator! is redundant, but some compilers need it
    bool operator !() const noexcept
    {
        return px == nullptr;
    }

    void swap(IntrusivePtr& rhs) noexcept
    {
        T *tmp = px;
        px = rhs.px;
        rhs.px = tmp;
    }

private:
    T *px;
};


template<class T, class U>
inline bool operator ==(IntrusivePtr<T> const& a, IntrusivePtr<U> const& b)
{
    return a.get() == b.get();
}

template<class T, class U>
inline bool operator !=(IntrusivePtr<T> const& a, IntrusivePtr<U> const& b)
{
    return a.get() != b.get();
}

template<class T, class U>
inline bool operator ==(IntrusivePtr<T> const& a, U *b)
{
    return a.get() == b;
}

template<class T, class U>
inline bool operator !=(IntrusivePtr<T> const& a, U *b)
{
    return a.get() != b;
}

template<class T, class U>
inline bool operator ==(T *a, IntrusivePtr<U> const& b)
{
    return a == b.get();
}

template<class T, class U>
inline bool operator !=(T *a, IntrusivePtr<U> const& b)
{
    return a != b.get();
}

template<class T>
inline bool operator ==(IntrusivePtr<T> const & p, std::nullptr_t) noexcept
{
    return p.get() == nullptr;
}

template<class T>
inline bool operator ==(std::nullptr_t, IntrusivePtr<T> const& p) noexcept
{
    return p.get() == nullptr;
}

template<class T>
inline bool operator !=(IntrusivePtr<T> const& p, std::nullptr_t) noexcept
{
    return p.get() != nullptr;
}

template<class T>
inline bool operator !=(std::nullptr_t, IntrusivePtr<T> const& p) noexcept
{
    return p.get() != nullptr;
}

template<class T>
inline bool operator <(IntrusivePtr<T> const& a, IntrusivePtr<T> const& b)
{
    return std::less<T *>()(a.get(), b.get());
}

template<class T>
void swap(IntrusivePtr<T>& lhs, IntrusivePtr<T>& rhs)
{
    lhs.swap(rhs);
}

// mem_fn support

template<class T>
T* get_pointer(IntrusivePtr<T> const& p)
{
    return p.get();
}

template<class T, class U>
IntrusivePtr<T> static_pointer_cast(IntrusivePtr<U> const& p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U>
IntrusivePtr<T> const_pointer_cast(IntrusivePtr<U> const& p)
{
    return const_cast<T *>(p.get());
}

template<class T, class U>
IntrusivePtr<T> dynamic_pointer_cast(IntrusivePtr<U> const& p)
{
    return dynamic_cast<T *>(p.get());
}


// Required to use ReferenceCounted with intrusive_ptr<T>

inline void intrusiveAddRef(ReferenceCounted *pRefCounted)
{
    pRefCounted->incrementReference();
}

inline void intrusiveAddRef(const ReferenceCounted *pRefCounted)
{
    pRefCounted->incrementReference();
}

inline void intrusiveDecRef(ReferenceCounted *pRefCounted)
{
    pRefCounted->decrementReference();
}

inline void intrusiveDecRef(const ReferenceCounted *pRefCounted)
{
    pRefCounted->decrementReference();
}


template<typename T>
using Ptr = IntrusivePtr<T>;

    
} // namespace RenderSpud


#endif // __RSD_Memory_h__
