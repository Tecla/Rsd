////////////
//
// File:      Platform.h
// Module:    RSD
// Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
// Content:   Platform defines, and generic programming concepts for the core
//
////////////

#ifndef __RSD_Platform_h__
#define __RSD_Platform_h__

#include <cstdlib>
#include <cstddef>
#include <cstdint>


#if !defined(__SSE__) && (_M_IX86_FP == 1 || _M_IX86_FP == 2)
    #define __SSE__ 1
#endif

#if defined(__SSE4_1__)
    #include <smmintrin.h>
#elif defined(__SSE3__)
    #include <pmmintrin.h>
#elif defined(__SSE2__)
    #include <emmintrin.h>
#elif defined(__SSE__)
    #include <xmmintrin.h>
#endif

#ifdef __SSE__
    #define RS_SIMD 1
    #define RS_SSE_SIMD 1
#endif

#if defined(__GNUC__) && !defined(restrict)
    #define restrict __restrict
#elif !defined(restrict)
    #define restrict
#endif

#ifdef __GNUC__
    #define RS_ALIGN(x)  __attribute__ ((aligned(x)))
    #define RS_MAY_ALIAS __attribute__ ((__may_alias__))
#else
    #define RS_ALIGN(x) __declspec(align(x))
    #define RS_MAY_ALIAS
#endif

// Freaking Microsoft
#define NOMINMAX 1

#if defined(min) || defined(_WIN32)
    #undef min
#endif

#if defined(max) || defined(_WIN32)
    #undef max
#endif

#if defined(_WIN32)
    typedef long ssize_t;
#endif

#ifndef __int8_t_defined
# define __int8_t_defined
typedef signed char		int8_t;
typedef short int		int16_t;
typedef int			int32_t;
# if defined(__WORDSIZE) && __WORDSIZE == 64
typedef long int		int64_t;
# else
typedef long long int		int64_t;
# endif
#endif

/* Unsigned.  */
#ifndef __uint32_t_defined
typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
#ifndef __uint32_t_defined
typedef unsigned int		uint32_t;
# define __uint32_t_defined
#endif
#if defined(__WORDSIZE) && __WORDSIZE == 64
typedef unsigned long int	uint64_t;
#else
typedef unsigned long long int	uint64_t;
#endif
#endif



#if __GNUC__
    #define RS_DYNAMIC_STACK_ALLOC 1
#endif


namespace RenderSpud
{
    namespace Core
    {


#if RS_SSE_SIMD
    
const size_t kNativeSimdSize = 4;
const size_t kNativeSimdAlignment = 16;

#define RS_SIMD_ALIGNMENT 16
#define RS_SIMD_ALIGN RS_ALIGN(16)


#ifdef __GNUC__

typedef __m128  v4sf __attribute__((aligned(16)));
typedef __m128i v4si __attribute__((aligned(16)));

#else

typedef __m128 v4sf;
typedef __m128i v4si;

#endif

#ifndef __GNUC__

inline v4sf operator +(const v4sf& v1, const v4sf& v2) { return _mm_add_ps(v1, v2); }
inline v4sf operator -(const v4sf& v1, const v4sf& v2) { return _mm_sub_ps(v1, v2); }
inline v4sf operator -(const v4sf& v)                  { return _mm_sub_ps(_mm_setzero_ps(), v); }
inline v4sf operator *(const v4sf& v1, const v4sf& v2) { return _mm_mul_ps(v1, v2); }
inline v4sf operator *(const v4sf& v, float f)         { return _mm_mul_ps(v, _mm_set1_ps(f)); }
inline v4sf operator *(float f, const v4sf& v)         { return _mm_mul_ps(_mm_set1_ps(f), v); }
inline v4sf operator /(const v4sf& v1, const v4sf& v2) { return _mm_div_ps(v1, v2); }
inline v4sf operator /(const v4sf& v, float f)         { return _mm_div_ps(v, _mm_set1_ps(f)); }

#endif


#else

const size_t kNativeSimdSize = 1;
const size_t kNativeSimdAlignment = sizeof(void*);

#define RS_SIMD_ALIGNMENT sizeof(void*)
#define RS_SIMD_ALIGN

#endif


#if 0

////////////
// Concepts
////////////

template<typename Type>
class ValueConcept
{
public:
    typedef ValueConcept<Type> SelfType;


public:
    ValueConcept();
    ValueConcept(const SelfType& value);
    ValueConcept(const Type& initialValue); // May be redundant if this is a built-in type

