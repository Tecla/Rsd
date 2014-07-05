////////////
//
//  File:      Reference.h
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data variable references
//
////////////

#include <Rsd/Reference.h>
#include <Rsd/Value.h>
#include <Rsd/Parser.h>


namespace RenderSpud
{
    namespace Rsd
    {


Reference::Reference(const Reference& ref)
    : ReferenceCounted(), m_parts()
{
    for (std::list<Part>::const_iterator iter = ref.m_parts.begin();
         iter != ref.m_parts.end();
         ++iter)
    {
        Part p;
        p.m_identifier = iter->m_identifier;
        p.m_pSubscriptValue = iter->m_pSubscriptValue->clone();
        m_parts.push_back(p);
    }
}


Reference::Ptr Reference::clone() const
{
    return Reference::Ptr(new Reference(*this));
}


Reference::PartType Reference::getPartType(const Part& p)
{
    return (p.m_identifier != "") ? kPartIdentifier : kPartSubscript;
}


std::string Reference::str() const
{
    std::string results;
    bool first = true;
    for (std::list<Part>::const_iterator iter = m_parts.begin();
         iter != m_parts.end();
         ++iter)
    {
        if (Reference::getPartType(*iter) == kPartIdentifier)
        {
            if (!first)
                results += '.';
            results += iter->m_identifier;
        }
        else
        {
            std::string subscriptStr = iter->m_pSubscriptValue->str(false,
                                                                    true,
                                                                    0);
            results += '[';
            results += subscriptStr;
            results += ']';
        }
        first = false;
    }
    return results;
}


Reference::Ptr Reference::fromString(const std::string& refStr)
{
    Parser::Parser referenceParser;
    Reference::Ptr pRef(new Reference());
    referenceParser.parseReference(refStr, *pRef);
    return pRef;
}


    } // namespace Rsd
} // namespace RenderSpud
