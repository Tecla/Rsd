////////////
//
//  File:      Base.h
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   Reference counted base class
//
////////////

#ifndef __RSD_Base_h__
#define __RSD_Base_h__

#include <Rsd/Memory.h>


namespace RenderSpud
{
    namespace Rsd
    {


// Base class for reference counted objects.  You should not delete one of these
// directly, but instead use ClassName::Ptr (or IntrusivePtr<ClassName>)
// and let it go out of scope.  The last smart pointer dying deletes the object.
typedef RenderSpud::ReferenceCounted ReferenceCounted;

/// Base exception class if you want to catch all RSD exceptions.  These derive
/// from std::exception so you may use the what() method to get a description.
class Exception : public std::exception
{
public:
    Exception() : std::exception() { }

    virtual ~Exception() throw() { }
};



    } // namespace Rsd
} // namespace RenderSpud


#endif // __RSD_Base_h__
