////////////
//
//  File:      TypeName.h
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data type names
//
////////////

#ifndef __RSD_TypeName_h__
#define __RSD_TypeName_h__

#include <vector>
#include <string>


namespace RenderSpud
{
    namespace Rsd
    {


//
// TypeName
//

typedef std::vector<std::string> TypeName;

/// Convert from a string ("type.subtype.subsubtype....") rep to a \ref TypeName
TypeName    typeNameFromString(const std::string& typeStr);
/// Convert from a \ref TypeName to a string rep ("type.subtype.subsubtype...")
std::string typeNameToString(const TypeName& type);


    } // namespace Rsd
} // namespace RenderSpud


#endif // __RSD_TypeName_h__
