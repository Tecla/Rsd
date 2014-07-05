////////////
//
//  File:      TypeName.cpp
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data type names
//
////////////

#include <sstream>

#include <Rsd/TypeName.h>


namespace RenderSpud
{
    namespace Rsd
    {


TypeName typeNameFromString(const std::string& typeStr)
{
    TypeName result;
    for (size_t index = 0;
         index < typeStr.size() && index != std::string::npos;
         )
    {
        size_t nextSep = typeStr.find_first_of('.', index);
        if (nextSep == std::string::npos)
            nextSep = typeStr.size();
        result.push_back(typeStr.substr(index, nextSep - index));
        index = nextSep + 1;
    }
    return result;
}


std::string typeNameToString(const TypeName& type)
{
    std::ostringstream stream;
    for (size_t i = 0; i < type.size(); ++i)
    {
        stream << type[i];
        if (i < type.size() - 1)
            stream << '.';
    }
    return stream.str();
}


    } // namespace Rsd
} // namespace RenderSpud
