////////////
//
//  File:      Value.h
//  Module:    Rsd
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data values
//
////////////

#ifndef __RSD_Value_h__
#define __RSD_Value_h__

#include <string>
#include <map>
#include <vector>
#include <limits>

#include <Rsd/Base.h>
#include <Rsd/Reference.h>
#include <Rsd/Macro.h>
#include <Rsd/TypeName.h>


namespace RenderSpud
{
    namespace Rsd
    {


// Forward declaration needed for Array
class Value;


//
// Exceptions
//

/// Exception thrown for various input/state errors in class Value
class ValueException : public Exception
{
public:
    ValueException(const std::string& desc) : Exception(), m_desc(desc) { }

    virtual ~ValueException() throw() { }

    /// A description of the parse error
    virtual const char* what() const throw()
    {
        return m_desc.c_str();
    }

private:
    std::string m_desc;
};


/// Exception thrown during conversion to/from other values in class Value
class ValueConversionException : public Exception
{
public:
    ValueConversionException(const std::string& desc)
        : Exception(), m_desc()
    {
        m_desc = std::string("Value conversion error: ") + desc;
    }

    virtual ~ValueConversionException() throw() { }

    /// A description of the parse error
    virtual const char* what() const throw()
    {
        return m_desc.c_str();
    }

private:
    std::string m_desc;
};


//
// Array
//

typedef std::vector< RenderSpud::Ptr<Value> >       ValueArray;
typedef std::vector< RenderSpud::Ptr<const Value> > ConstValueArray;


//
// Value
//

/// Special value denoting the node was not constructed from a file source
const size_t kNotFromFile = std::numeric_limits<size_t>::max();
/// Special value denoting there is no index for the value
const size_t kInvalidIndex = std::numeric_limits<size_t>::max();

class Value : public ReferenceCounted
{
public:
    typedef RenderSpud::Ptr<Value> Ptr;
    typedef RenderSpud::Ptr<const Value> ConstPtr;

    typedef std::pair<Value::Ptr, bool> Resolved;
    typedef std::pair<Value::ConstPtr, bool> ConstResolved;

    /// The basic type of data stored within a value.
    enum Type
    {
        kTypeInvalid,   ///< Invalid type, value cannot be used
        kTypeBoolean,   ///< Boolean value, one of true or false
        kTypeInteger,   ///< Integer value (signed)
        kTypeFloat,     ///< Floating-point value (double precision)
        kTypeString,    ///< String value, possibly with ${...} embedded references
        kTypeReference, ///< Reference value, pointing to another value by path/subscript
        kTypeMacro,     ///< Macro value, executed to produce another value
        kTypeArray,     ///< Array value with a list of member values (without names)
        kTypeBlock      ///< Block value with a list of member values with unique names
    };

public:
    /// Default construct an invalid value
    Value();

    /// Construct a boolean value
    Value(bool b);
    /// Construct an integer value
    Value(long i);
    /// Construct a float value
    Value(double f);
    /// Construct a string value
    Value(const std::string& s);
    /// Construct a macro value
    Value(MacroInvocation::Ptr pMacroInvocation);
    /// Construct a reference value
    Value(Reference::Ptr pReference);
    /// Construct an array value
    Value(ValueArray& arrayValues);
    /// Construct a block value
    Value(const std::vector<std::string>& names, ValueArray& blockValues);
    /// Construct a value of a given type (but empty/zero/etc)
    Value(Type type);


    /// Clone this value deeply to a new value.
    Value::Ptr clone() const;


    /// Block or array value eventually containing this value, if one exists.
    Value::ConstPtr context() const               { return m_pContext; }
    /// Block or array value eventually containing this value, if one exists.
    Value::Ptr      context()                     { return m_pContext; }
    /// Block or array value eventually containing this value.
    void            setContext(Value::Ptr pValue) { m_pContext = pValue; }

    /// Block or array value eventually contains this value, if one exists.
    bool hasContext() const { return m_pContext != NULL; }

    /// Utility to propagate this context of this value to its children.
    void fixupContexts();

    /// For block values, the block they inherit from (null if no inheritance).
    Value::ConstPtr inheritedBlock() const               { return m_pInheritedBlock; }
    /// For block values, the block they inherit from (null if no inheritance).
    Value::Ptr      inheritedBlock()                     { return m_pInheritedBlock; }
    /// For block values, the block they inherit from (null if no inheritance).
    void            setInheritedBlock(Value::Ptr pValue) { m_pInheritedBlock = pValue; }

