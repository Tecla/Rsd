////////////
//
//  File:      Parser.h
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data parser
//
////////////

#ifndef __RSD_Parser_h__
#define __RSD_Parser_h__

#include <string>
#include <exception>

#include <Rsd/Value.h>


namespace RenderSpud
{
    namespace Rsd
    {
        namespace Parser
        {


/// Main parser class for RSD files
class Parser
{
public:
    Parser() { }

    /// Parse into a block value
    void parse(const std::string& input, Value& root);

    /// Parse into a reference
    void parseReference(const std::string& input, Reference& ref);
};


//
// Parser help
//

/// Parser helper; used during parsing to remember the current lines, track stats, etc.
struct ParserState
{
    Value *m_pRoot;
    Reference *m_pRootReference;
    std::string m_currentSource;
    size_t m_currentLine;
    size_t m_currentPosition;
    size_t m_numTokensDestroyed;

    ParserState(Value& root)
        : m_pRoot(&root),
          m_pRootReference(NULL),
          m_currentSource(),
          m_currentLine(1),
          m_currentPosition(0),
          m_numTokensDestroyed(0)
    {

    }

    ParserState(Reference& root)
        : m_pRoot(NULL),
          m_pRootReference(&root),
          m_currentSource(),
          m_currentLine(1),
          m_currentPosition(0),
          m_numTokensDestroyed(0)
    {

    }
};


/// Exception thrown during parsing on token/syntax errors.
class ParseException : public Exception
{
public:
    ParseException(const std::string& desc,
                   const std::string& source,
                   size_t line,
                   size_t pos)
        : Exception(), m_desc(desc), m_source(source), m_line(line), m_pos(pos) { }

    virtual ~ParseException() throw() { }

    /// A description of the parse error
    virtual const char* what() const throw()
    {
        return m_desc.c_str();
    }

    /// Description
    const std::string& description() const { return m_desc; }
    /// Source (usually a file) the error occurred in
    const std::string& source() const { return m_source; }
    /// Line from the source file the error occurred on
    size_t line() const { return m_line; }
    /// Position from the source file the error occurred on
    size_t pos() const { return m_pos; }

private:
    std::string m_desc;
    std::string m_source;
    size_t m_line, m_pos;
};


        } // namespace Parser
    } // namespace Rsd
} // namespace RenderSpud


#endif // __RSD_Parser_h__
