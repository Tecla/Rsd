////////////
//
//  File:      SchemaManager.cpp
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data schemas/validation
//
////////////

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <sys/types.h>
#include <dirent.h>
#endif
#include <errno.h>
#include <sstream>

#include <Rsd/File.h>
#include <Rsd/SchemaManager.h>


namespace RenderSpud
{
    namespace Rsd
    {


enum SchemaType
{
    kSchemaTypePrimitive,
    kSchemaTypeBlock,
    kSchemaTypeArray,
    kSchemaTypeFunction,
    kSchemaTypeUnknown
};

SchemaType findSchemaType(Value::Ptr pValue)
{
    const TypeName& t = pValue->typeName();
    if (t.size() == 1 && pValue->type() == Value::kTypeBlock)
    {
        if (t[0] == "Primitive")
            return kSchemaTypePrimitive;
        else if (t[0] == "Block")
            return kSchemaTypeBlock;
        else if (t[0] == "Array")
            return kSchemaTypeArray;
        else if (t[0] == "Function")
            return kSchemaTypeFunction;
    }
    return kSchemaTypeUnknown;
}


std::string schemaTypeString(SchemaType t)
{
    if (t == kSchemaTypePrimitive)
        return "Primitive";
    else if (t == kSchemaTypeBlock)
        return "Block";
    else if (t == kSchemaTypeArray)
        return "Array";
    else if (t == kSchemaTypeFunction)
        return "Function";
    return "\"<unknown\"";
}


//------------------------------------------------------------------------------


// Individual attribute in a schema
class Attribute
{
public:
    Attribute()
        : m_isArrayAttribute(false),
          m_isRequired(true),
          m_sizes(),
          m_types()
    {

    }

    Attribute(const Attribute& attr)
        : m_isArrayAttribute(attr.m_isArrayAttribute),
          m_isRequired(attr.m_isRequired),
          m_sizes(attr.m_sizes),
          m_types(attr.m_types)
    {

    }

    Attribute& operator =(const Attribute& attr)
    {
        m_isArrayAttribute = attr.m_isArrayAttribute;
        m_isRequired = attr.m_isRequired;
        m_sizes = attr.m_sizes;
        m_types = attr.m_types;
        return *this;
    }

    bool  isArrayAttribute() const    { return m_isArrayAttribute; }
    bool& isArrayAttribute()          { return m_isArrayAttribute; }
    void  setIsArrayAttribute(bool a) { m_isArrayAttribute = a; }

    bool  isRequired() const    { return m_isRequired; }
    bool& isRequired()          { return m_isRequired; }
    void  setIsRequired(bool i) { m_isRequired = i; }

    const std::vector<TypeName>& types() const { return m_types; }
    std::vector<TypeName>&       types()       { return m_types; }

    const std::vector<size_t>& sizes() const { return m_sizes; }
    std::vector<size_t>&       sizes()       { return m_sizes; }

    std::string typesListAsString()
    {
        std::string result;
        for (size_t i = 0; i < m_types.size(); ++i)
        {
            result += typeNameToString(m_types[i]);
            if (i < m_types.size() - 1)
            {
                result += ", ";
            }
        }
        return result;
    }

private:
    bool m_isArrayAttribute;       // m_sizes can be any size if this is true (instead of exactly 1)
    bool m_isRequired;
    std::vector<size_t> m_sizes;   // If not an array, then m_sizes.size() == 1
    std::vector<TypeName> m_types; // Empty => don't care which type
};

typedef std::map<std::string, Attribute> AttributeMap;


//------------------------------------------------------------------------------


// Individual schema, holds some of the validation implementation
class Schema
{
public:
    Schema(Value::Ptr pSchemaValue = NULL,
           bool isBuiltin = false,
           const ValidationPolicy& defaultPolicy = ValidationPolicy())
        : m_pSchemaValue(pSchemaValue),
          m_isBuiltin(isBuiltin),
          m_type(),
          m_policy(defaultPolicy)
    {

    }

    virtual ~Schema() { }


    bool isBuiltin() const { return m_isBuiltin; }

    Value::Ptr schemaValue() { return m_pSchemaValue; }

    const TypeName& fullyQualifiedType() const { return m_type; }

    virtual SchemaType schemaType() const = 0;

    const ValidationPolicy& policy() { return m_policy; }


    virtual bool validate(Value::Ptr pValue,
                          SchemaManager& manager,
                          std::vector<std::string>* pValidationResults = NULL,
                          bool recursiveValidation = true) = 0;