    /// For values read from a file, the file it came from.
    std::string file() const { return file(m_fileIndex); }
    /// For values read from a file, the line it came from.
    size_t      line() const { return m_line; }
    /// For values read from a file, the position within the line it came from.
    size_t      pos()  const { return m_pos; }

    // For the parser: set line/pos.  Don't use these directly.
    void setLine(size_t lineNo) { m_line = lineNo; }
    void setPos(size_t posNo)   { m_pos = posNo; }
    void setFileIndex(size_t i) { m_fileIndex = i; }

    /// Does the value have source file information (did it come from parsing a file)?
    bool hasSourceInfo() const { return m_fileIndex != kNotFromFile; }


    /// Basic type the value holds: bool, long, double, string, ref, macro, block, or array.
    /// This is the type of the value BEFORE resolving it.
    ///
    /// Use Value::canConvertTo(Type) to see if the value can be resolved to
    /// another Type as well.
    Type type() const { return m_type; }

    /// The type name for the value.  This is not the same as the basic type of
    /// the value, but indicates its use by the application.  It is composed of
    /// strings separated by dots indicating type namespaces.  When stored here
    /// the dots are dropped and a vector of strings is used instead.
    const TypeName& typeName() const    { return m_typeName; }
    /// The type name for the value.  This is not the same as the basic type of
    /// the value, but indicates its use by the application.  It is composed of
    /// strings separated by dots indicating type namespaces.  When stored here
    /// the dots are dropped and a vector of strings is used instead.
    TypeName&       typeName()          { return m_typeName; }
    /// The type name for the value.  This is not the same as the basic type of
    /// the value, but indicates its use by the application.  It is composed of
    /// strings separated by dots indicating type namespaces.  When stored here
    /// the dots are dropped and a vector of strings is used instead.
    void setTypeName(const TypeName& t) { m_typeName = t; }

    /// Check if the type name matches a string (of the form "type.subtype.subsubtype...")
    bool typeNameMatches(const std::string& typeName);
    /// Check if the type name matches another.
    bool typeNameMatches(const TypeName& typeName);

    /// Whether the value has a type name attached or not.
    bool hasTypeName() const { return !m_typeName.empty(); }


    //
    // Name / index / path from context
    //
    // (Note: these are not stored in the value itself, so they are invalid
    // unless this value is added/set on another value!)
    //

    /// If this value is a member of a block, this is the assigned name.  Note:
    /// this is invalid if this value is not a member of a block.
    std::string name() const;
    /// If this value is a member of a block, set the assigned name.  Note:
    /// this is invalid if this value is not a member of a block.
    void setName(const std::string& n);
    /// Tell if this value is a member of a block.
    bool hasName() const;

    /// As a member of a block or array, this is the index this value lives at.
    size_t index() const;

    /// Get the fully-qualified path to this node (suitable for use in \ref find()).
    /// Note that even if a value has a context, it may not have a discernable
    /// path: a subscript in a reference, or an argument to a macro do not,
    /// while members of blocks and arrays that are only ever inside other
    /// blocks and arrays do have a discernable path.
    std::string path() const;


    //
    // Convenience information
    //

    // A value may return true for more than one of the following (depending on
    // its actual internal value)

    /// This value is convertible to boolean
    bool isBoolean()   const;
    /// This value is convertible to integer
    bool isInteger()   const;
    /// This value is convertible to float
    bool isFloat()     const;
    /// This value is convertible to string
    bool isString()    const;
    /// This value is convertible to a block (and nothing else)
    bool isBlock()     const;
    /// This value is convertible to an array (and nothing else)
    bool isArray()     const;

    // If one of the following two are true, then this value *may* resolve to
    // one of the above types.  This means that a value may return true for
    // more than one of the above, and at most one of isReference() or isMacro()

    /// Is this value a reference to another value?  If so, it may also convert
    /// to other value types as well (but not a macro).
    bool isReference() const;
    /// Is this value a macro?  If so, it may also convert to other value types
    /// as well (but not a reference).
    bool isMacro()     const;

    /// Can this value be converted to a value of another specified type?
    bool canConvertTo(Type type) const;

    /// Is this value fully evaluated, and contains no references, macros, etc?
    bool isFullyEvaluated() const;
    /// Is this value a member of a block value?
    bool isInsideBlock()    const;
    /// Is this value a member of an array value?
    bool isInsideArray()    const;

    /// Is this value valid and of a known type?
    bool isValid() const { return m_type != kTypeInvalid; }

    /// Does this value boil down to the null value?
    bool isNull() const;

    /// The null value is predefined and always exists.  Get it here.
    static Value::ConstPtr nullValue();

