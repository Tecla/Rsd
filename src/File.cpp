////////////
//
//  File:      File.cpp
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data file format
//
////////////

#include <sstream>
#include <fstream>
#include <cstdlib>

#include <Rsd/File.h>
#include <Rsd/Parser.h>


namespace RenderSpud
{
    namespace Rsd
    {


File::File()
    : Value(kTypeBlock),
      m_pEnvironment(),
      m_fileIndexMap()
{

}


File::File(const std::string& filename,
           bool openIncludes)
    : Value(kTypeBlock),
      m_pEnvironment(),
      m_fileIndexMap()
{
    m_fileIndexMap.push_back(filename);

    // Read all of the input into a string buffer
    std::string inputBuffer;
    std::ifstream inputStream(filename.c_str());
    if (inputStream.fail())
    {
        throw FileIOException(filename, "opened");
    }
    while (inputStream.good() && !inputStream.eof())
    {
        std::string inputLine;
        getline(inputStream, inputLine);
        inputBuffer += inputLine + '\n';
    }

    std::string pathBase = "";
    size_t lastSlash = filename.rfind('/');
    if (lastSlash != std::string::npos)
    {
        pathBase = filename.substr(0, lastSlash);
    }
    else
    {
        pathBase = ".";
    }

    openBuffer(inputBuffer, m_fileIndexMap, pathBase, openIncludes);
}


File::File(std::istream& inputStream,
           const std::string& streamName,
           const std::string& pathBase,
           bool openIncludes)
    : Value(kTypeBlock),
      m_pEnvironment(),
      m_fileIndexMap()
{
    m_fileIndexMap.push_back(streamName);

    // Read all of the input into a string buffer
    std::string inputBuffer;
    while (inputStream.good() && !inputStream.eof())
    {
        std::string inputLine;
        getline(inputStream, inputLine);
        inputBuffer += inputLine + '\n';
    }

    openBuffer(inputBuffer, m_fileIndexMap, pathBase, openIncludes);
}


File::File(const std::string& bufferString,
           const std::string& bufferName,
           const std::string& pathBase,
           bool openIncludes)
    : Value(kTypeBlock),
      m_pEnvironment(),
      m_fileIndexMap()
{
    m_fileIndexMap.push_back(bufferName);
    openBuffer(bufferString, m_fileIndexMap, pathBase, openIncludes);
}


File::File(const char* buffer,
           const std::string& bufferName,
           const std::string& pathBase,
           bool openIncludes)
    : Value(kTypeBlock),
      m_pEnvironment(),
      m_fileIndexMap()
{
    m_fileIndexMap.push_back(bufferName);
    openBuffer(buffer, m_fileIndexMap, pathBase, openIncludes);
}


File::~File()
{

}


void File::addShellEnvironment()
{
    if (m_pEnvironment == NULL)
    {
        m_pEnvironment = Value::Ptr(new Value(kTypeMacro));
    }

    for (size_t i = 0; environ[i] != NULL; ++i)
    {
        std::string nameValuePair = environ[i];
        size_t split = nameValuePair.find('=');
        std::string name = nameValuePair.substr(0, split);
        std::string value = (split < nameValuePair.length() - 1) ?
                                nameValuePair.substr(split + 1, nameValuePair.length() - split - 1) :
                                std::string();
        m_pEnvironment->appendValue(name, Value::Ptr(new Value(value)));
    }
}


Value::ConstPtr File::find(const Reference& ref) const
{
    Value::ConstPtr found = Value::find(ref);
    if (!found && m_pEnvironment)
    {
        found = m_pEnvironment->find(ref);
    }
    return found;
}


Value::Ptr File::find(const Reference& ref)
{
    Value::Ptr found = Value::find(ref);
    if (!found && m_pEnvironment)
    {
        found = m_pEnvironment->find(ref);
    }
    return found;
}


void File::write(std::ostream& stream,
                 bool followIncludes,
                 size_t indentation)
{
    if (stream.fail())
        throw FileIOException("output stream", "written");
    for (size_t i = 0; i < indentation; ++i)
    {
        stream << ' ';
    }
    stream << this->str(followIncludes, false, indentation);
    if (stream.fail())
        throw FileIOException("output stream", "written");
}


void File::write(const std::string& filename,
                 bool followIncludes,
                 size_t indentation)
{
    std::ofstream stream(filename.c_str());
    if (stream.fail())
        throw FileIOException(filename, "written");
    write(stream, followIncludes, indentation);
    if (stream.fail())
        throw FileIOException(filename, "written");
}


std::string File::str(bool followIncludes,
                      bool keepInline,
                      size_t indentation) const
{
    if (isInclude() && !followIncludes)
    {
        // Just spit back the original include statement
        return std::string("include \"") + this->name() + "\"";
    }

    std::ostringstream stream;

    for (size_t i = 0; i < m_values.size(); ++i)
    {
        if (!keepInline)
        {
            for (size_t j = 0; j < indentation; ++j)
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
                                   indentation);
        if (!m_values[i]->isInclude() || !followIncludes)
        {
            stream << ';';
        }

        if (i < m_values.size() - 1)
        {
            if (!keepInline)
            {
                stream << '\n';
            }
            else
            {
                stream << ' ';
            }
        }
    }

    return stream.str();
}


void File::openBuffer(const std::string& input,
                      FileIndexMap& fileIndexMap,
                      const std::string& pathBase,
                      bool openIncludes)
{
    try
    {
        // Ensure we don't get deallocated accidentally somewhere along the way if
        // someone picks up a ptr to this.  We are in the constructor, so there
        // is no one yet who has a ptr to this from outside to keep the refcount
        // high enough.
        this->incrementReference();

        Parser::Parser parser;

        FileIndex currentIndex = fileIndexMap.size() - 1;

        try
        {
            parser.parse(input, *this);
        }
        catch (Parser::ParseException& e)
        {
            // Re-throw parser exceptions with the correct source name
            throw Parser::ParseException(e.description(),
                                         this->file(currentIndex),
                                         e.line(),
                                         e.pos());
        }

        fixupContexts();
        processValue(*this, currentIndex, fileIndexMap, pathBase, openIncludes);

        // Undo our temporary refcount change
        this->decrementReferenceNoDestroy();
    }
    catch (...)
    {
        // Undo our temporary refcount change
        this->decrementReferenceNoDestroy();
        // Rethrow the exception
        throw;
    }
}


void File::processValue(Value& v,
                        FileIndex fileIndex,
                        FileIndexMap& fileIndexMap,
                        const std::string& pathBase,
                        bool openIncludes)
{
    v.setFileIndex(fileIndex);
    for (size_t i = 0; i < v.values().size(); ++i)
    {
        processValue(*v.values()[i],
                     fileIndex,
                     fileIndexMap,
                     pathBase,
                     openIncludes);

        if (v.values()[i]->isInclude())
        {
            if (openIncludes)
            {
                FilePtr pInclude = new File();

                pInclude->setTypeName(v.values()[i]->typeName());

                m_fileIndexMap.push_back(pathBase + "/" + v.values()[i]->name());
                pInclude->m_fileIndexMap = m_fileIndexMap;

                const std::string& filename = m_fileIndexMap.back();

                // Read all of the input into a string buffer
                std::string inputBuffer;
                std::ifstream inputStream(filename.c_str());
                if (inputStream.fail())
                {
                    throw FileIOException(filename, "opened");
                }
                while (inputStream.good() && !inputStream.eof())
                {
                    std::string inputLine;
                    getline(inputStream, inputLine);
                    inputBuffer += inputLine + '\n';
                }

                std::string includePathBase = "";
                size_t lastSlash = filename.rfind('/');
                if (lastSlash != std::string::npos)
                {
                    includePathBase = filename.substr(0, lastSlash);
                }

                pInclude->openBuffer(inputBuffer, m_fileIndexMap, includePathBase, openIncludes);
                v.setValue(i, pInclude);
            }
            else
            {
                // Put the include out there, but let it be empty
                FilePtr pInclude = new File();
                pInclude->setTypeName(v.values()[i]->typeName());
                v.setValue(i, pInclude);
            }
        }
    }
}


Value::ConstResolved File::resolve(const Value& v) const
{
    Value::ConstResolved resolved = Value::resolve(v);
    if (!resolved.second && m_pEnvironment)
    {
        resolved = m_pEnvironment->resolve(v);
    }
    return resolved;
}


Value::Resolved File::resolve(Value& v)
{
    Value::Resolved resolved = Value::resolve(v);
    if (!resolved.second && m_pEnvironment)
    {
        resolved = m_pEnvironment->resolve(v);
    }
    return resolved;
}


std::string File::file(FileIndex index) const
{
    if (index != kNotFromFile && index < m_fileIndexMap.size())
    {
        return m_fileIndexMap[index];
    }
    return std::string();
}


    } // namespace Rsd
} // namespace RenderSpud
