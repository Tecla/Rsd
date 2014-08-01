////////////
//
//  File:      Value.cpp
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data values
//
////////////

#include <sstream>
#include <cmath>

#include <Rsd/Value.h>
#include <Rsd/Parser.h>


namespace RenderSpud
{
    namespace Rsd
    {


const Value::ConstPtr Value::m_spNull = new Value();


Value::Value()
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(),
      m_type(kTypeInvalid), m_typeName(), m_line(0), m_pos(0),
      m_fileIndex(kInvalidIndex), m_values(), m_blockValueNames(),
      m_pReference(), m_pMacroInvocation(), m_string(), m_integer(0)
{

}


Value::Value(const Value& v)
    : ReferenceCounted(), m_pContext(v.m_pContext),
      m_pInheritedBlock(v.m_pInheritedBlock), m_type(v.m_type),
      m_typeName(v.m_typeName), m_line(0), m_pos(0), m_fileIndex(kInvalidIndex),
      m_values(), m_blockValueNames(), m_pReference(), m_pMacroInvocation(),
      m_string(), m_integer(0)
{
    if (m_type == kTypeBoolean) m_boolean = v.m_boolean;
    else if (m_type == kTypeInteger) m_integer = v.m_integer;
    else if (m_type == kTypeFloat) m_float = v.m_float;
    else if (m_type == kTypeString) m_string = v.m_string;
    else if (m_type == kTypeReference)
    {
        m_pReference = v.m_pReference->clone();
    }
    else if (m_type == kTypeMacro)
    {
        m_pMacroInvocation = v.m_pMacroInvocation->clone();
    }
    else if (m_type == kTypeArray || m_type == kTypeBlock)
    {
        if (m_type == kTypeBlock)
        {
            m_blockValueNames = v.m_blockValueNames;
        }
        m_values.reserve(v.m_values.size());
        for (size_t i = 0; i < v.m_values.size(); ++i)
        {
            m_values.push_back(v.m_values[i]->clone());
        }
    }
    fixupContexts();
}


Value::Value(Type type)
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(), m_type(type),
      m_typeName(), m_line(0), m_pos(0), m_fileIndex(kInvalidIndex), m_values(),
      m_blockValueNames(), m_pReference(), m_pMacroInvocation(), m_string(),
      m_integer(0)
{

}


Value::Value(bool b)
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(),
      m_type(kTypeBoolean), m_typeName(), m_line(0), m_pos(0),
      m_fileIndex(kInvalidIndex), m_values(), m_blockValueNames(),
      m_pReference(), m_pMacroInvocation(), m_string(), m_boolean(b)
{

}


Value::Value(long i)
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(),
      m_type(kTypeInteger), m_typeName(), m_line(0), m_pos(0),
      m_fileIndex(kInvalidIndex), m_values(), m_blockValueNames(),
      m_pReference(), m_pMacroInvocation(), m_string(), m_integer(i)
{

}


Value::Value(double f)
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(),
      m_type(kTypeFloat), m_typeName(), m_line(0), m_pos(0),
      m_fileIndex(kInvalidIndex), m_values(), m_blockValueNames(),
      m_pReference(), m_pMacroInvocation(), m_string(), m_float(f)
{

}


Value::Value(const std::string& s)
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(),
      m_type(kTypeString), m_typeName(), m_line(0), m_pos(0),
      m_fileIndex(kInvalidIndex), m_values(), m_blockValueNames(),
      m_pReference(), m_pMacroInvocation(), m_string(s), m_integer(0)
{

}


Value::Value(MacroInvocation::Ptr pMacroInvocation)
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(),
      m_type(kTypeMacro), m_typeName(), m_line(0), m_pos(0),
      m_fileIndex(kInvalidIndex), m_values(), m_blockValueNames(),
      m_pReference(), m_pMacroInvocation(pMacroInvocation), m_string(),
      m_integer(0)
{

}


Value::Value(Reference::Ptr pReference)
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(),
      m_type(kTypeReference), m_typeName(), m_line(0), m_pos(0),
      m_fileIndex(kInvalidIndex), m_values(), m_blockValueNames(),
      m_pReference(pReference), m_pMacroInvocation(), m_string(), m_integer(0)
{

}


Value::Value(ValueArray& arrayValues)
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(),
      m_type(kTypeArray), m_typeName(), m_line(0), m_pos(0),
      m_fileIndex(kInvalidIndex), m_values(arrayValues), m_blockValueNames(),
      m_pReference(), m_pMacroInvocation(), m_string(), m_integer(0)
{

}


Value::Value(const std::vector<std::string>& names, ValueArray& blockValues)
    : ReferenceCounted(), m_pContext(NULL), m_pInheritedBlock(),
      m_type(kTypeBlock), m_typeName(), m_line(0), m_pos(0),
      m_fileIndex(kInvalidIndex), m_values(blockValues),
      m_blockValueNames(names), m_pReference(), m_pMacroInvocation(),
      m_string(), m_integer(0)
{

}


Value::Ptr Value::clone() const
{
    return new Value(*this);
}


void Value::fixupContexts()
{
    if (m_type == kTypeReference && m_pReference)
    {
        for (Reference::PartsList::iterator iter = m_pReference->parts().begin();
             iter != m_pReference->parts().end();
             ++iter)
        {
            if (iter->m_pSubscriptValue)
            {
                iter->m_pSubscriptValue->setContext(m_pContext);
                iter->m_pSubscriptValue->fixupContexts();
            }
        }
    }
    else if (m_type == kTypeMacro && m_pMacroInvocation)
    {
        for (MacroInvocation::ArgumentValueMap::iterator iter = m_pMacroInvocation->arguments().begin();
             iter != m_pMacroInvocation->arguments().end();
             ++iter)
        {
            iter->second->setContext(m_pContext);
            iter->second->fixupContexts();
        }
    }
    else if (m_type == kTypeArray || m_type == kTypeBlock)
    {
        for (size_t i = 0; i < m_values.size(); ++i)
        {
            m_values[i]->setContext(this);
            m_values[i]->fixupContexts();
        }
    }
}


bool Value::typeNameMatches(const std::string& typeName)
{
    return typeNameToString(m_typeName) == typeName;
}


bool Value::typeNameMatches(const TypeName& typeName)
{
    return typeNameMatches(typeNameToString(typeName));
}


std::string Value::name() const
{
    if (m_pContext && m_pContext->type() == kTypeBlock)
    {
        for (size_t i = 0; i < m_pContext->size(); ++i)
        {
            if (m_pContext->value(i) == this)
            {
                return m_pContext->names()[i];
            }
        }
    }
    return std::string();
}


bool Value::hasName() const
{
    if (m_pContext && m_pContext->type() == kTypeBlock)
    {
        for (size_t i = 0; i < m_pContext->size(); ++i)
        {
            if (m_pContext->value(i) == this)
            {
                return true;
            }
        }
    }
    return false;
}


void Value::setName(const std::string& n)
{
    size_t existingIndex = kInvalidIndex;
    if (m_pContext && m_pContext->type() == kTypeBlock)
    {
        for (size_t i = 0; i < m_pContext->size(); ++i)
        {
            if (m_pContext->names()[i] == n && m_pContext->value(i) != this)
            {
                throw ValueException("Could not set the name of the value; another value already has that name!");
            }

            if (m_pContext->value(i) == this)
            {
                existingIndex = i;
            }
        }
    }
    if (existingIndex != kInvalidIndex)
    {
        m_pContext->names()[existingIndex] = n;
    }
    else
    {
        throw ValueException("Could not set the name of the value; it is not inside a block!");
    }
}


size_t Value::index() const
{
    if (m_pContext && m_pContext->type() == kTypeArray)
    {
        for (size_t i = 0; i < m_pContext->size(); ++i)
        {
            if (m_pContext->value(i) == this)
            {
                return i;
            }
        }
    }
    return kInvalidIndex;
}


std::string Value::path() const
{
    std::ostringstream stream;

    if (m_pContext)
    {
        // Are we inside of an array?
        size_t arrayIndex = index();
        if (arrayIndex != kInvalidIndex)
        {
            stream << '[' << arrayIndex << ']';
            return m_pContext->path() + stream.str();
        }
        // Or are we inside a block?
        std::string varName = name();
        if (!varName.empty())
        {
            if (m_pContext->isInsideBlock() || m_pContext->isInsideArray())
            {
                // And our block is inside of something else
                stream << '.';
            }
            stream << varName;
            return m_pContext->path() + stream.str();
        }
    }
    // Anything else is inaccessible by path
    return std::string();
}


bool Value::isBoolean() const
{
    ConstResolved result = resolve(*this);
    return result.first->m_type == kTypeBoolean;
}


bool Value::isInteger() const
{
    ConstResolved result = resolve(*this);
    return result.first->m_type == kTypeInteger;
}


bool Value::isFloat() const
{
    ConstResolved result = resolve(*this);
    // Can implicitly upcast from integer to float
    return result.first->m_type == kTypeInteger || result.first->m_type == kTypeFloat;
}


bool Value::isString() const
{
    ConstResolved result = resolve(*this);
    // Can implicitly upcast from boolean, integer, and float to string
    return result.first->m_type == kTypeBoolean ||
           result.first->m_type == kTypeInteger ||
           result.first->m_type == kTypeFloat ||
           result.first->m_type == kTypeString;
}


bool Value::isBlock() const
{
    ConstResolved result = resolve(*this);
    return result.first->m_type == kTypeBlock;
}


bool Value::isArray() const
{
    ConstResolved result = resolve(*this);
    return result.first->m_type == kTypeArray;
}


bool Value::isReference() const
{
    return m_type == kTypeReference;
}


bool Value::isMacro() const
{
    return m_type == kTypeMacro;
}


bool Value::canConvertTo(Type type) const
{
    // Are they asking for types that occur before resolving?
    if (type == kTypeReference || type == kTypeMacro) return m_type == type;

    // The rest of the types may need to be resolved first
    ConstResolved result = resolve(*this);
    if (type == kTypeBoolean)      return result.first->isBoolean();
    else if (type == kTypeInteger) return result.first->isInteger();
    else if (type == kTypeFloat)   return result.first->isFloat();
    else if (type == kTypeString)  return result.first->isString();
    else if (type == kTypeArray)   return result.first->isArray();
    else if (type == kTypeBlock)   return result.first->isBlock();
    // Not a defined type?  If it's the 'null' value, it resolves.
    else                           return (result.first == m_spNull) ? true : false;
}


bool Value::isFullyEvaluated() const
{
    if (m_type == kTypeBoolean || m_type == kTypeInteger || m_type == kTypeFloat ||
        m_type == kTypeArray || m_type == kTypeBlock || m_type == kTypeInvalid)
    {
        return true;
    }
    else if (m_type == kTypeReference || m_type == kTypeMacro)
    {
        return false;
    }
    else // if (m_type == kTypeString)
    {
        // Check if there's still a ${...} in there
        size_t varStartIndex = m_string.find("${", 0);
        if (varStartIndex != std::string::npos &&
            m_string.find('}', varStartIndex) != std::string::npos)
        {
            return false;
        }
        return true;
    }
}


bool Value::isInsideBlock() const
{
    return hasName();
}


bool Value::isInsideArray() const
{
    if (m_pContext && m_pContext->type() == kTypeArray)
    {
        for (size_t i = 0; i < m_pContext->size(); ++i)
        {
            if (m_pContext->value(i) == this)
            {
                return true;
            }
        }
    }
    return false;
}


bool Value::isNull() const
{
    ConstResolved result = resolve(*this);
    return (result.first == m_spNull) ? true : false;
}


Value::ConstPtr Value::nullValue()
{
    return m_spNull;
}


bool Value::isInclude() const
{
    return m_typeName.size() == 1 && m_typeName[0] == "include";
}


bool Value::allValuesResolvable(ValueArray *pArray, bool recursive)
{
    if (m_type == kTypeReference || m_type == kTypeMacro || m_type == kTypeString)
    {
        try
        {
            // may be thrown during string resolving?
            Resolved thisResolved = resolve(*this);
            if (!thisResolved.second)
            {
                return false;
            }
            return recursive ? thisResolved.first->allValuesResolvable(pArray, recursive) : true;
        }
        catch (ValueConversionException)
        {
            return false;
        }
    }
    if (!pArray)
    {
        pArray = &m_values;
    }
    for (size_t i = 0; i < pArray->size(); ++i)
    {
        if (recursive)
        {
            if (!pArray->at(i)->allValuesResolvable(NULL, true))
            {
                return false;
            }
        }
    }
    return true;
}


bool Value::asBoolean() const
{
    ConstResolved result = resolve(*this);
    if (result.first->m_type != kTypeBoolean)
    {
        throw ValueConversionException("Cannot convert resolved value to a boolean!");
    }
    return result.first->m_boolean;
}


long Value::asInteger() const
{
    ConstResolved result = resolve(*this);
    if (result.first->m_type != kTypeInteger)
    {
        throw ValueConversionException("Cannot convert resolved value to an integer!");
    }
    return result.first->m_integer;
}


double Value::asFloat() const
{
    ConstResolved result = resolve(*this);
    if (result.first->m_type == kTypeInteger)
    {
        return static_cast<double>(result.first->m_integer);
    }
    else if (result.first->m_type == kTypeFloat)
    {
        return result.first->m_float;
    }

    throw ValueConversionException("Cannot convert resolved value to a float!");
    return 0.0f;
}


std::string Value::asString() const
{
    ConstResolved result = resolve(*this);
    if (result.first->m_type == kTypeBoolean)
    {
        return result.first->m_boolean ? std::string("true") : std::string("false");
    }
    else if (result.first->m_type == kTypeInteger)
    {
        std::ostringstream numStream;
        numStream << result.first->m_integer;
        return numStream.str();
    }
    else if (result.first->m_type == kTypeFloat)
    {
        std::ostringstream numStream;
        numStream << result.first->m_float;
        return numStream.str();
    }
    else if (result.first->m_type == kTypeString)
    {
        return result.first->m_string;
    }

    throw ValueConversionException("Cannot convert resolved value to a string!");
    return std::string();
}


Value::ConstPtr Value::asArray() const
{
    ConstResolved result = resolve(*this);
    if (result.first->m_type != kTypeArray)
    {
        throw ValueConversionException("Cannot convert resolved value to an array!");
    }
    return result.first;
}


Value::Ptr Value::asArray()
{
    Resolved result = resolve(*this);
    if (result.first->m_type != kTypeArray)
    {
        throw ValueConversionException("Cannot convert resolved value to an array!");
    }
    return result.first;
}


Value::ConstPtr Value::asBlock() const
{
    ConstResolved result = resolve(*this);
    if (result.first->m_type != kTypeBlock)
    {
        throw ValueConversionException("Cannot convert resolved value to a block!");
    }
    return result.first;
}


Value::Ptr Value::asBlock()
{
    Resolved result = resolve(*this);
    if (result.first->m_type != kTypeBlock)
    {
        throw ValueConversionException("Cannot convert resolved value to a block!");
    }
    return result.first;
}


Value::Ptr Value::asInlinedBlock()
{
    Value::Ptr pBlock = asBlock();
    Value::Ptr pInlinedBlock = new Value(kTypeBlock);

    for (size_t i = 0; i < pBlock->size(); ++i)
    {
        if (!pBlock->value(i)->isInclude())
        {
            continue;
        }

        Value::Ptr pIncludeInlinedBlock = pBlock->value(i)->asInlinedBlock();
        for (size_t j = 0; j < pIncludeInlinedBlock->size(); ++j)
        {
            pInlinedBlock->setValue(pIncludeInlinedBlock->names()[j],
                                    pIncludeInlinedBlock->value(j)->clone());
        }
    }

    if (pBlock->inheritsBlock())
    {
        Value::Ptr pInheritedInlinedBlock = pBlock->inheritedBlock()->asInlinedBlock();
        for (size_t j = 0; j < pInheritedInlinedBlock->size(); ++j)
        {
            pInlinedBlock->setValue(pInheritedInlinedBlock->names()[j],
                                    pInheritedInlinedBlock->value(j)->clone());
        }
    }

    for (size_t j = 0; j < pBlock->size(); ++j)
    {
        if (pBlock->value(j)->isInclude())
            continue;

        pInlinedBlock->setValue(pBlock->names()[j], pBlock->value(j)->clone());
    }

    return pInlinedBlock;
}


std::string Value::asRawString() const
{
    if (m_type != kTypeString)
    {
        throw ValueConversionException("Cannot convert raw value to a string!");
    }
    return m_string;
}


MacroInvocation::ConstPtr Value::asRawMacro() const
{
    if (m_type != kTypeMacro)
    {
        throw ValueConversionException("Cannot convert raw value to a macro!");
    }
    return m_pMacroInvocation;
}


MacroInvocation::Ptr Value::asRawMacro()
{
    if (m_type != kTypeMacro)
    {
        throw ValueConversionException("Cannot convert raw value to a macro!");
    }
    return m_pMacroInvocation;
}


Reference::ConstPtr Value::asRawReference() const
{
    if (m_type != kTypeReference)
    {
        throw ValueConversionException("Cannot convert raw value to a reference!");
    }
    return m_pReference;
}


Reference::Ptr Value::asRawReference()
{
    if (m_type != kTypeReference)
    {
        throw ValueConversionException("Cannot convert raw value to a reference!");
    }
    return m_pReference;
}


Value::ConstPtr Value::value(size_t i) const
{
    return m_values.size() > i ? m_values[i] : NULL;
}


Value::Ptr Value::value(size_t i)
{
    return m_values.size() > i ? m_values[i] : NULL;
}


const Value& Value::operator [](size_t i) const
{
    if (m_values.size() <= i)
    {
        throw ValueException("Index out of range in Value::operator[](size_t) const");
    }
    return *m_values[i];
}


Value& Value::operator [](size_t i)
{
    if (m_values.size() <= i)
    {
        throw ValueException("Index out of range in Value::operator[](size_t)");
    }
    return *m_values[i];
}


void Value::removeValue(size_t i)
{
    if (m_type != kTypeArray && m_type != kTypeBlock)
    {
        throw ValueException("Cannot remove values by index on non-array/non-block values!");
    }
    if (i >= m_values.size())
    {
        throw ValueException("Index out of range in Value::removeValue(size_t)");
    }
    if (m_type == kTypeBlock)
    {
        removeValue(m_blockValueNames[i]);
    }
    else
    {
        m_values[i]->setContext(NULL);
        m_values.erase(m_values.begin() + i);
    }
}


void Value::setValue(size_t i, Value::Ptr pValue)
{
    if (m_type != kTypeArray && m_type != kTypeBlock)
    {
        throw ValueException("Cannot set values by index on non-array/non-block values!");
    }
    if (i >= m_values.size())
    {
        throw ValueException("Index out of range in Value::setValue(size_t)");
    }
    pValue->setContext(this);
    m_values[i] = pValue;
}


void Value::insertValue(size_t i, Value::Ptr pValue)
{
    if (m_type != kTypeArray)
    {
        throw ValueException("Cannot insert indexed values into non-array values!");
    }
    if (i >= m_values.size() - 1)
    {
        throw ValueException("Index out of range in Value::setValue(size_t)");
    }
    pValue->setContext(this);
    m_values.insert(m_values.begin() + i, pValue);
}


void Value::appendValue(Value::Ptr pValue)
{
    if (m_type != kTypeArray)
    {
        throw ValueException("Cannot append indexed values into non-array values!");
    }
    pValue->setContext(this);
    m_values.push_back(pValue);
}


Value::ConstPtr Value::value(const std::string& name,
                             bool searchIncludes,
                             bool searchInherited) const
{
    if (m_type != kTypeBlock)
    {
        throw ValueException("Cannot get named values from non-block values!");
    }
    for (size_t i = 0; i < m_blockValueNames.size(); ++i)
    {
        if (m_blockValueNames[i] == name)
        {
            return m_values[i];
        }
        else if (searchIncludes &&
                 m_values[i]->isInclude() &&
                 m_values[i]->type() == kTypeBlock)
        {
            Value::Ptr pIncludedVal = m_values[i]->value(name);
            if (pIncludedVal)
            {
                return pIncludedVal;
            }
        }
        // Haven't found it yet?  Give inherited block a shot at resolving it.
        if (searchInherited && m_pInheritedBlock != NULL)
        {
            return m_pInheritedBlock->asBlock()->value(name, searchIncludes);
        }
    }
    return NULL;
}


Value::Ptr Value::value(const std::string& name,
                        bool searchIncludes,
                        bool searchInherited)
{
    if (m_type != kTypeBlock)
    {
        throw ValueException("Cannot get named values from non-block values!");
    }
    for (size_t i = 0; i < m_blockValueNames.size(); ++i)
    {
        if (m_blockValueNames[i] == name)
        {
            return m_values[i];
        }
        else if (searchIncludes &&
                 m_values[i]->isInclude() &&
                 m_values[i]->type() == kTypeBlock)
        {
            Value::Ptr pIncludedVal = m_values[i]->value(name);
            if (pIncludedVal)
            {
                return pIncludedVal;
            }
        }
        // Haven't found it yet?  Give inherited block a shot at resolving it.
        if (searchInherited && m_pInheritedBlock != NULL)
        {
            return m_pInheritedBlock->asBlock()->value(name, searchIncludes);
        }
    }
    return NULL;
}


const Value& Value::operator [](const std::string& name) const
{
    Value::ConstPtr pValue = value(name);
    if (pValue == NULL)
    {
        throw ValueException("Named value not found in Value::operator[](const std::string&) const");
    }
    return *pValue;
}


Value& Value::operator [](const std::string& name)
{
    Value::Ptr pValue = value(name);
    if (pValue == NULL)
    {
        throw ValueException("Named value not found in Value::operator[](const std::string&)");
    }
    return *pValue;
}


void Value::removeValue(const std::string& name)
{
    if (m_type != kTypeBlock)
    {
        throw ValueException("Cannot get named values from non-block values!");
    }
    for (size_t i = 0; i < m_blockValueNames.size(); ++i)
    {
        if (m_blockValueNames[i] == name)
        {
            m_values[i]->setContext(NULL);
            m_values.erase(m_values.begin() + i);
            m_blockValueNames.erase(m_blockValueNames.begin() + i);
            return;
        }
    }
    throw ValueException(std::string("Value of name \"") + name +
                             "\" doesn't exist in the block!");
}


void Value::setValue(const std::string& name, Value::Ptr pValue)
{
    if (m_type != kTypeBlock)
    {
        throw ValueException("Cannot get named values from non-block values!");
    }
    if (value(name, false, false))
    {
        for (size_t i = 0; i < m_blockValueNames.size(); ++i)
        {
            if (m_blockValueNames[i] == name)
            {
                m_values[i] = pValue;
                pValue->setContext(this);
                return;
            }
        }
    }
    else
    {
        appendValue(name, pValue);
    }
}


void Value::insertValue(const std::string& before,
                        const std::string& name,
                        Value::Ptr pValue)
{
    if (m_type != kTypeBlock)
    {
        throw ValueException("Cannot get named values from non-block values!");
    }
    if (value(name, false, false))
    {
        throw ValueException(std::string("Value of name \"") + name +
                                 "\" already exists in the block!");
    }
    for (size_t i = 0; i < m_blockValueNames.size(); ++i)
    {
        if (m_blockValueNames[i] == before)
        {
            m_values.insert(m_values.begin() + (i - 1), pValue);
            m_blockValueNames.insert(m_blockValueNames.begin() + (i - 1), name);
            pValue->setContext(this);
            return;
        }
    }
    throw ValueException(std::string("Value of name \"") + before +
                             "\" doesn't exist in the block!");
}

void Value::appendValue(const std::string& name, Value::Ptr pValue)
{
    if (m_type != kTypeBlock)
    {
        throw ValueException("Cannot get named values from non-block values!");
    }
    if (value(name, false, false))
    {
        throw ValueException(std::string("Value of name \"") + name +
                                 "\" already exists in the block!");
    }
    m_values.push_back(pValue);
    m_blockValueNames.push_back(name);
    pValue->setContext(this);
}


Value::ConstPtr Value::find(const Reference& ref) const
{
    // Start with what we might refer to
    ConstResolved thisResolved = resolve(*this);

    // Sanity check
    if (thisResolved.first->type() != kTypeBlock &&
        thisResolved.first->type() != kTypeArray)
    {
        return NULL;
    }

    try
    {
        // Go through each part of the reference, resolving/indexing as we go
        ConstPtr pCurrentValue = thisResolved.first;
        for (Reference::PartsList::const_iterator iter = ref.parts().begin();
             iter != ref.parts().end() && pCurrentValue != NULL;
             ++iter)
        {
            Reference::PartType pt = Reference::getPartType(*iter);
            ConstPtr partValue;
            if (pt == Reference::kPartIdentifier)
            {
                // Identifiers are looked up by name
                partValue = pCurrentValue->value(iter->m_identifier);
            }
            else
            {
                // Subscript is a value?  Resolve it to find out what kind of
                // subscript it is.
                ConstResolved subscriptResolved = resolve(*iter->m_pSubscriptValue);
                if (!subscriptResolved.second)
                {
                    // Didn't resolve?  Let partValue stay NULL, so we exit the loop.
                }
                else if (subscriptResolved.first->isInteger())
                {
                    // Integer subscript
                    partValue = pCurrentValue->value(static_cast<size_t>(subscriptResolved.first->asInteger()));
                }
                else if (subscriptResolved.first->type() == kTypeString)
                {
                    // String subscript (look up values by name)
                    partValue = pCurrentValue->value(subscriptResolved.first->asString());
                }
            }
            // Move on to the next part (if partValue is NULL, we'll give up)
            pCurrentValue = partValue;
        }
        return pCurrentValue;
    }
    catch (ValueException&)
    {
        // Just eat the exception
    }
    return NULL;
}


Value::Ptr Value::find(const Reference& ref)
{
    // Start with what we might refer to
    Resolved thisResolved = resolve(*this);

    // Sanity check
    if (thisResolved.first->type() != kTypeBlock &&
        thisResolved.first->type() != kTypeArray)
    {
        return NULL;
    }

    try
    {
        // Go through each part of the reference, resolving/indexing as we go
        Value::Ptr pCurrentValue = thisResolved.first;
        for (Reference::PartsList::const_iterator iter = ref.parts().begin();
             iter != ref.parts().end() && pCurrentValue != NULL;
             ++iter)
        {
            Reference::PartType pt = Reference::getPartType(*iter);
            Ptr partValue;
            if (pt == Reference::kPartIdentifier)
            {
                // Identifiers are looked up by name
                partValue = pCurrentValue->value(iter->m_identifier);
            }
            else
            {
                // Subscript is a value?  Resolve it to find out what kind of
                // subscript it is.
                Resolved subscriptResolved = resolve(*iter->m_pSubscriptValue);
                if (!subscriptResolved.second)
                {
                    // Didn't resolve?  Let partValue stay NULL, so we exit the loop.
                }
                else if (subscriptResolved.first->isInteger())
                {
                    // Integer subscript
                    partValue = pCurrentValue->value(static_cast<size_t>(subscriptResolved.first->asInteger()));
                }
                else if (subscriptResolved.first->type() == kTypeString)
                {
                    // String subscript (look up values by name)
                    partValue = pCurrentValue->value(subscriptResolved.first->asString());
                }
            }
            // Move on to the next part (if partValue is NULL, we'll give up)
            pCurrentValue = partValue;
        }
        return pCurrentValue;
    }
    catch (ValueException&)
    {
        // Just eat the exception
    }
    return NULL;
}


ConstValueArray Value::findByTypeName(const TypeName& typeName,
                                      bool searchIncludes,
                                      bool searchInherited) const
{
    ConstValueArray results;
    std::string typeNameStr = typeNameToString(typeName);
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        if (typeNameToString(m_values[i]->typeName()) == typeNameStr)
        {
            results.push_back(m_values[i]);
        }
        else if (searchIncludes && m_values[i]->isInclude())
        {
            ConstValueArray includedResults =
                ConstPtr(m_values[i])->findByTypeName(typeName, searchIncludes);
            for (size_t j = 0; j < includedResults.size(); ++j)
            {
                results.push_back(includedResults[j]);
            }
        }
    }
    // Haven't found it yet?  Give inherited block a shot at resolving it.
    if (searchInherited && m_pInheritedBlock != NULL)
    {
        ValueArray inheritedResults =
            m_pInheritedBlock->asBlock()->findByTypeName(typeName, searchIncludes);
        for (size_t i = 0; i < inheritedResults.size(); ++i)
        {
            results.push_back(inheritedResults[i]);
        }
    }
    return results;
}


ValueArray Value::findByTypeName(const TypeName& typeName,
                                 bool searchIncludes,
                                 bool searchInherited)
{
    ValueArray results;
    std::string typeNameStr = typeNameToString(typeName);
    for (size_t i = 0; i < m_values.size(); ++i)
    {
        if (typeNameToString(m_values[i]->typeName()) == typeNameStr)
        {
            results.push_back(m_values[i]);
        }
        else if (searchIncludes && m_values[i]->isInclude())
        {
            ValueArray includedResults =
                m_values[i]->findByTypeName(typeName, searchIncludes);
            for (size_t j = 0; j < includedResults.size(); ++j)
            {
                results.push_back(includedResults[j]);
            }
        }
    }
    // Haven't found it yet?  Give inherited block a shot at resolving it.
    if (searchInherited && m_pInheritedBlock != NULL)
    {
        ValueArray inheritedResults =
            m_pInheritedBlock->asBlock()->findByTypeName(typeName, searchIncludes);
        for (size_t i = 0; i < inheritedResults.size(); ++i)
        {
            results.push_back(inheritedResults[i]);
        }
    }
    return results;
}


namespace
{
    inline bool isIdentifierCharacter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }


    inline bool isDecimalDigit(char c)
    {
        return c >= '0' && c <= '9';
    }
}


bool Value::isNameStandardFormat(const std::string& name)
{
    for (size_t i = 0; i < name.length(); ++i)
    {
        if (i == 0)
        {
            if (!isIdentifierCharacter(name[i]))
            {
                return false;
            }
        }
        else if (!(isIdentifierCharacter(name[i]) || isDecimalDigit(name[i])))
        {
            return false;
        }
    }
    return true;
}


std::string Value::str(bool followIncludes,
                       bool keepInline,
                       size_t indentation) const
{
    std::ostringstream stream;

    if (!m_typeName.empty())
    {
        stream << '@' << typeNameToString(m_typeName) << ' ';
    }
    if (m_type == kTypeInvalid)
    {
        stream << "null";
    }
    else if (m_type == kTypeBoolean)
    {
        stream << (m_boolean ? "true" : "false");
    }
    else if (m_type == kTypeInteger)
    {
        stream << m_integer;
    }
    else if (m_type == kTypeFloat)
    {
        stream << m_float;

        double intPart;
        double fracPart = std::modf(m_float, &intPart);
        if (fracPart == 0.0)
        {
            // Ensure float numbers are written out as such, even when they can
            // round exactly to integers otherwise.
            stream << ".0";
        }
    }
    else if (m_type == kTypeString)
    {
        stream << '"' << m_string << '"';
    }
    else if (m_type == kTypeArray)
    {
        stream << "[ ";
        for (size_t i = 0; i < m_values.size(); ++i)
        {
            stream << m_values[i]->str(followIncludes, true, 0);
            if (i < m_values.size() - 1)
            {
                stream << ", ";
            }
            else
            {
                stream << " ";
            }
        }
        stream << ']';
    }
    else if (m_type == kTypeBlock)
    {
        if (m_pInheritedBlock != NULL)
        {
            stream << ": " << m_pInheritedBlock->asRawReference()->str() << ' ';
        }
        stream << '{';
        if (!keepInline)
        {
            stream << '\n';
        }
        for (size_t i = 0; i < m_values.size(); ++i)
        {
            if (!keepInline)
            {
                for (size_t j = 0; j < indentation + 4; ++j)
                {
                    stream << ' ';
                }
            }
            if (!m_values[i]->isInclude())
            {
                if (Value::isNameStandardFormat(m_blockValueNames[i]))
                {
                    stream << m_blockValueNames[i] << " = ";
                }
                else
                {
                    stream << '"' << m_blockValueNames[i] << "\" = ";
                }
            }
            stream << m_values[i]->str(followIncludes,
                                       keepInline,
                                       indentation + 4);
            if (!m_values[i]->isInclude() || !followIncludes)
            {
                stream << ';';
            }

            if (!keepInline)
            {
                stream << '\n';
            }
            else
            {
                stream << ' ';
            }
        }
        if (!keepInline)
        {
            for (size_t i = 0; i < indentation; ++i)
            {
                stream << ' ';
            }
        }
        stream << '}';
    }
    else if (m_type == kTypeMacro)
    {
        stream << m_pMacroInvocation->str();
    }
    else if (m_type == kTypeReference)
    {
        stream << m_pReference->str();
    }

    return stream.str();
}


