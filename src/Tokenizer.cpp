////////////
//
//  File:      Tokenizer.cpp
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data tokenizer
//
////////////

#include <string>
#include <cmath>
#include <map>

#include "Tokenizer.h"


namespace RenderSpud
{
    namespace Rsd
    {
        namespace Parser
        {


struct TokenTypeAndDesc
{
    size_t m_type;
    std::string m_desc;
};


TokenTypeAndDesc sTokenTypeToDesc[] =
{
    { 0, "<none>" },
    { 100000, "<none>" },
    { 100001, "whitespace" },
    { kToken_ASSIGN, "=" },
    { kToken_COLON, ":" },
    { kToken_AT, "@" },
    { kToken_SEMICOLON, ";" },
    { kToken_INCLUDE, "include" },
    { kToken_STRING, "string" },
    { kToken_IDENTIFIER, "identifier" },
    { kToken_DOT, "." },
    { kToken_LEFTPAREN, "(" },
    { kToken_RIGHTPAREN, ")" },
    { kToken_INTEGER, "integer" },
    { kToken_FLOAT, "float" },
    { kToken_BOOLEAN, "boolean" },
    { kToken_LEFTCURLYBRACKET, "{" },
    { kToken_RIGHTCURLYBRACKET, "}" },
    { kToken_LEFTSQUAREBRACKET, "[" },
    { kToken_RIGHTSQUAREBRACKET, "]" },
    { kToken_COMMA, "," }
};


const size_t kNumTokenTypes = sizeof(sTokenTypeToDesc) / sizeof(TokenTypeAndDesc);


std::string Token::description() const
{
    for (size_t i = 0; i < kNumTokenTypes; i++)
    {
        if (sTokenTypeToDesc[i].m_type == (size_t)m_type)
            return sTokenTypeToDesc[i].m_desc;
    }
    return sTokenTypeToDesc[0].m_desc;
}


static const char* sTokenMessages[] =
{
    "Unterminated comment",
    "Unterminated string literal",
    "Invalid number literal",
    "Invalid token"
};


enum TokenMessages
{
    kTokenUnterminatedComment,
    kTokenUnterminatedStringLiteral,
    kTokenInvalidNumberLiteral,
    kTokenInvalidToken
};


inline bool isWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}


inline bool isIdentifierCharacter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}


inline bool isDecimalDigit(char c)
{
    return c >= '0' && c <= '9';
}


inline unsigned int charToInt(char c)
{
    if (isDecimalDigit(c))
        return static_cast<unsigned int>(c - '0');
    return 0;
}


size_t Token::parseToken(size_t startIndex,
                         const std::string& inputString,
                         Token& outputToken,
                         size_t& inOutLineNumber,
                         size_t& inOutPosition)
{
    outputToken.setLine(inOutLineNumber);
    outputToken.setPos(inOutPosition + 1);
    size_t curIndex = startIndex;
    char c = inputString[curIndex];
    if (Parser::isWhitespace(c))
    {
        outputToken.setType(kTokenWhitespace);
        size_t tempStartIndex = startIndex;
        while (Parser::isWhitespace(c))
        {
            if (c == '\r')
            {
                ++curIndex;
                if (curIndex < inputString.length() && inputString[curIndex] != '\n')
                {
                    inOutLineNumber++;
                    inOutPosition = 0;
                    tempStartIndex = curIndex;
                }
            }
            if (c == '\n')
            {
                inOutLineNumber++;
                inOutPosition = 0;
                tempStartIndex = curIndex + 1;
            }
            curIndex++;
            if (curIndex < inputString.length())
                c = inputString[curIndex];
            else
                break;
        }
        // Fixup position due to potential newlines, and return early
        inOutPosition += curIndex - tempStartIndex;
        return curIndex;
    }
    else if (c == '/')
    {
        curIndex++;
        outputToken.setType(kTokenComment);
        if (curIndex < inputString.length())
        {
            c = inputString[curIndex];
            if (c != '/')
            {
                throw TokenException(inOutLineNumber, inOutPosition,
                                     sTokenMessages[kTokenUnterminatedComment]);
            }
        }
        else
        {
            throw TokenException(inOutLineNumber, inOutPosition,
                                 sTokenMessages[kTokenUnterminatedComment]);
        }

        while (c != '\r' && c != '\n')
        {
            curIndex++;
            if (curIndex < inputString.length())
                c = inputString[curIndex];
            else
                break;
        }
        outputToken.setTextValue(inputString.substr(startIndex + 1, curIndex - startIndex - 2));
    }
    else if (c == '=')
    {
        outputToken.setType(kTokenAssign);
        curIndex++;
    }
    else if (c == ':')
    {
        outputToken.setType(kTokenColon);
        curIndex++;
    }
    else if (c == '@')
    {
        outputToken.setType(kTokenAt);
        curIndex++;
    }
    else if (c == ';')
    {
        curIndex++;
        outputToken.setType(kTokenSemicolon);
    }
    else if (c == ',')
    {
        curIndex++;
        outputToken.setType(kTokenComma);
    }
    else if (c == '.' && (curIndex + 1 < inputString.length() &&
                          !isDecimalDigit(inputString[curIndex + 1])))
    {
        // We encountered a . that isn't directly in front of a number, so it is
        // supposed to be an accessor
        curIndex++;
        outputToken.setType(kTokenDot);
    }
    else if (c == '(')
    {
        curIndex++;
        outputToken.setType(kTokenLeftParen);
    }
    else if (c == ')')
    {
        curIndex++;
        outputToken.setType(kTokenRightParen);
    }
    else if (c == '{')
    {
        curIndex++;
        outputToken.setType(kTokenLeftCurlyBracket);
    }
    else if (c == '}')
    {
        curIndex++;
        outputToken.setType(kTokenRightCurlyBracket);
    }
    else if (c == '[')
    {
        curIndex++;
        outputToken.setType(kTokenLeftSquareBracket);
    }
    else if (c == ']')
    {
        curIndex++;
        outputToken.setType(kTokenRightSquareBracket);
    }
    else if (c == '"')
    {
        curIndex++;
        outputToken.setType(kTokenString);
        std::string value;
        if (curIndex < inputString.length())
        {
            c = inputString[curIndex];
        }
        else
        {
            throw TokenException(inOutLineNumber, inOutPosition,
                                 sTokenMessages[kTokenUnterminatedStringLiteral]);
        }

        bool escaped = false;
        while (c != '"' || (c == '"' && escaped))
        {
            if (c == '\\' && !escaped)
            {
                // Don't add the escape indicator
                escaped = true;
            }
            else
            {
                if (escaped)
                {
                    if (c == 'n')
                    {
                        value += '\n';
                    }
                    else if (c == 'r')
                    {
                        value += '\r';
                    }
                    else if (c == 't')
                    {
                        value += '\t';
                    }
                    else
                    {
                        value += c;
                    }
                }
                else
                {
                    value += c;
                }
                escaped = false;
            }
            curIndex++;
            if (curIndex < inputString.length())
            {
                c = inputString[curIndex];
            }
            else
            {
                throw TokenException(inOutLineNumber, inOutPosition,
                                     sTokenMessages[kTokenUnterminatedStringLiteral]);
            }
        }
        curIndex++;
        outputToken.setTextValue(value);
    }
    else if (c == '-' || isDecimalDigit(c) || c == '.')
    {
        bool isNegative;
        if (c == '-')
        {
            isNegative = true;
            curIndex++;
            if (curIndex < inputString.length())
            {
                c = inputString[curIndex];
            }
            else
            {
                throw TokenException(inOutLineNumber, inOutPosition,
                                     sTokenMessages[kTokenInvalidNumberLiteral]);
            }
        }
        else
        {
            isNegative = false;
        }

        long intPart = 0;
        int exponentPart = 0;
        double floatPart = 0.0;
        double floatScale = 1.0;

        bool isFloat = false;
        bool encounteredDecimal = false;
        bool encounteredExponent = false;
        bool encounteredExponentNegative = false;
        bool encounteredDigit = false;

        bool numberDone = false;
        while (!numberDone)
        {
            if (c == '.')
            {
                if (encounteredExponent || encounteredDecimal)
                {
                    throw TokenException(inOutLineNumber, inOutPosition,
                                         sTokenMessages[kTokenInvalidNumberLiteral]);
                }
                isFloat = true;
                encounteredDecimal = true;
            }
            else if (c == 'e' || c == 'E')
            {
                if (!encounteredDigit || encounteredExponent)
                {
                    throw TokenException(inOutLineNumber, inOutPosition,
                                         sTokenMessages[kTokenInvalidNumberLiteral]);
                }
                isFloat = true;
                encounteredExponent = true;
                encounteredDigit = false;
            }
            else if (c == '-' && encounteredExponent && !encounteredExponentNegative && !encounteredDigit)
            {
                encounteredExponentNegative = true;
            }
            else if (isDecimalDigit(c))
            {
                if (encounteredExponent)
                {
                    exponentPart *= 10;
                    exponentPart += charToInt(c);
                }
                else if (encounteredDecimal)
                {
                    floatScale *= 0.1;
                    floatPart += static_cast<double>(charToInt(c)) * floatScale;
                }
                else
                {
                    intPart *= 10;
                    intPart += charToInt(c);
                }
                encounteredDigit = true;
            }
            else
            {
                numberDone = true;
                break;
            }

            curIndex++;
            if (curIndex < inputString.length())
            {
                c = inputString[curIndex];
            }
            else
            {
                numberDone = true;
            }
        }

        if (isFloat)
        {
            double result = static_cast<double>(intPart);
            result += floatPart;
            if (encounteredExponent)
            {
                if (encounteredExponentNegative)
                    exponentPart = -exponentPart;
                result *= pow(10, exponentPart);
            }
            if (isNegative)
                result = -result;
            outputToken.setType(kTokenFloat);
            outputToken.setFloatValue(result);
        }
        else
        {
            if (isNegative)
                intPart = -intPart;
            outputToken.setType(kTokenInteger);
            outputToken.setIntegerValue(intPart);
        }
    }
    else if (isIdentifierCharacter(c))
    {
        // Must be an identifier or keyword
        std::string identifier = "";
        while (isIdentifierCharacter(c) || isDecimalDigit(c))
        {
            identifier += c;
            curIndex++;
            if (curIndex < inputString.length())
                c = inputString[curIndex];
            else
                break;
        }
        if (identifier == "include")
        {
            outputToken.setType(kTokenInclude);
            outputToken.setTextValue(identifier);
        }
        else if (identifier == "true" || identifier == "false")
        {
            outputToken.setType(kTokenBoolean);
            outputToken.setBooleanValue(identifier == "true" ? true : false);
        }
        else
        {
            outputToken.setType(kTokenIdentifier);
            outputToken.setTextValue(identifier);
        }
    }
    else
    {
        throw TokenException(inOutLineNumber, inOutPosition,
                             sTokenMessages[kTokenInvalidToken]);
    }
    inOutPosition += curIndex - startIndex;
    return curIndex;
}


        } // namespace Parser
    } // namespace Rsd
} // namespace RenderSpud
