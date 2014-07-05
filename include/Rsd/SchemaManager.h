////////////
//
//  File:      SchemaManager.h
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data schemas/validation
//
////////////

#ifndef __RSD_SchemaManager_h__
#define __RSD_SchemaManager_h__

#include <vector>
#include <string>
#include <map>

#include <Rsd/File.h>


namespace RenderSpud
{
    namespace Rsd
    {


enum ValidationPolicyPreset
{
    /// Strict policy means no extra attributes allowed
    kPolicyStrict,
    /// Permissive policy means extra attributes are allowed
    kPolicyPermissive
};


/// Strictness settings for node validation
struct ValidationPolicy
{
    /// Are extra members not specified in the schema allowed?  (true for strict)
    bool m_disallowExtraMembers;
    /// Are members required by default? (true for strict)
    bool m_requireMemberByDefault;

    /// Default settings are for strict validation
    ValidationPolicy(ValidationPolicyPreset preset = kPolicyStrict)
        : m_disallowExtraMembers(preset == kPolicyStrict),
          m_requireMemberByDefault(preset == kPolicyStrict) { }
};


class Schema;


/// Schema manager holds all loaded schemas and provides the validation API for nodes
class SchemaManager
{
public:
    SchemaManager(bool loadBuiltinSchemas = true);
    virtual ~SchemaManager();


    /// When a policy is not specified in the schema, these are the defaults
    const ValidationPolicy& defaultPolicy() const    { return m_defaultPolicy; }
    ValidationPolicy&       defaultPolicy()          { return m_defaultPolicy; }
    void setDefaultPolicy(const ValidationPolicy& p) { m_defaultPolicy = p; }

    /// Does every root node need a type, and then needs to be validated?
    bool  validateAllTypedValues() const    { return m_validateAllTypedValues; }
    bool& validateAllTypedValues()          { return m_validateAllTypedValues; }
    void  setValidateAllTypedValues(bool v) { m_validateAllTypedValues = v; }


    //
    // Available schema management
    //

    /// Add a schema based on the value; may be of type 'Block', 'Array', or 'Function'.
    /// @param pSchemaBlock The block value to be added as a schema
    /// @param isBuiltin Optional; whether this is a builtin or not (don't use this unless you know what you are doing)
    virtual void addSchema(Value::Ptr pSchemaBlock, bool isBuiltin = false);
    /// Find schema-typed nodes (Block, Array, Function) and add them as schemas.
    /// @param pRoot The block value that contains schemas
    void addSchemas(Value::Ptr pRoot);

    /// Find the underlying schema data for a given type.  Returns an empty ptr if it none exists.
    /// This is useful for testing if a schema exists for a given type as well.
    Value::Ptr findSchema(const TypeName& type);
    /// Find the underlying schema data for a given type.  Returns an empty ptr if it none exists.
    /// This is useful for testing if a schema exists for a given type as well.
    Value::Ptr findSchema(const std::string& type) { return findSchema(typeNameFromString(type)); }

    /// Used internally to match up to underlying schemas; you won't find this useful.
    Schema* findSchemaForType(const TypeName& type);

    /// Load all schemas from the RSD file or files in the directory or file path.
    bool loadAllSchemas(const std::string& fileOrDirectoryPath);

    /// Clear all loaded schemas, optionally reloading the builtins schemas.
    void clearSchemas(bool reloadBuiltins = true);


    //
    // Node and schema validation
    //

    /// Validate all schemas themselves against the builtin schema for schemas.
    /// @param pValidationResults Optional, the output list of validation errors as text
    /// @return Whether the schemas were all validated
    bool validateAllLoadedSchemas(std::vector<std::string>* pValidationResults = NULL);

    /// Validate a schema node itself against the builtin schema for schemas.
    /// @param pSchemaBlock The schema node (of type 'Class') to be validated
    /// @param options The validation strictness options
    /// @param pValidationResults Optional, the output list of validation errors as text
    /// @return Whether the schema node was validated
    virtual bool validateSchema(Value::Ptr pSchemaBlock,
                                std::vector<std::string>* pValidationResults = NULL);

    /// Validate a value against the available schemas.
    /// @param pValue The value to be validated
    /// @param pValidationResults Optional, the output list of validation errors as text
    /// @param recursiveValidation Optional, whether to validate the items of the value against their types' schemas as well
    /// @param overrideType Optional, a type to use for the value, overriding what it already has
    /// @return Whether the schema node was validated
    virtual bool validate(Value::Ptr pValue,
                          std::vector<std::string>* pValidationResults = NULL,
                          bool recursiveValidation = true,
                          const TypeName& overrideType = TypeName());

protected:
    virtual void addBuiltinSchemas();

    typedef std::multimap<std::string, Schema*> SchemaMap;

    SchemaMap m_schemas;
    ValueArray m_schemaValues;

    /// Force validation of all values with a type
    bool m_validateAllTypedValues;
    /// Default validation policy for schemas that don't specify a policy
    ValidationPolicy m_defaultPolicy;
};


    } // namespace Rsd
} // namespace RenderSpud


#endif // __RSD_SchemaManager_h__