    /// Is this value an include, referencing another RSD file?
    bool isInclude() const;
    /// Is this block value inheriting another block value?
    bool inheritsBlock() const { return m_pInheritedBlock != NULL; }

    /// Discover if all references, macros, etc are resolvable.
    bool allValuesResolvable(ValueArray *pArray = NULL, bool recursive = true);

    /// Resolve another value in the context of this value (usually the member
    /// of an array or block value).
    virtual ConstResolved resolve(const Value& v) const;
    /// Resolve another value in the context of this value (usually the member
    /// of an array or block value).
    virtual Resolved      resolve(Value& v);

    /// Resolve this value as fully as possible, removing references, macros, etc.
    virtual Value::Ptr resolved() { return resolve(*this).first; }


    //
    // Conversion
    //

    // The following will resolve references/macros to get at the final value

    /// Resolve references and macros to get at a final boolean value.  If not, throw \ref ValueConversionException.
    bool             asBoolean() const;
    /// Resolve references and macros to get at a final integer value.  If not, throw \ref ValueConversionException.
    long             asInteger() const;
    /// Resolve references and macros to get at a final float value.  If not, throw \ref ValueConversionException.
    double           asFloat()   const;
    /// Resolve references, macros, and strings to get at a final string value.  If not, throw \ref ValueConversionException.
    std::string      asString()  const;
    /// Resolve references and macros to get at a final array value.  If not, throw \ref ValueConversionException.
    Value::ConstPtr  asArray()   const;
    /// Resolve references and macros to get at a final array value.  If not, throw \ref ValueConversionException.
    Value::Ptr       asArray();
    /// Resolve references and macros to get at a final block value.  If not, throw \ref ValueConversionException.
    Value::ConstPtr  asBlock()   const;
    /// Resolve references and macros to get at a final block value.  If not, throw \ref ValueConversionException.
    Value::Ptr       asBlock();
    /// Resolve to a block, and then inline all inherited and included values
    /// directly into the block (useful for using size()/value(...) iteration afterwards)
    Value::Ptr       asInlinedBlock();

    // The following never resolve references/macros; they look at *this* value only

    /// Get the raw unresolved string value.  If it is not a string value, throw \ref ValueConversionException.
    std::string               asRawString()    const;
    /// Get the raw unresolved macro value.  If it is not a macro value, throw \ref ValueConversionException.
    MacroInvocation::ConstPtr asRawMacro()     const;
    /// Get the raw unresolved macro value.  If it is not a macro value, throw \ref ValueConversionException.
    MacroInvocation::Ptr      asRawMacro();
    /// Get the raw unresolved reference value.  If it is not a reference value, throw \ref ValueConversionException.
    Reference::ConstPtr       asRawReference() const;
    /// Get the raw unresolved reference value.  If it is not a reference value, throw \ref ValueConversionException.
    Reference::Ptr            asRawReference();


    //
    // Aggregate value access (arrays, blocks)
    //

    /// Number of values inside this value (for arrays and blocks)
    size_t size() const { return (m_type == kTypeArray || m_type == kTypeBlock) ? m_values.size() : 0; }

    // Raw values access; only use this if you know what you are doing
    const ValueArray& values() const { return m_values; }


    // Array access / management

    /// \brief Access an array value by index.
    ///
    /// Returns NULL when the index is out of range.  Note this works for block
    /// values too (but beware, include values are in the list!)
    /// @param i Value index
    Value::ConstPtr value(size_t i) const;
    /// \brief Access an array value by index.
    ///
    /// Returns NULL when the index is out of range.  Note this works for block
    /// values too (but beware, include values are in the list!)
    /// @param i Value index
    Value::Ptr      value(size_t i);

    /// \brief Access an array value by index.
    ///
    /// Throws a \ref ValueException when the index is out of range.  Note this
    /// works for block values too (but beware, include values are in the list!)
    /// @param i Value index
    const Value& operator [](size_t i) const;
    /// \brief Access an array value by index.
    ///
    /// Throws a \ref ValueException when the index is out of range.  Note this
    /// works for block values too (but beware, include values are in the list!)
    /// @param i Value index
    Value&       operator [](size_t i);

    /// Remove a value by index in an array or block.
    void removeValue(size_t i);
    /// Set a value by index in an array or block.
    void setValue(size_t i, Value::Ptr pValue);
    /// Insert a value into an array.  Note: does not work on blocks.
    void insertValue(size_t i, Value::Ptr pValue);
    /// Append a value to an array.  Note: does not work on blocks.
    void appendValue(Value::Ptr pValue);

    // Named variable access / management (for block values)

