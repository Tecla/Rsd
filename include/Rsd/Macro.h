////////////
//
//  File:      Macro.h
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data macro functions
//
////////////

#ifndef __RSD_Macro_h__
#define __RSD_Macro_h__

#include <string>
#include <map>
#include <vector>

#include <Rsd/Base.h>


namespace RenderSpud
{
    namespace Rsd
    {


// Forward declaration needed for MacroInvocation
class Value;


//
// Macro
//

// To create a macro, follow this paradigm:
//
// class MyMacro : public Macro
// {
// public:
//     MyMacro(const std::string& name) : Macro(name) { }
//     virtual ~MyMacro() { }
//
//     virtual Ptr<Value> execute(const Value& context,
//                                const ArgumentValueMap& keywordArgValues);
// };
//
// void setupMyMacros()
// {
//     new MyMacro("MyMacro"); // Yes, this really is all you have to do
// }
//
// void takedownMyMacros()
// {
//     Macro::unregisterMacro(Macro::find("MyMacro"));
// }
//
// And of course, call setupMyMacros() and takedownMyMacros() while you want it available.

/// Macros create values when executed
class Macro : public ReferenceCounted
{
public:
    typedef RenderSpud::Ptr<Macro> Ptr;
    typedef RenderSpud::Ptr<const Macro> ConstPtr;

    typedef std::map< std::string, RenderSpud::Ptr<Value> > ArgumentValueMap;


public:
    explicit Macro(const std::string& name)
        : ReferenceCounted(), m_name(name), m_argumentKeywords()
    {
        // Auto-register the macro
        Macro::registerMacro(Ptr(this));
    }

    virtual ~Macro()
    {
        Ptr shouldBeThis = Macro::find(m_name);
        if (shouldBeThis == this)
        {
            Macro::unregisterMacro(shouldBeThis);
        }
    }

    /// Name of this macro (must be unique among all macros)
    const std::string& name() const { return m_name; }

    /// Run the macro to create a value
    virtual RenderSpud::Ptr<Value> execute(const Value& context,
                                           const ArgumentValueMap& keywordArgValues) = 0;

    /// Register the macro so it is available for resolving
    static void registerMacro(Macro::Ptr pMacro);
    /// Unregister the macro to make it unavailable for resolving
    static void unregisterMacro(Macro::Ptr pMacro);

    /// Find a macro by name
    static Macro::Ptr find(const std::string& name);

private:
    // Forbidden
    Macro() { }
    Macro(const Macro&) { }
    Macro& operator =(const Macro&) { return *this; }

protected:
    std::string m_name;
    std::vector<std::string> m_argumentKeywords;
};


//
// MacroInvocation
//

/// Representation of a macro "call" with value arguments ready to pass in.
class MacroInvocation : public ReferenceCounted
{
public:
    typedef RenderSpud::Ptr<MacroInvocation> Ptr;
    typedef RenderSpud::Ptr<const MacroInvocation> ConstPtr;

    typedef std::map< std::string, RenderSpud::Ptr<Value> > ArgumentValueMap;

public:
    MacroInvocation() : m_name(), m_arguments() { }

    /// Deep-copy of the macro invocation.
    MacroInvocation::Ptr clone() const;

    /// Name of the macro invocation (matches the macro to be executed).
    const std::string& name() const    { return m_name; }
    std::string&       name()          { return m_name; }
    void setName(const std::string& n) { m_name = n; }

    /// Value arguments to be passed to the macro to be executed.
    const ArgumentValueMap& arguments() const { return m_arguments; }
    ArgumentValueMap&       arguments()       { return m_arguments; }

    /// Find and execute the actual macro with the values in this invocation.
    RenderSpud::Ptr<Value> execute(const Value& context);

    /// Textual representation of this macro invocation.
    std::string str() const;

private:
    MacroInvocation(const MacroInvocation& mi);

    std::string m_name;
    ArgumentValueMap m_arguments;

protected:
    virtual ~MacroInvocation() { }
};


    } // namespace Rsd
} // namespace RenderSpud


#endif // __RSD_Macro_h__
