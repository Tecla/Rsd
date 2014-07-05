////////////
//
//  File:      Reference.h
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data variable references
//
////////////

#ifndef __RSD_Reference_h__
#define __RSD_Reference_h__

#include <string>
#include <list>

#include <Rsd/Base.h>


namespace RenderSpud
{
    namespace Rsd
    {


// Forward declaration needed for Reference
class Value;


//
// Reference
//

/// Reference to another value (by mixed path and nested array subscripts)
class Reference : public ReferenceCounted
{
public:
    enum PartType
    {
        kPartIdentifier,
        kPartSubscript
    };

    /// References are composed of parts, either path identifiers
    /// or subscript values (which themselves may be references)
    struct Part
    {
        std::string m_identifier;
        RenderSpud::Ptr<Value> m_pSubscriptValue;

        Part() : m_identifier(), m_pSubscriptValue() { }
    };

    typedef std::list<Part> PartsList;

    typedef RenderSpud::Ptr<Reference> Ptr;
    typedef RenderSpud::Ptr<const Reference> ConstPtr;

public:
    /// Create an empty reference
    Reference() : ReferenceCounted(), m_parts() { }

    /// Deep-copy this reference to a new one.
    Reference::Ptr clone() const;

    /// List of parts making up this reference
    const PartsList& parts() const { return m_parts; }
    PartsList&       parts()       { return m_parts; }

    /// Textual representation of this reference
    std::string str() const;

    /// Invoke reference expression parser to parse a string
    static Reference::Ptr fromString(const std::string& refStr);

    /// Get what kind this part in the reference is
    static PartType getPartType(const Part& p);

private:
    Reference(const Reference& ref);

    PartsList m_parts;

protected:
    virtual ~Reference() { }
};


    } // namespace Rsd
} // namespace RenderSpud


#endif // __RSD_Reference_h__