    bool typeMatches(Value::Ptr pValue,
                     SchemaManager& manager,
                     const std::vector<TypeName>& typeNames);

protected:
    Value::Ptr m_pSchemaValue;
    bool m_isBuiltin;
    TypeName m_type; // Fully-qualified type; size == 1 => global namespace
    ValidationPolicy m_policy;
};


class PrimitiveSchema : public Schema
{
public:
    PrimitiveSchema(Value::Ptr pSchemaValue = NULL);

    virtual SchemaType schemaType() const { return kSchemaTypePrimitive; }

    virtual bool validate(Value::Ptr pValue,
                          SchemaManager& manager,
                          std::vector<std::string>* pValidationResults = NULL,
                          bool recursiveValidation = true);

private:
    Value::Type m_primType;
};


class BlockSchema : public Schema
{
public:
    BlockSchema(Value::Ptr pSchemaValue = NULL,
                bool isBuiltin = false,
                const ValidationPolicy& defaultPolicy = ValidationPolicy());

    const TypeName& superType() const { return m_superType; }

    virtual SchemaType schemaType() const { return kSchemaTypeBlock; }

    virtual bool validate(Value::Ptr pValue,
                          SchemaManager& manager,
                          std::vector<std::string>* pValidationResults = NULL,
                          bool recursiveValidation = true);

    void addRequiredAttributes(std::map< Attribute*, std::pair<std::string, bool> >& requiresMap,
                               SchemaManager& manager);

private:
    TypeName m_superType; // Empty => no super type
    AttributeMap m_attributes;
};


class ArraySchema : public Schema
{
public:
    ArraySchema(Value::Ptr pSchemaValue = NULL,
                bool isBuiltin = false,
                const ValidationPolicy& defaultPolicy = ValidationPolicy());

    virtual SchemaType schemaType() const { return kSchemaTypeArray; }

    virtual bool validate(Value::Ptr pValue,
                          SchemaManager& manager,
                          std::vector<std::string>* pValidationResults = NULL,
                          bool recursiveValidation = true);

private:
    Attribute m_arrayAttribute;
};


class FunctionSchema : public Schema
{
public:
    FunctionSchema(Value::Ptr pSchemaValue = NULL,
                   bool isBuiltin = false,
                   const ValidationPolicy& defaultPolicy = ValidationPolicy());

    virtual SchemaType schemaType() const { return kSchemaTypeFunction; }