    SelfType& operator =(const SelfType& value);

    SelfType  operator +(const SelfType& value);
    SelfType& operator +=(const SelfType& value);

    SelfType  operator -(const SelfType& value);
    SelfType& operator -=(const SelfType& value);

    SelfType  operator *(const SelfType& value);
    SelfType& operator *=(const SelfType& value);

    SelfType  operator /(const SelfType& value);
    SelfType& operator /=(const SelfType& value);

    // These really should return masks, so that they fit the
    // set/container concepts as well
    bool operator <(const SelfType& value);
    bool operator <=(const SelfType& value);
    bool operator >(const SelfType& value);
    bool operator >=(const SelfType& value);
    bool operator ==(const SelfType& value);
    bool operator !=(const SelfType& value);

    SelfType operator !();
};


// Indivisible ordered set, has specializations to take advantage of SIMD ops
template<size_t NumElements, typename Type>
class ValueSetConcept
{
public:
    typedef ValueSetConcept<NumElements, Type> SelfType;

    enum
    {
        kNumElements = NumElements
    };

public:
    ValueSetConcept();
    ValueSetConcept(const SelfType& set);
    ValueSetConcept(const Type* initialValues);

    SelfType& operator =(const SelfType& set);

    void setValues(const Type* newValues);
    void setValue(const Type& value);

    const Type& value(size_t i) const;
    Type&       value(size_t i);
    void        setValue(size_t i, const Type& value);

    const Type& operator [](size_t i) const;
    Type&       operator [](size_t i);

    size_t size() const { return kNumElements; }

    // These really should return masks, so that they fit the
    // set/container concepts as well
    bool operator <(const SelfType& set);
    bool operator <=(const SelfType& set);
    bool operator >(const SelfType& set);
    bool operator >=(const SelfType& set);
    bool operator ==(const SelfType& set);
    bool operator !=(const SelfType& set);

    SelfType operator !();
};


// Arbitrarily-sized ordered set, can contain subcontainers (probably organized around SIMD widths)
template<size_t NumElements, typename Subcontainer>
class ContainerConcept
{
public:
    typedef ContainerConcept<NumElements, Subcontainer> SelfType;
    typedef Subcontainer SubcontainerType;
    typedef typename Subcontainer::Type Type;

    enum
    {
        kNumElements = NumElements,
        kSubcontainerSize = Subcontainer::kNumElements,
        kNumSubcontainers = (kNumElements + kSubcontainerSize - 1) / kSubcontainerSize
    };

public:
    ContainerConcept();
    ContainerConcept(const SelfType& set);
    ContainerConcept(const Type* initialValues);

    SelfType& operator =(const SelfType& set);

    size_t size()             const { return kNumElements; }
    size_t subcontainerSize() const { return kSubcontainerSize; }
    size_t numSubcontainers() const { return kNumSubcontainers; }

    const SubcontainerType& subcontainer(size_t i) const;
    SubcontainerType&       subcontainer(size_t i);
    void                    subcontainer(size_t i, const SubcontainerType& sc);

    const SubcontainerType& operator ()(size_t i) const;
    SubcontainerType&       operator ()(size_t i);

    // These really should return masks, so that they fit the
    // set/container concepts as well
    bool operator <(const SelfType& set);
    bool operator <=(const SelfType& set);
    bool operator >(const SelfType& set);
    bool operator >=(const SelfType& set);
    bool operator ==(const SelfType& set);
    bool operator !=(const SelfType& set);

    SelfType operator !();
};

#endif


    } // namespace Core
} // namespace RenderSpud


#endif // __RSD_Platform_h__
