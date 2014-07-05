////////////
//
//  File:      Tokenizer.h
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data tokenizer
//
////////////

#ifndef __RSD_Tokenizer_h__
#define __RSD_Tokenizer_h__

#include <string>
#include <exception>

#include <Rsd/Base.h>

#ifndef __RSD_GrammarMain_h__
    #include "GrammarMain.h"
    #define __RSD_GrammarMain_h__
#endif

#ifndef __RSD_GrammarReference_h__
    #include "GrammarReference.h"
    #define __RSD_GrammarReference_h__
#endif


namespace RenderSpud
{
    namespace Rsd
    {
        namespace Parser
        {


/// Token exception (thrown and caught internally to the parser)
class TokenException : public Exception
{
public:
    TokenException(size_t line, size_t pos, const char* pMessage = NULL) throw()
        : Exception(), m_line(line), m_position(pos), m_pMessage(pMessage) { }

    virtual ~TokenException() throw() { }

    /// Description of the token error
    virtual const char* what() const throw() { return m_pMessage; }

    /// What line the bad token was found on
    size_t line()     const throw() { return m_line; }
    /// What position in the input the bad token was found on
    size_t position() const throw() { return m_position; }

protected:
    size_t m_line;
    size_t m_position;
    const char* m_pMessage;
};


/// Available token types
enum TokenType
{
    kTokenIdentifier            = kToken_IDENTIFIER,         // Standard C-like identifier rules
    kTokenAssign                = kToken_ASSIGN,             // = (name to value association)
    kTokenColon                 = kToken_COLON,              // : (macro keyword and inheritance)
    kTokenAt                    = kToken_AT,                 // @ (type specifications)
    kTokenSemicolon             = kToken_SEMICOLON,          // ; (end of value in block)
    kTokenComma                 = kToken_COMMA,              // , (end of value in array/arg list)
    kTokenDot                   = kToken_DOT,                // . (path or type name separator)
    kTokenFloat                 = kToken_FLOAT,              // All standard floating-point formats
    kTokenInteger               = kToken_INTEGER,            // Decimal integer format
    kTokenBoolean               = kToken_BOOLEAN,            // true or false
    kTokenString                = kToken_STRING,             // "..."
    kTokenLeftParen             = kToken_LEFTPAREN,          // ( (macro arg list start)
    kTokenRightParen            = kToken_RIGHTPAREN,         // ) (macro arg list end)
    kTokenLeftCurlyBracket      = kToken_LEFTCURLYBRACKET,   // { (block start)
    kTokenRightCurlyBracket     = kToken_RIGHTCURLYBRACKET,  // } (block end)
    kTokenLeftSquareBracket     = kToken_LEFTSQUAREBRACKET,  // [ (array/subscript start)
    kTokenRightSquareBracket    = kToken_RIGHTSQUAREBRACKET, // ] (array/subscript end)
    kTokenInclude               = kToken_INCLUDE,            // include
    kTokenInvalid               = 100000,
    kTokenWhitespace            = 100001,                    // \t, ,\r,\n
    kTokenComment               = kTokenWhitespace           // //...\n
};


/// Token class, holds only primitive parsed operators and values
class Token
{
public:
    Token() : m_type(kTokenInvalid), m_boolValue(false), m_intValue(0),
              m_floatValue(0.0), m_textValue(), m_line(0), m_pos(0) { }

    Token(const Token& t)
        : m_type(t.m_type), m_boolValue(t.m_boolValue), m_intValue(t.m_intValue),
          m_floatValue(t.m_floatValue), m_textValue(t.m_textValue),
          m_line(t.m_line), m_pos(t.m_pos) { }

    Token& operator =(const Token& t)
    {
        m_type = t.m_type;
        m_boolValue = t.m_boolValue;
        m_intValue = t.m_intValue;
        m_floatValue = t.m_floatValue;
        m_textValue = t.m_textValue;
        m_line = t.m_line;
        m_pos = t.m_pos;
        return *this;
    }

    /// Type of this token
    TokenType type() const         { return m_type; }
    void      setType(TokenType t) { m_type = t; }

    bool isValid()      const { return m_type != kTokenInvalid; }
    bool isWhitespace() const { return m_type == kTokenWhitespace; }
    bool isComment()    const { return m_type == kTokenWhitespace && m_textValue.length() > 0; }
    bool isIdentifier() const { return m_type == kTokenIdentifier; }
    bool isString()     const { return m_type == kTokenString; }
    bool isInteger()    const { return m_type == kTokenInteger; }
    bool isFloat()      const { return m_type == kTokenFloat; }
    bool isBoolean()    const { return m_type == kTokenBoolean; }

    bool   booleanValue() const { return m_boolValue; }
    long   integerValue() const { return m_intValue; }
    double floatValue()   const { return m_floatValue; }
    /// For identifiers and strings, and comments (whitespace with "text")
    const std::string& textValue() const { return m_textValue; }

    void setBooleanValue(bool v)            { m_boolValue = v; }
    void setIntegerValue(long v)            { m_intValue = v; }
    void setFloatValue(double v)            { m_floatValue = v; }
    void setTextValue(const std::string& v) { m_textValue = v; }

    /// Line this token was encountered
    size_t line() const { return m_line; }
    size_t pos()  const { return m_pos; }
    void setLine(size_t line) { m_line = line; }
    void setPos(size_t pos)   { m_pos = pos; }


    /**
     * Parse a token given an input stream
     * @param startingIndex Start index in the string where we begin parsing
     * @param inputString Input string to parse a token from
     * @param outputToken Token parsed out will be placed in this value
     * @param inOutLineNumber The current line number; modified to be line number for outputToken
     * @return Index in the input string just after the token we parsed
     */
    static size_t parseToken(size_t startingIndex,
                             const std::string& inputString,
                             Token& outputToken,
                             size_t& inOutLineNumber,
                             size_t& inOutPosNumber);

    /// Textual representation of this token
    std::string description() const;

private:
    TokenType m_type;
    bool m_boolValue;
    long m_intValue;
    double m_floatValue;
    std::string m_textValue;
    size_t m_line, m_pos;
};


        } // namespace Parser
    } // namespace Rsd
} // namespace RenderSpud


#endif // __RSD_Tokenizer_h__