Value::ConstResolved Value::resolve(const Value& v) const
{
    // Make sure we know the right context to evaluate strings and macros in
    Value::ConstPtr pEvaluationContext;
    Value::Ptr pNonConstEvaluationContext;
        if (m_type == kTypeBlock || m_type == kTypeArray)
    {
        pEvaluationContext = this;
        // We're allow the cheat the const system here; once the context
        // is set on a dependent variable, the value is const on the way out and
        // shouldn't be changed.
        pNonConstEvaluationContext = const_cast<Value*>(this);
    }
    else
    {
        pEvaluationContext = m_pContext;
        // We're allow the cheat the const system here; once the context
        // is set on a dependent variable, the value is const on the way out and
        // shouldn't be changed.
        pNonConstEvaluationContext = const_cast<Value*>(m_pContext.get());
    }


    if (v.type() == kTypeInvalid || v.type() == kTypeBoolean ||
        v.type() == kTypeInteger || v.type() == kTypeFloat ||
        v.type() == kTypeArray || v.type() == kTypeBlock)
    {
        // The value is a type that cannot reference any other values?
        // It's (naturally) resolved already. =)
        return ConstResolved(ConstPtr(&v), true);
    }
    else if (v.type() == kTypeString)
    {
        // Look for ${...} variables in the string to resolve.  Each of those
        // is a reference to another value.  We'll resolve as many as we can.
        bool resolvedAll = true;
        bool resolvedAny = false;
        std::string result;
        size_t index = 0;
        while (index != std::string::npos && index < v.m_string.length())
        {
            // Find the next ${...}

            size_t varStart = v.m_string.find_first_of("${", index);
            size_t varEnd = v.m_string.length();
            if (varStart == std::string::npos)
            {
                // No var; just add the rest of the string...
                result += v.m_string.substr(index, v.m_string.length() - index);
                // ...and bail
                break;
            }

            varEnd = v.m_string.find_first_of('}', varStart);
            if (varEnd == std::string::npos)
            {
                // Unterminated var; just add the rest of the string...
                result += v.m_string.substr(index, v.m_string.length() - index);
                // ...and bail
                break;
            }

            // Add in the string up to the start of the variable
            result += v.m_string.substr(index, varStart - index);

            try
            {
                // Try to parse the reference and resolve it
                std::string refString = v.m_string.substr(varStart + 2,
                                                          varEnd - varStart - 2);
                Reference::Ptr pRef = refString.length() > 0 ?
                    Reference::fromString(refString) :
                    NULL;
                if (pRef != NULL)
                {
                    Ptr pTempRefValue = new Value(pRef);
                    // We're allow the cheat the const system here; once the
                    // context is set on a dependent variable, the value is
                    // const on the way out and shouldn't be changed.
                    pTempRefValue->setContext(pNonConstEvaluationContext);
                    pTempRefValue->fixupContexts();
                    ConstResolved refResolved = pEvaluationContext->resolve(*pTempRefValue);
                    if (refResolved.second)
                    {
                        // Reference resolved, turn it into a string.
                        // Note: this may throw, but we should let it as
                        // that provides valuable information about why the
                        // resolving failed.
                        result += refResolved.first->asString();
                        resolvedAny = true;
                    }
                    else
                    {
                        // Couldn't resolve the reference; put the ${...}
                        // back in so they can see what didn't resolve.
                        resolvedAll = false;
                        result += v.m_string.substr(varStart, varEnd + 1 - varStart);
                    }
                }
                else
                {
                    // Couldn't resolve the reference; put the ${...} back
                    // in so they can see what didn't resolve.
                    resolvedAll = false;
                    result += v.m_string.substr(varStart, varEnd + 1 - varStart);
                }
            }
            catch (Parser::ParseException&)
            {
                // Parsing the reference failed?  Just put it back in as
                // unresolved so they can see what didn't make it in.
                // Couldn't resolve the reference; put the ${...} back
                // in so they can see what didn't resolve.
                resolvedAll = false;
                result += v.m_string.substr(varStart, varEnd + 1 - varStart);
            }
            // Skip past the end of the variable
            index = varEnd + 1;
        }
        if (resolvedAll && !resolvedAny)
        {
            // Didn't touch the string; just return the value directly.
            return ConstResolved(ConstPtr(&v), true);
        }
        else
        {
            Ptr resolvedStringValue = new Value(result);
            // We're allow the cheat the const system here; once the context is
            // set, the value is const on the way out and shouldn't be changed.
            resolvedStringValue->setContext(pNonConstEvaluationContext);
            return ConstResolved(resolvedStringValue, resolvedAll);
        }
    }
    else if (v.type() == kTypeMacro)
    {
        try
        {
            ConstPtr ptr;
            if (pEvaluationContext != NULL)
            {
                ptr = v.m_pMacroInvocation->execute(*pEvaluationContext);
            }

            if (ptr != NULL)
            {
                return resolve(*ptr);
            }
        }
        catch (ValueException&)
        {
            // Fall through to the default (unresolved) case
        }
    }
    else //if (m_type == kTypeReference)
    {
        // Force 'this' to be resolved in the context of its parent
        ConstPtr ptr = (&v == this) ? NULL : find(*v.m_pReference);

        // Resolved the found reference if possible.  Note we do NOT use the
        // context found above; we either find the reference and resolve it in
        // this value directly (which will apply if it is a block or an array),
        // or else we use this value's context to resolve the reference.  If we
        // were to use pEvaluationContext instead, the reference would never get
        // a shot at evaluation within the actual block/array it may exist in.
        if (ptr != NULL)
        {
            return resolve(*ptr);
        }
        else if (m_pContext != NULL)
        {
            return m_pContext->resolve(v);
        }
    }
    return ConstResolved(ConstPtr(&v), false);
}


