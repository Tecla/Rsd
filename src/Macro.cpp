////////////
//
//  File:      Macro.cpp
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data macro functions
//
////////////

#include <Rsd/Macro.h>
#include <Rsd/Value.h>


namespace RenderSpud
{
    namespace Rsd
    {


namespace
{

std::map<std::string, Macro::Ptr>& registeredMacros()
{
    // Function-static initialization trick so that others can auto-register
    // macros via static initialization without order issues
static std::map<std::string, Macro::Ptr> sRegisteredMacros;
    return sRegisteredMacros;
}

}

void Macro::registerMacro(Macro::Ptr pMacro)
{
    registeredMacros()[pMacro->name()] = pMacro;
}


void Macro::unregisterMacro(Macro::Ptr pMacro)
{
    std::map<std::string, Macro::Ptr>& rm = registeredMacros();
    std::map<std::string, Macro::Ptr>::iterator iter = rm.find(pMacro->name());
    if (iter != rm.end())
    {
        rm.erase(iter);
    }
}


Macro::Ptr Macro::find(const std::string& name)
{
    std::map<std::string, Macro::Ptr>& rm = registeredMacros();
    std::map<std::string, Macro::Ptr>::iterator iter = rm.find(name);
    if (iter != rm.end())
    {
        return iter->second;
    }
    return Macro::Ptr();
}


MacroInvocation::MacroInvocation(const MacroInvocation& mi)
    : m_name(mi.m_name), m_arguments()
{
    for (ArgumentValueMap::const_iterator iter = mi.m_arguments.begin();
         iter != mi.m_arguments.end();
         ++iter)
    {
        m_arguments.insert(std::pair<std::string, Value::Ptr>(iter->first,
                                                              iter->second->clone()));
    }
}


MacroInvocation::Ptr MacroInvocation::clone() const
{
    return MacroInvocation::Ptr(new MacroInvocation(*this));
}


Value::Ptr MacroInvocation::execute(const Value& context)
{
    Macro::Ptr pMacro = Macro::find(m_name);
    if (!pMacro)
    {
        throw ValueException(std::string("Could not execute macro \"") +
                                 m_name +
                                 "\" because its implementation could not be found.");
    }
    return pMacro->execute(context, m_arguments);
}


std::string MacroInvocation::str() const
{
    std::string result = m_name;
    result += '(';
    bool first = true;
    for (ArgumentValueMap::const_iterator iter = m_arguments.begin();
         iter != m_arguments.end();
         ++iter)
    {
        if (!first)
            result += ", ";
        first = false;

        result += iter->first;
        result += ": ";
        result += iter->second->str(false, true);
    }
    result += ')';
    return result;
}


    } // namespace Rsd
} // namespace RenderSpud