    virtual bool validate(Value::Ptr pValue,
                          SchemaManager& manager,
                          std::vector<std::string>* pValidationResults = NULL,
                          bool recursiveValidation = true);

private:
    AttributeMap m_keywordAttributes;
};


//------------------------------------------------------------------------------


SchemaManager::SchemaManager(bool loadBuiltinSchemas)
    : m_schemas(),
      m_schemaValues(),
      m_validateAllTypedValues(true),
      m_defaultPolicy()
{
    if (loadBuiltinSchemas)
    {
        addBuiltinSchemas();
    }
}


SchemaManager::~SchemaManager()
{
    clearSchemas(false);
}


void SchemaManager::addSchema(Value::Ptr pSchemaBlock, bool isBuiltin)
{
    // Sanity check that this is actually a schema node
    SchemaType type = findSchemaType(pSchemaBlock);
    if (type == kSchemaTypeUnknown || pSchemaBlock->name().empty())
    {
        return;
    }

//    m_schemaValues.push_back(pSchemaBlock->clone());
    m_schemaValues.push_back(pSchemaBlock);
    if (type == kSchemaTypePrimitive)
    {
        m_schemas.insert(std::pair<std::string, Schema*>(pSchemaBlock->name(),
                                                         new PrimitiveSchema(m_schemaValues.back())));
    }
    else if (type == kSchemaTypeBlock)
    {
        m_schemas.insert(std::pair<std::string, Schema*>(pSchemaBlock->name(),
                                                         new BlockSchema(m_schemaValues.back(),
                                                                         isBuiltin,
                                                                         m_defaultPolicy)));
    }
    else if (type == kSchemaTypeArray)
    {
        m_schemas.insert(std::pair<std::string, Schema*>(pSchemaBlock->name(),
                                                         new ArraySchema(m_schemaValues.back(),
                                                                         isBuiltin,
                                                                         m_defaultPolicy)));
    }
    else if (type == kSchemaTypeFunction)
    {
        m_schemas.insert(std::pair<std::string, Schema*>(pSchemaBlock->name(),
                                                         new FunctionSchema(m_schemaValues.back(),
                                                                            isBuiltin,
                                                                            m_defaultPolicy)));
    }
}


void SchemaManager::addSchemas(Value::Ptr pRoot)
{
    for (size_t i = 0; i < pRoot->size(); ++i)
    {
        // This checks if the node really is a schema node, and adds it if so
        addSchema(pRoot->value(i));
        // Recurse in looking for schemas deeper in the namespace hierarchy
        addSchemas(pRoot->value(i));
    }
}


Value::Ptr SchemaManager::findSchema(const TypeName& type)
{
    Schema *pSchema = findSchemaForType(type);
    return pSchema != NULL ? pSchema->schemaValue() : NULL;
}


Schema* SchemaManager::findSchemaForType(const TypeName& type)
{
    if (type.empty())
    {
        return NULL;
    }

    std::string fullyQualifiedName = typeNameToString(type);
    SchemaMap::iterator iter = m_schemas.find(fullyQualifiedName);
    if (iter == m_schemas.end())
    {
        return iter->second;
    }
    return NULL;
}


bool SchemaManager::loadAllSchemas(const std::string& fileOrDirectoryPath)
{
#ifdef _WIN32
    struct _finddata_t info;
    intptr_t pHandle = _findfirst(fileOrDirectoryPath.c_str(), &info);
    if (pHandle == intptr_t(-1))
    {
        return false;
    }
    int result = 0;
    while (result != -1)
    {
        // Skip subdirectories
        if ((info.attrib & _A_SUBDIR) == 0)
        {
            std::string path = (fileOrDirectoryPath + '\\') + info.name;
            if (path.length() > 4 &&
                (path.substr(path.length() - 4, 4) == ".rsd" ||
                 path.substr(path.length() - 4, 4) == ".RSD"))
            {
                // Only load files that seem to have '.rsd' extensions
                File::Ptr pRsdFile = new File(path);
                addSchemas(pRsdFile);
            }
        }
        result = _findnext(pHandle, &info);
    }
    _findclose(pHandle);
    return true;
#else // _WIN32
    DIR* pDir = opendir(fileOrDirectoryPath.c_str());
    if (pDir == NULL)
    {
        if (errno == ENOTDIR)
        {
            // Not a directory?  Must be a file.
            File::Ptr pRsdFile = new File(fileOrDirectoryPath);
            addSchemas(pRsdFile);
            return true;
        }
        else
        {
            // Some other error occured (permissions, etc.)
            return false;
        }
    }

    // It's a directory
    for (dirent *pDirEntry = readdir(pDir);
         pDirEntry != NULL;
         pDirEntry = readdir(pDir))
    {
        // Skip hidden entries, . and ..
        if (std::string(pDirEntry->d_name)[0] == '.')
        {
            continue;
        }

        std::string path = fileOrDirectoryPath + '/' + pDirEntry->d_name;
        DIR* pSubDir = opendir(path.c_str());
        if (pSubDir != NULL)
        {
            // Skip subdirectories
            closedir(pSubDir);
            continue;
        }
        else if (path.length() > 4 && path.substr(path.length() - 4, 4) == ".rsd")
        {
            // Only load files that seem to have '.rsd' extensions
            File::Ptr pRsdFile = new File(path);
            addSchemas(pRsdFile);
        }
    }

    closedir(pDir);

    return true;
#endif // _WIN32
}


void SchemaManager::clearSchemas(bool reloadBuiltins)
{
    for (std::map<std::string, Schema*>::iterator iter = m_schemas.begin();
         iter != m_schemas.end();
         ++iter)
    {
        delete iter->second;
    }
    m_schemas.clear();
    m_schemaValues.clear();
    if (reloadBuiltins)
    {
        addBuiltinSchemas();
    }
}


bool SchemaManager::validateAllLoadedSchemas(std::vector<std::string>* pValidationResults)
{
    bool valid = true;
    for (size_t i = 0; i < m_schemaValues.size(); ++i)
    {
        bool thisSchemaValid =
            validateSchema(m_schemaValues[i], pValidationResults);
        valid = valid && thisSchemaValid;
    }
    return valid;
}


bool SchemaManager::validateSchema(Value::Ptr pSchemaBlock,
                                   std::vector<std::string>* pValidationResults)
{
    // Sanity check that this is actually a schema node
    SchemaType type = findSchemaType(pSchemaBlock);
    if (type == kSchemaTypeUnknown)
    {
        if (pValidationResults)
        {
            std::ostringstream stream;
            stream << pSchemaBlock->file() << ':' << pSchemaBlock->line() << ':'
                   << pSchemaBlock->pos()
                   << " (" << pSchemaBlock->path()
                   << ") is not a schema, and so can't be validated as a schema. "
                   << "It is of type: \"" << typeNameToString(pSchemaBlock->typeName()) << '"'
                   << std::flush;
            pValidationResults->push_back(stream.str());
        }
        return false;
    }

    return m_schemas.find(schemaTypeString(type))->second->validate(pSchemaBlock,
                                                                    *this,
                                                                    pValidationResults,
                                                                    true);
}


bool SchemaManager::validate(Value::Ptr pValue,
                             std::vector<std::string>* pValidationResults,
                             bool recursiveValidation,
                             const TypeName& overrideType)
{
    // Forward to schema validation if possible
    SchemaType schemaType = findSchemaType(pValue);
    if (schemaType != kSchemaTypeUnknown)
    {
        return validateSchema(pValue, pValidationResults);
    }

    TypeName type = overrideType.empty() ? pValue->typeName() : overrideType;

    // Double-check if it's a macro; we get the 'type' from the name
    if (pValue->canConvertTo(Value::kTypeMacro))
    {
        Value::Ptr pFinalValue = pValue->resolved();
        type = typeNameFromString(pFinalValue->asRawMacro()->name());
    }

    if (type.empty())
    {
        if (recursiveValidation)
        {
            bool valid = true;
            for (size_t i = 0; i < pValue->size(); ++i)
            {
                Value::Ptr pFinalValue = pValue->value(i)->resolved();
                if (!validate(pFinalValue,
                              pValidationResults,
                              recursiveValidation))
                {
                    valid = false;
                }
            }
            if (!valid)
            {
                return false;
            }
        }
        else
        {
            // Generic data, no recursive validation?  Ship it!
            return true;
        }
    }

    Schema* pSchema = findSchemaForType(type);
    if (pSchema == NULL)
    {
        if (m_validateAllTypedValues)
        {
            if (pValidationResults)
            {
                std::ostringstream stream;
                stream << pValue->file() << ':' << pValue->line() << ':'
                       << pValue->pos()
                       << " (" << pValue->path()
                       << ") Value has a type with no schema to validate it: \""
                       << typeNameToString(type) << '"' << std::flush;
                pValidationResults->push_back(stream.str());
            }
            return false;
        }
        else
        {
            // No type, and we don't care?  It's valid!
            return true;
        }
    }

    return pSchema->validate(pValue,
                             *this,
                             pValidationResults,
                             recursiveValidation);
}


void SchemaManager::addBuiltinSchemas()
{
    // TODO: builtin schemas
}


//------------------------------------------------------------------------------



bool Schema::typeMatches(Value::Ptr pValue,
                         SchemaManager& manager,
                         const std::vector<TypeName>& typeNames)
{
    // TODO: given a value, see if the type (or an alias/supertype) matches a list
    return false;
}


PrimitiveSchema::PrimitiveSchema(Value::Ptr pSchemaValue)
    : Schema(pSchemaValue, true)
{
    if (pSchemaValue->name() == "string")
        m_primType = Value::kTypeString;
    else if (pSchemaValue->name() == "bool")
        m_primType = Value::kTypeBoolean;
    else if (pSchemaValue->name() == "int")
        m_primType = Value::kTypeInteger;
    else if (pSchemaValue->name() == "float")
        m_primType = Value::kTypeFloat;
    else
        m_primType = Value::kTypeInvalid;
}


bool PrimitiveSchema::validate(Value::Ptr pValue,
                           SchemaManager& manager,
                           std::vector<std::string>* pValidationResults,
                           bool recursiveValidation)
{
    if (pValue->type() != m_primType)
    {
        if (pValidationResults)
        {
            std::ostringstream stream;
            stream << pValue->file() << ':' << pValue->line() << ':'
                   << pValue->pos()
                   << " (" << pValue->path()
                   << ") Value with wrong primitive type."
                   << std::flush;
            pValidationResults->push_back(stream.str());
        }
        return false;
    }
    return true;
}



BlockSchema::BlockSchema(Value::Ptr pSchemaValue,
                         bool isBuiltin,
                         const ValidationPolicy& defaultPolicy)
    : Schema(pSchemaValue, isBuiltin, defaultPolicy),
      m_superType(),
      m_attributes()
{
    Value::Ptr pSuperTypeSchema = pSchemaValue->inheritedBlock();
    if (pSuperTypeSchema)
    {
        m_superType = pSuperTypeSchema->typeName();
    }

    Value::Ptr pPolicyValue = pSchemaValue->value("policy");
    if (pPolicyValue != NULL && pPolicyValue->canConvertTo(Value::kTypeString))
    {
        std::string policyString = pPolicyValue->asString();
        if (policyString == "strict")
        {
            m_policy = ValidationPolicy(kPolicyStrict);
        }
        else if (policyString == "permissive")
        {
            m_policy = ValidationPolicy(kPolicyPermissive);
        }
    }

    ValueArray attrs = pSchemaValue->findByTypeName(typeNameFromString("Attribute"));
    for (size_t ai = 0; ai < attrs.size(); ++ai)
    {
        Value::Ptr pRequiredValue = attrs[ai]->value("required");
        bool required = m_policy.m_requireMemberByDefault;
        if (pRequiredValue != NULL && pRequiredValue->canConvertTo(Value::kTypeBoolean))
        {
            required = pRequiredValue->asBoolean();
        }

        Attribute attr;
        attr.setIsRequired(required);
        attr.setIsArrayAttribute(false);

        Value::Ptr pTypeValue = attrs[ai]->value("type");
        if (pTypeValue != NULL)
        {
            if (pTypeValue->canConvertTo(Value::kTypeArray))
            {
                Value::Ptr pArrayValue = pTypeValue->asArray();
                for (size_t ti = 0; ti < pArrayValue->size(); ++ti)
                {
                    Value::Ptr pArrayItem = pArrayValue->value(ti);
                    if (pArrayItem->canConvertTo(Value::kTypeBlock))
                    {
                        attr.types().push_back(typeNameFromString(pArrayItem->asBlock()->path()));
                    }
                }
            }
            else if (pTypeValue->canConvertTo(Value::kTypeBlock))
            {
                attr.types().push_back(typeNameFromString(pTypeValue->asBlock()->path()));
            }
        }
        m_attributes[attrs[ai]->name()] = attr;
    }

    attrs = pSchemaValue->findByTypeName(typeNameFromString("ArrayAttribute"));
    for (size_t ai = 0; ai < attrs.size(); ++ai)
    {
        Value::Ptr pRequiredValue = attrs[ai]->value("required");
        bool required = m_policy.m_requireMemberByDefault;
        if (pRequiredValue != NULL && pRequiredValue->canConvertTo(Value::kTypeBoolean))
        {
            required = pRequiredValue->asBoolean();
        }

        Attribute attr;
        attr.setIsRequired(required);
        attr.setIsArrayAttribute(true);

        Value::Ptr pTypeValue = attrs[ai]->value("type");
        if (pTypeValue != NULL)
        {
            if (pTypeValue->canConvertTo(Value::kTypeArray))
            {
                Value::Ptr pArrayValue = pTypeValue->asArray();
                for (size_t ti = 0; ti < pArrayValue->size(); ++ti)
                {
                    Value::Ptr pArrayItem = pArrayValue->value(ti);
                    if (pArrayItem->canConvertTo(Value::kTypeBlock))
                    {
                        attr.types().push_back(typeNameFromString(pArrayItem->asBlock()->path()));
                    }
                }
            }
            else if (pTypeValue->canConvertTo(Value::kTypeBlock))
            {
                attr.types().push_back(typeNameFromString(pTypeValue->asBlock()->path()));
            }
        }

        Value::Ptr pSizesValue = attrs[ai]->value("sizes");
        if (pSizesValue != NULL)
        {
            if (pSizesValue->canConvertTo(Value::kTypeArray))
            {
                Value::Ptr pArrayValue = pSizesValue->asArray();
                for (size_t ti = 0; ti < pArrayValue->size(); ++ti)
                {
                    Value::Ptr pArrayItem = pArrayValue->value(ti);
                    if (pArrayItem->canConvertTo(Value::kTypeInteger))
                    {
                        attr.sizes().push_back(pArrayItem->asInteger());
                    }
                }
            }
        }
        m_attributes[attrs[ai]->name()] = attr;
    }
}


bool BlockSchema::validate(Value::Ptr pValue,
                           SchemaManager& manager,
                           std::vector<std::string>* pValidationResults,
                           bool recursiveValidation)
{
    // Make sure the final value is a block

    Value::Ptr pFinalValue = NULL;
    Value::Type checkType = Value::kTypeInvalid;
    try
    {
        // Get all values; includes and inherits are all placed together and
        // includes removed so that we have all the final set of values ready.
        pFinalValue = pValue->asInlinedBlock();
        checkType = pFinalValue->type();
    }
    catch (ValueConversionException&)
    {
        // Not a block, eat the exception
    }

    if (checkType != Value::kTypeBlock)
    {
        if (pValidationResults != NULL)
        {
            std::ostringstream stream;
            stream << pValue->file() << ':' << pValue->line() << ':'
                   << pValue->pos()
                   << " (" << pValue->path()
                   << ") Value with block type is not a block."
                   << std::flush;
            pValidationResults->push_back(stream.str());
        }
        return false;
    }

    // Get all required attributes in a list, marked as not seen yet
    std::map< Attribute*, std::pair<std::string, bool> > visitedRequiredAttributes;
    addRequiredAttributes(visitedRequiredAttributes, manager);

    // Verify member types
    bool allMatch = true;
    for (size_t i = 0; i < pFinalValue->size(); ++i)
    {
        // Check for keyword existence
        Value::Ptr pMemberValue = pFinalValue->value(i);
        std::string memberName = pMemberValue->name();
        AttributeMap::iterator attrIter;

        // Walk up and find the nearest inherited attribute spec for this name
        BlockSchema *pCurSchema = this;
        while (pCurSchema != NULL)
        {
            attrIter = pCurSchema->m_attributes.find(memberName);
            if (attrIter == m_attributes.end())
            {
                pCurSchema = reinterpret_cast<BlockSchema*>(manager.findSchemaForType(pCurSchema->m_superType));
            }
        }
        if (attrIter == m_attributes.end())
        {
            if (m_policy.m_disallowExtraMembers)
            {
                // Only whine about it and fail validation if the policy is strict
                if (pValidationResults != NULL)
                {
                    std::ostringstream stream;
                    stream << pMemberValue->file() << ':' << pMemberValue->line() << ':'
                           << pMemberValue->pos()
                           << " In block (" << pValue->path()
                           << "), member " << memberName
                           << " does not match any in the schema, and extra members are disallowed."
                           << std::flush;
                    pValidationResults->push_back(stream.str());
                }
                allMatch = false;
            }
            continue;
        }

        Attribute& attr = attrIter->second;

        // Mark this attribute as having been visited
        visitedRequiredAttributes[&attr] = std::pair<std::string, bool>(memberName, true);

        // Check if the type matches what's available
        if (attr.isArrayAttribute())
        {
            // TODO: verify array elements; check depths, sizes, and leaf types
        }
        else
        {
            if (!Schema::typeMatches(pMemberValue, manager, attr.types()))
            {
                if (pValidationResults != NULL)
                {
                    std::ostringstream stream;
                    stream << pMemberValue->file() << ':' << pMemberValue->line() << ':'
                           << pMemberValue->pos()
                           << " In block (" << pValue->path()
                           << "), member " << memberName
                           << " does not match any type "
                           << "in the schema for that attribute; type is "
                           << typeNameToString(pMemberValue->typeName())
                           << " but available types were: "
                           << attr.typesListAsString() << std::flush;
                    pValidationResults->push_back(stream.str());
                }
                allMatch = false;
                continue;
            }

            // Check the arg recursively
            if (recursiveValidation)
            {
                bool matches = manager.validate(pMemberValue,
                                                pValidationResults,
                                                recursiveValidation);
                allMatch = allMatch && matches;
            }
        }
    }

    // List out missing required attributes, if any
    for (std::map< Attribute*, std::pair<std::string, bool> >::iterator visitedIter = visitedRequiredAttributes.begin();
         visitedIter != visitedRequiredAttributes.end();
         ++visitedIter)
    {
        if (visitedIter->second.second == false)
        {
            if (pValidationResults != NULL)
            {
                std::ostringstream stream;
                stream << pValue->file() << ':' << pValue->line() << ':'
                       << pValue->pos()
                       << " In block (" << pValue->path()
                       << "), member " << visitedIter->second.first
                       << " was not found, but is required; type is "
                       << typeNameToString(pValue->typeName()) << std::flush;
                pValidationResults->push_back(stream.str());
            }
            allMatch = false;
        }
    }

    return allMatch;
}


void BlockSchema::addRequiredAttributes(std::map< Attribute*, std::pair<std::string, bool> >& requiresMap,
                                        SchemaManager& manager)
{
    BlockSchema* pSuperSchema = reinterpret_cast<BlockSchema*>(manager.findSchemaForType(m_superType));
    pSuperSchema->addRequiredAttributes(requiresMap, manager);

    for (AttributeMap::iterator iter = m_attributes.begin();
         iter != m_attributes.end();
         ++iter)
    {
        requiresMap[&iter->second] = std::pair<std::string, bool>(iter->first, false);
    }
}


ArraySchema::ArraySchema(Value::Ptr pSchemaValue,
                         bool isBuiltin,
                         const ValidationPolicy& defaultPolicy)
    : Schema(pSchemaValue, isBuiltin, defaultPolicy),
      m_arrayAttribute()
{
    m_arrayAttribute.setIsArrayAttribute(true);
    m_arrayAttribute.setIsRequired(true);

    Value::Ptr pPolicyValue = pSchemaValue->value("policy");
    if (pPolicyValue != NULL && pPolicyValue->canConvertTo(Value::kTypeString))
    {
        std::string policyString = pPolicyValue->asString();
        if (policyString == "strict")
        {
            m_policy = ValidationPolicy(kPolicyStrict);
        }
        else if (policyString == "permissive")
        {
            m_policy = ValidationPolicy(kPolicyPermissive);
        }
    }

    Value::Ptr pTypeValue = pSchemaValue->value("type");
    if (pTypeValue != NULL)
    {
        if (pTypeValue->canConvertTo(Value::kTypeArray))
        {
            Value::Ptr pArrayValue = pTypeValue->asArray();
            for (size_t ti = 0; ti < pArrayValue->size(); ++ti)
            {
                Value::Ptr pArrayItem = pArrayValue->value(ti);
                if (pArrayItem->canConvertTo(Value::kTypeBlock))
                {
                    m_arrayAttribute.types().push_back(typeNameFromString(pArrayItem->asBlock()->path()));
                }
            }
        }
        else if (pTypeValue->canConvertTo(Value::kTypeBlock))
        {
            m_arrayAttribute.types().push_back(typeNameFromString(pTypeValue->asBlock()->path()));
        }
    }

    Value::Ptr pSizesValue = pSchemaValue->value("sizes");
    if (pSizesValue != NULL)
    {
        if (pSizesValue->canConvertTo(Value::kTypeArray))
        {
            Value::Ptr pArrayValue = pSizesValue->asArray();
            for (size_t ti = 0; ti < pArrayValue->size(); ++ti)
            {
                Value::Ptr pArrayItem = pArrayValue->value(ti);
                if (pArrayItem->canConvertTo(Value::kTypeInteger))
                {
                    m_arrayAttribute.sizes().push_back(pArrayItem->asInteger());
                }
            }
        }
    }
}


bool ArraySchema::validate(Value::Ptr pValue,
                           SchemaManager& manager,
                           std::vector<std::string>* pValidationResults,
                           bool recursiveValidation)
{
    // TODO: validate!
    return false;
}


FunctionSchema::FunctionSchema(Value::Ptr pSchemaValue,
                               bool isBuiltin,
                               const ValidationPolicy& defaultPolicy)
    : Schema(pSchemaValue, isBuiltin, defaultPolicy),
      m_keywordAttributes()
{
    Value::Ptr pPolicyValue = pSchemaValue->value("policy");
    if (pPolicyValue != NULL && pPolicyValue->canConvertTo(Value::kTypeString))
    {
        std::string policyString = pPolicyValue->asString();
        if (policyString == "strict")
        {
            m_policy = ValidationPolicy(kPolicyStrict);
        }
        else if (policyString == "permissive")
        {
            m_policy = ValidationPolicy(kPolicyPermissive);
        }
    }

    for (size_t ki = 0; ki < pSchemaValue->size(); ++ki)
    {
        Value::Ptr pKeywordValue = pSchemaValue->value(ki);
        if (pKeywordValue->name() == "policy")
            continue;

        Attribute attr;
        attr.setIsRequired(m_policy.m_requireMemberByDefault);
        attr.setIsArrayAttribute(false);

        if (pKeywordValue->canConvertTo(Value::kTypeArray))
        {
            Value::Ptr pArrayValue = pKeywordValue->asArray();
            for (size_t ti = 0; ti < pArrayValue->size(); ++ti)
            {
                Value::Ptr pArrayItem = pArrayValue->value(ti);
                if (pArrayItem->canConvertTo(Value::kTypeBlock))
                {
                    attr.types().push_back(typeNameFromString(pArrayItem->asBlock()->path()));
                }
            }
        }
        else if (pKeywordValue->canConvertTo(Value::kTypeBlock))
        {
            attr.types().push_back(typeNameFromString(pKeywordValue->asBlock()->path()));
        }
        m_keywordAttributes[pKeywordValue->name()] = attr;
    }
}


bool FunctionSchema::validate(Value::Ptr pValue,
                              SchemaManager& manager,
                              std::vector<std::string>* pValidationResults,
                              bool recursiveValidation)
{
    // Make sure the value is a function
    if (pValue->type() != Value::kTypeMacro)
    {
        if (pValidationResults != NULL)
        {
            std::ostringstream stream;
            stream << pValue->file() << ':' << pValue->line() << ':'
                   << pValue->pos()
                   << " (" << pValue->path()
                   << ") Value with function type is not a function."
                   << std::flush;
            pValidationResults->push_back(stream.str());
        }
        return false;
    }

    // This check should be extraneous, but just in case they try to use @type
    // with a different function schema we double-check.
    MacroInvocation::Ptr pFunction = pValue->asRawMacro();
    if (pFunction->name() != typeNameToString(fullyQualifiedType()))
    {
        if (pValidationResults != NULL)
        {
            std::ostringstream stream;
            stream << pValue->file() << ':' << pValue->line() << ':'
                   << pValue->pos()
                   << " (" << pValue->path()
                   << ") Function name and type do not match."
                   << std::flush;
            pValidationResults->push_back(stream.str());
        }
        return false;
    }

    // Build a list of required attributes, and make sure they all get included
    std::map<std::string, bool> visitedRequiredAttributes;
    for (AttributeMap::iterator iter = m_keywordAttributes.begin();
         iter != m_keywordAttributes.end();
         ++iter)
    {
        if (iter->second.isRequired())
        {
            visitedRequiredAttributes[iter->first] = false;
        }
    }

    // Verify argument types
    bool allMatch = true;
    Macro::ArgumentValueMap& args = pFunction->arguments();
    for (Macro::ArgumentValueMap::iterator iter = args.begin();
         iter != args.end();
         ++iter)
    {
        // Check for keyword existence
        const std::string& argName = iter->first;
        Value::Ptr pArgValue = iter->second;
        AttributeMap::iterator attrIter = m_keywordAttributes.find(argName);
        if (attrIter == m_keywordAttributes.end())
        {
            if (pValidationResults != NULL)
            {
                std::ostringstream stream;
                stream << pArgValue->file() << ':' << pArgValue->line() << ':'
                       << pArgValue->pos()
                       << " In function (" << pValue->path()
                       << "), keyword " << argName << " does not match any in the schema."
                       << std::flush;
                pValidationResults->push_back(stream.str());
            }
            allMatch = false;
            continue;
        }
        Attribute& attr = attrIter->second;

        // Mark this attribute as visited
        visitedRequiredAttributes[attrIter->first] = true;

        // Check if the type matches what's available
        if (!Schema::typeMatches(pArgValue, manager, attr.types()))
        {
            if (pValidationResults != NULL)
            {
                std::ostringstream stream;
                stream << pArgValue->file() << ':' << pArgValue->line() << ':'
                       << pArgValue->pos()
                       << " In function (" << pValue->path()
                       << "), argument " << argName << " does not match any type "
                       << "in the schema for that keyword; type is "
                       << typeNameToString(pArgValue->typeName())
                       << " but available types were: "
                       << attr.typesListAsString() << std::flush;
                pValidationResults->push_back(stream.str());
            }
            allMatch = false;
            continue;
        }

        // Check the arg recursively
        if (recursiveValidation)
        {
            bool matches = manager.validate(pArgValue,
                                            pValidationResults,
                                            recursiveValidation);
            allMatch = allMatch && matches;
        }
    }

    // List missing required arguments, if any
    for (std::map<std::string, bool>::iterator visitedIter = visitedRequiredAttributes.begin();
         visitedIter != visitedRequiredAttributes.end();
         ++visitedIter)
    {
        if (visitedIter->second == false)
        {
            if (pValidationResults != NULL)
            {
                std::ostringstream stream;
                stream << pValue->file() << ':' << pValue->line() << ':'
                       << pValue->pos()
                       << " In function (" << pValue->path()
                       << "), argument " << visitedIter->first
                       << " was not found, but is required." << std::flush;
                pValidationResults->push_back(stream.str());
            }
            allMatch = false;
        }
    }

    return allMatch;
}


    } // namespace Rsd
} // namespace RenderSpud