Value::Resolved Value::resolve(Value& v)
{
    // Make sure we know the right context to evaluate strings and macros in
    Value::Ptr pEvaluationContext;
    if (m_type == kTypeBlock || m_type == kTypeArray)
    {
        pEvaluationContext = this;
    }
    else
    {
        pEvaluationContext = m_pContext;
    }

    if (v.type() == kTypeInvalid || v.type() == kTypeBoolean ||
        v.type() == kTypeInteger || v.type() == kTypeFloat ||
        v.type() == kTypeArray || v.type() == kTypeBlock)
    {
        // The value is a type that cannot reference any other values?
        // It's (naturally) resolved already. =)
        return Resolved(Ptr(&v), true);
    }
    else if (v.type() == kTypeString)
    {
        // Look for ${...} variables in the string to resolve.  Each of those
        // is a reference to another value.  We'll resolve as many as we can.
        bool resolvedAll = true;
        bool resolvedAny = false;
        std::string result;
        size_t index = 0;
        while (index != std::string::npos && index < v.m_string.length())
        {
            // Find the next ${...}

            size_t varStart = v.m_string.find_first_of("${", index);
            size_t varEnd = v.m_string.length();
            if (varStart == std::string::npos)
            {
                // No var; just add the rest of the string...
                result += v.m_string.substr(index, v.m_string.length() - index);
                // ...and bail
                break;
            }

            varEnd = v.m_string.find_first_of('}', varStart);
            if (varEnd == std::string::npos)
            {
                // Unterminated var; just add the rest of the string...
                result += v.m_string.substr(index, v.m_string.length() - index);
                // ...and bail
                break;
            }

            // Add in the string up to the start of the variable
            result += v.m_string.substr(index, varStart - index);

            try
            {
                // Try to parse the reference and resolve it
                std::string refString = v.m_string.substr(varStart + 2,
                                                          varEnd - varStart - 2);
                Reference::Ptr pRef = refString.length() > 0 ?
                    Reference::fromString(refString) :
                    NULL;
                if (pRef != NULL)
                {
                    Ptr pTempRefValue = new Value(pRef);
                    pTempRefValue->setContext(pEvaluationContext);
                    pTempRefValue->fixupContexts();
                    Resolved refResolved = pEvaluationContext->resolve(*pTempRefValue);
                    if (refResolved.second)
                    {
                        // Reference resolved, turn it into a string.
                        // Note: this may throw, but we should let it as
                        // that provides valuable information about why the
                        // resolving failed.
                        result += refResolved.first->asString();
                        resolvedAny = true;
                    }
                    else
                    {
                        // Couldn't resolve the reference; put the ${...}
                        // back in so they can see what didn't resolve.
                        resolvedAll = false;
                        result += v.m_string.substr(varStart, varEnd + 1 - varStart);
                    }
                }
                else
                {
                    // Couldn't resolve the reference; put the ${...} back
                    // in so they can see what didn't resolve.
                    resolvedAll = false;
                    result += v.m_string.substr(varStart, varEnd + 1 - varStart);
                }
            }
            catch (Parser::ParseException&)
            {
                // Parsing the reference failed?  Just put it back in as
                // unresolved so they can see what didn't make it in.
                // Couldn't resolve the reference; put the ${...} back
                // in so they can see what didn't resolve.
                resolvedAll = false;
                result += v.m_string.substr(varStart, varEnd + 1 - varStart);
            }
            // Skip past the end of the variable
            index = varEnd + 1;
        }
        if (resolvedAll && !resolvedAny)
        {
            // Didn't touch the string; just return the value directly.
            return Resolved(Ptr(&v), true);
        }
        else
        {
            Ptr resolvedStringValue = new Value(result);
            resolvedStringValue->setContext(pEvaluationContext);
            return Resolved(resolvedStringValue, resolvedAll);
        }
    }
    else if (v.type() == kTypeMacro)
    {
        try
        {
            Ptr ptr;
            if (pEvaluationContext != NULL)
            {
                ptr = v.m_pMacroInvocation->execute(*pEvaluationContext);
            }

            if (ptr != NULL)
            {
                return resolve(*ptr);
            }
        }
        catch (ValueException&)
        {
            // Fall through to the default (unresolved) case
        }
    }
    else //if (m_type == kTypeReference)
    {
        // Force 'this' to be resolved in the context of its parent
        Ptr ptr = (&v == this) ? NULL : find(*v.m_pReference);

        // Resolved the found reference if possible.  Note we do NOT use the
        // context found above; we either find the reference and resolve it in
        // this value directly (which will apply if it is a block or an array),
        // or else we use this value's context to resolve the reference.  If we
        // were to use pEvaluationContext instead, the reference would never get
        // a shot at evaluation within the actual block/array it may exist in.
        if (ptr != NULL)
        {
            return resolve(*ptr);
        }
        else if (m_pContext != NULL)
        {
            return m_pContext->resolve(v);
        }
    }
    return Resolved(Ptr(&v), false);
}


    } // namespace Rsd
} // namespace RenderSpud
