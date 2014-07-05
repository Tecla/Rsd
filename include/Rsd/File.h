////////////
//
//  File:      File.h
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data file format
//
////////////

#ifndef __RSD_File_h__
#define __RSD_File_h__

#include <istream>
#include <string>
#include <vector>

#include <Rsd/Value.h>


namespace RenderSpud
{
    namespace Rsd
    {


/// This exception is thrown if any file open/read/write operations fail.
class FileIOException : public Exception
{
public:
    FileIOException(const std::string& filename, const std::string& failedOperation)
        : Exception()
    {
        m_desc = "The file \"" + filename + "\" could not be " + failedOperation + "!";
    }

    virtual ~FileIOException() throw() { }

    virtual const char* what() const throw()
    {
        return m_desc.c_str();
    }

private:
    std::string m_desc;
};



//
// File
//

class File : public Value
{
public:
    typedef RenderSpud::Ptr<File> FilePtr;
    typedef RenderSpud::Ptr<const File> ConstFilePtr;


public:
    File();
    File(const std::string& filename,
         bool openIncludes = true);
    File(std::istream& inputStream,
         const std::string& streamName,
         const std::string& pathBase = ".",
         bool openIncludes = true);
    File(const std::string& bufferString,
         const std::string& bufferName,
         const std::string& pathBase = ".",
         bool openIncludes = true);
    File(const char* buffer,
         const std::string& bufferName,
         const std::string& pathBase = ".",
         bool openIncludes = true);


    //
    // Environment
    //

    /// Typical usage of this is to add your environment variables that you want
    /// to be looked up last after all other lookups/substitutions fail.  Usually
    /// you want to add a few of your own values:
    ///     file.environment().appendValue(101, "frame");
    const Value::Ptr environment() const { return m_pEnvironment; }

    /// Typical usage of this is to add your environment variables that you want
    /// to be looked up last after all other lookups/substitutions fail.  Usually
    /// you want to add a few of your own values:
    ///     file.environment()->appendValue(101, "frame");
    Value::Ptr environment() { return m_pEnvironment; }

    /// Wholesale set the \ref Value acting as the environment; should be a block.
    void setEnvironment(Value::Ptr pEnvNode) { m_pEnvironment = pEnvNode; }

    /// Add the shell environment to the environment Node.  Often, you will want
    /// the shell environment to be a fallback.  Call this to automatically suck
    /// in the shell environment as name/string value pairs.
    void addShellEnvironment();

    /// Explicit environment lookup.  You could just as easily call
    /// environment().findValue("name") as well, but this is here to make it
    /// obvious what you are doing.
    Value::Ptr lookupEnvironmentValue(const std::string& name)
    {
        return m_pEnvironment->find(name);
    }


    //
    // Search / filtering
    //

    virtual Value::ConstPtr find(const Reference& ref) const;
    virtual Value::Ptr      find(const Reference& ref);

    virtual Value::ConstResolved resolve(const Value& v) const;
    virtual Value::Resolved      resolve(Value& v);


    //
    // I/O
    //

    /// \brief Write this node's data to a stream.
    ///
    /// Indentation can be used to indicate how deeply nested (scoped) this node
    /// is.  Throws a FileIOException if the stream could not be written.
    ///
    /// There are two variants of write(...), one if you have a stream you want
    /// to write to, and one if you have a file you want to write directly to.
    ///
    /// @param stream         The stream to write to.
    /// @param followIncludes Optional, whether includes should be replaced by the contents of each include file.
    /// @param indentation    Optional, how far to indent the written data, useful for indicating nesting (internally, 4 spaces are used for each recursive indentation level).
    void write(std::ostream& stream,
               bool followIncludes = false,
               size_t indentation = 0);

    /// Write this node's data to a file.  See the other \ref write method docs for more info.
    /// @param filename       The file to write to.
    /// @param followIncludes Optional, whether includes should be replaced by the contents of each include file.
    /// @param indentation    Optional, how far to indent the written data, useful for indicating nesting (internally, 4 spaces are used for each recursive indentation level).
    void write(const std::string& filename,
               bool followIncludes = false,
               size_t indentation = 0);


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
    virtual ~File();

    Value::Ptr m_pEnvironment;

    typedef std::vector<std::string> FileIndexMap;
    FileIndexMap m_fileIndexMap;

    /// Parse a string buffer of data, and assign newly created nodes the file
    /// index.  Throws on parser errors.
    void openBuffer(const std::string& input,
                    FileIndexMap& fileIndexMap,
                    const std::string& pathBase = ".",
                    bool openIncludes = true);

    /// Fixup a value and process includes if necessary
    void processValue(Value& v,
                      FileIndex fileIndex,
                      FileIndexMap& fileIndexMap,
                      const std::string& pathBase,
                      bool openIncludes = true);

    /// Map a file index to a filename, if available
    virtual std::string file(FileIndex index) const;
};


    } // namespace Rsd
} // namespace RenderSpud


#endif // __RSD_File_h__
