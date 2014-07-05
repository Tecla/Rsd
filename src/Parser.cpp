////////////
//
//  File:      Parser.cpp
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved
//  Content:   RenderSpud scene data parser
//
////////////

#include <string>
#include <vector>

#include <Rsd/Parser.h>

#include "Tokenizer.h"


//
// Main grammar interface
//

void  ParseFree(void *p,                  // The parser to be deleted
                void (*freeProc)(void*)); // Function used to reclaim memory
void* ParseAlloc(void *(*mallocProc)(size_t));
void  ParseTrace(FILE *traceFile, char *zTracePrompt);
void  Parse(void *yyp,                                     // The parser
            int yymajor,                                   // The major token code number
            RenderSpud::Rsd::Parser::Token* yyminor,       // The value for the token
            RenderSpud::Rsd::Parser::ParserState *pState); // Optional %extra_argument parameter


//
// Reference grammar interface
//

void  ReferenceParseFree(void *p,                  // The parser to be deleted
                         void (*freeProc)(void*)); // Function used to reclaim memory
void* ReferenceParseAlloc(void *(*mallocProc)(size_t));
void  ReferenceParseTrace(FILE *traceFile, char *zTracePrompt);
void  ReferenceParse(void *yyp,                                     // The parser
                     int yymajor,                                   // The major token code number
                     RenderSpud::Rsd::Parser::Token* yyminor,       // The value for the token
                     RenderSpud::Rsd::Parser::ParserState *pState); // Optional %extra_argument parameter


namespace RenderSpud
{
    namespace Rsd
    {
        namespace Parser
        {


enum GrammarVariant
{
    kMainGrammar,
    kReferenceGrammar
};

int tokenTypeToLemonId(GrammarVariant variant, TokenType type)
{
    if (variant == kMainGrammar)
    {
        // These ones match automatically
        return static_cast<int>(type);
    }
    else if (variant == kReferenceGrammar)
    {
        switch (type)
        {
        case kTokenIdentifier:
            return kReferenceToken_IDENTIFIER;
        case kTokenAssign:
            return kReferenceToken_ASSIGN;
        case kTokenColon:
            return kReferenceToken_COLON;
        case kTokenAt:
            return kReferenceToken_AT;
        case kTokenSemicolon:
            return kReferenceToken_SEMICOLON;
        case kTokenComma:
            return kReferenceToken_COMMA;
        case kTokenDot:
            return kReferenceToken_DOT;
        case kTokenFloat:
            return kReferenceToken_FLOAT;
        case kTokenInteger:
            return kReferenceToken_INTEGER;
        case kTokenBoolean:
            return kReferenceToken_BOOLEAN;
        case kTokenString:
            return kReferenceToken_STRING;
        case kTokenLeftParen:
            return kReferenceToken_LEFTPAREN;
        case kTokenRightParen:
            return kReferenceToken_RIGHTPAREN;
        case kTokenLeftCurlyBracket:
            return kReferenceToken_LEFTCURLYBRACKET;
        case kTokenRightCurlyBracket:
            return kReferenceToken_RIGHTCURLYBRACKET;
        case kTokenLeftSquareBracket:
            return kReferenceToken_LEFTSQUAREBRACKET;
        case kTokenRightSquareBracket:
            return kReferenceToken_RIGHTSQUAREBRACKET;
        case kTokenInclude:
            return kReferenceToken_INCLUDE;
        default:
            return static_cast<int>(kTokenInvalid);
        }
    }
    return static_cast<int>(kTokenInvalid);
}


void Parser::parse(const std::string& input, Value& root)
{
    //
    // Tokenize / parse input into AST
    //

    ParserState state(root);
    void *pParser = ParseAlloc(&(::operator new));
    size_t index = 0;
    std::vector<Token*> tokens;
    while (index < input.length())
    {
        tokens.push_back(new Token());
        Token& token = *tokens.back();
        try
        {
            index = Token::parseToken(index,
                                      input,
                                      token,
                                      state.m_currentLine,
                                      state.m_currentPosition);
        }
        catch (TokenException tokenException)
        {
            // Rethrow token exceptions as parse errors, with more information
            throw ParseException(std::string("Syntax error: invalid token '") +
                                     input[index] + "'",
                                 state.m_currentSource,
                                 state.m_currentLine,
                                 state.m_currentPosition);
        }

        // Feed tokens to the parser
        if (token.type() != kTokenWhitespace)
        {
            Parse(pParser,
                  tokenTypeToLemonId(kMainGrammar, token.type()),
                  &token,
                  &state);
        }
    }

    // Clean up the parser
    Parse(pParser, 0, NULL, &state);
    ParseFree(pParser, &(::operator delete));

    // Clean up all the tokens we allocated
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        delete tokens[i];
    }
}


void Parser::parseReference(const std::string& input, Reference& ref)
{
    //
    // Tokenize / parse input into AST
    //

    ParserState state(ref);
    void *pParser = ReferenceParseAlloc(&(::operator new));
    size_t index = 0;
    std::vector<Token*> tokens;
    while (index < input.length())
    {
        tokens.push_back(new Token());
        Token& token = *tokens.back();
        try
        {
            index = Token::parseToken(index,
                                      input,
                                      token,
                                      state.m_currentLine,
                                      state.m_currentPosition);
        }
        catch (TokenException tokenException)
        {
            // Rethrow token exceptions as parse errors, with more information
            throw ParseException(std::string("Syntax error: invalid token '") +
                                     input[index] + "'",
                                 state.m_currentSource,
                                 state.m_currentLine,
                                 state.m_currentPosition);
        }

        // Feed tokens to the parser
        if (token.type() != kTokenWhitespace)
        {
            ReferenceParse(pParser,
                           tokenTypeToLemonId(kReferenceGrammar, token.type()),
                           &token,
                           &state);
        }
    }

    // Clean up the parser
    ReferenceParse(pParser, 0, NULL, &state);
    ReferenceParseFree(pParser, &(::operator delete));

    // Clean up all the tokens we allocated
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        delete tokens[i];
    }
}


        } // namespace Parser
    } // namespace Rsd
} // namespace RenderSpud