    /// Access a value in the block by name.  Note: does not work on arrays.
    /// @param name            Name of the value to access.
    /// @param searchIncludes  Optional, whether to look for the value in includes if not found locally.
    /// @param searchInherited Optional, whether to look for the value in inherited blocks if not found locally.
    Value::ConstPtr value(const std::string& name,
                          bool searchIncludes = true,
                          bool searchInherited = true) const;
    /// Access a value in the block by name.  Note: does not work on arrays.
    /// @param name            Name of the value to access.
    /// @param searchIncludes  Optional, whether to look for the value in includes if not found locally.
    /// @param searchInherited Optional, whether to look for the value in inherited blocks if not found locally.
    Value::Ptr      value(const std::string& name,
                          bool searchIncludes = true,
                          bool searchInherited = true);

    /// Access a value in the block by name, looking in includes and inherited blocks too.  Note: does not work on arrays.
    const Value& operator [](const std::string& name) const;
    /// Access a value in the block by name, looking in includes and inherited blocks too.  Note: does not work on arrays.
    Value&       operator [](const std::string& name);

    /// Remove a value by name.  Note: does not work on arrays.
    void removeValue(const std::string& name);
    /// Set a value by name.  Note: does not work on arrays.
    void setValue(const std::string& name, Value::Ptr pValue);
    /// Insert a value by name.  Note: does not work on arrays.
    void insertValue(const std::string& before,
                     const std::string& name,
                     Value::Ptr pValue);
    /// Append a value by name.  Note: does not work on arrays.
    void appendValue(const std::string& name, Value::Ptr pValue);

    /// List value names in a block.  Note: is empty for arrays and other values.
    const std::vector<std::string>& names() const { return m_blockValueNames; }
    /// List value names in a block.  Note: is empty for arrays and other values.
    std::vector<std::string>&       names()       { return m_blockValueNames; }

    /// Utility to check if a name is in a standard format, and doesn't require quoting when serialized.
    static bool isNameStandardFormat(const std::string& name);


    //
    // Search / filtering (by TypeName, Reference/string)
    //

    /// Find a value by following a reference
    virtual Value::ConstPtr find(const Reference& ref) const;
    /// Find a value by following a reference
    virtual Value::Ptr      find(const Reference& ref);

    /// Find a value by parsing and following a reference, throws \ref ParseException if parsing fails.
    Value::ConstPtr find(const std::string& refString) const
    {
        Reference::Ptr pRef = Reference::fromString(refString);
        return find(*pRef);
    }

    /// Find a value by parsing and following a reference, throws \ref ParseException if parsing fails.
    Value::Ptr find(const std::string& refString)
    {
        Reference::Ptr pRef = Reference::fromString(refString);
        return find(*pRef);
    }

    /// Find all values in this value that have the specified type name.
    ConstValueArray findByTypeName(const TypeName& typeName,
                                   bool searchIncludes = true,
                                   bool searchInherited = true) const;
    /// Find all values in this value that have the specified type name.
    ValueArray      findByTypeName(const TypeName& typeName,
                                   bool searchIncludes = true,
                                   bool searchInherited = true);


    //
    // Textual representation
    //

    /// Serialize this node's data to a string.
    /// @param followIncludes Optional, whether includes should be replaced by the contents of each include file.
    /// @param keepInline     Optional, whether to keep all output on one line.
    /// @param indentation    Optional, how far to indent the written data, useful for indicating nesting (internally, 4 spaces are used for each recursive indentation level).
    virtual std::string str(bool followIncludes = false,
                            bool keepInline = false,
                            size_t indentation = 0) const;


protected:
    typedef std::map<std::string, Value::Ptr> ValueMap;
    typedef size_t FileIndex;


    Value(const Value& v);

    virtual ~Value() { m_pContext.reset(); }


    /// Map a file index to a filename, if available
    virtual std::string file(FileIndex index) const { return m_pContext != NULL ? m_pContext->file(index) : ""; }


    static const Value::ConstPtr m_spNull; // Predefined null value


    Value::Ptr m_pContext;
    Value::Ptr m_pInheritedBlock;
    Type m_type;
    TypeName m_typeName;
    size_t m_line, m_pos;
    FileIndex m_fileIndex;

    // The value will fill out the following discriminated based on m_type
    ValueArray m_values;
    std::vector<std::string> m_blockValueNames;
    Reference::Ptr m_pReference;
    MacroInvocation::Ptr m_pMacroInvocation;
    std::string m_string;
    union
    {
        bool m_boolean;
        long m_integer;
        double m_float;
    };
};


    } // namespace Rsd
} // namespace RenderSpud


#endif // __RSD_Value_h__
