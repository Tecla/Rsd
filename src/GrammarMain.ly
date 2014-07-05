////////////
//
//  File:      GrammarMain.ly
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved.
//  Content:   RSD grammar for the lemon parser generator
//
////////////

%include
{
////////////
//
//  File:      GrammarMain.cpp
//  Module:    RSD
//  Author:    Michael Farnsworth
//  Copyright: (C)2012 by Michael Farnsworth, All Rights Reserved.
//  Content:   Generated RSD parser from the lemon parser generator
//
////////////

#include <sstream>
#include <iostream>
#include <cassert>
#include <list>
#include <new>

#include <Rsd/Parser.h>
#include <Rsd/Value.h>
#include <Rsd/Macro.h>
#include <Rsd/Reference.h>

#include "Tokenizer.h"

using namespace RenderSpud::Rsd;
using namespace RenderSpud::Rsd::Parser;

/*

RSD grammar (EBNF style):


subscript-value ::= macro | reference | <integer> | <string>

subscriptSequence ::= '[' subscript-value ']' |
                      subscript-sequence '[' subscript-value ']'

complex-identifier ::= <identifier> | <identifier> subscript-sequence

reference ::= complex-identifier | reference '.' complex-identifier

keyword-argument-list ::= <identifier> ':' node-value |
                          keyword-argument-list ',' <identifier> ':' node-value

macro ::= <identifier> '(' ')' | <identifier> '(' keyword-argument-list ')'

value-list ::= value-list ',' value | value

array ::= '[' value-list ']' | '[' ']'

inherit-reference ::= ':' reference

block ::= '{' node-list '}' | inherit-reference '{' node-list '}'

value ::= array | macro | reference | block | <boolean> | <integer> | <float> | <string>

type-sequence ::= node-name | node-name '.' type-sequence

type ::= '@' type-sequence

node-value ::= type value | value

node-name ::= <identifier> | <string>

node ::= node-name '=' node-value ';' |
         'include' <string> ';'

node-list ::= node node-list | <empty>

*/

struct ValueInBlock
{
    Value::Ptr m_pValue;
    std::string m_name;
    bool m_isInclude;

    ValueInBlock(Value::Ptr pValue, const std::string& name, bool isInclude)
        : m_pValue(pValue), m_name(name), m_isInclude(isInclude) { }
};

}

%extra_argument { ParserState* pState }
%default_type { Token* }

%token_type { Token* }
%token_destructor { pState->m_numTokensDestroyed++; /* Don't need to actually delete anything */ }

%token_prefix kToken_

%stack_size 10000
%start_symbol start
%syntax_error
{
    std::ostringstream stream;
    if (TOKEN != NULL)
    {
        stream << "Syntax error; token: " << TOKEN->description();
        if (TOKEN->textValue().length() > 0)
        {
            stream << ", value: " << TOKEN->textValue();
        }
        throw ParseException(stream.str(),
                             pState->m_currentSource,
                             TOKEN->line(),
                             TOKEN->pos());
    }
    else
    {
        stream << "Syntax error at end of file. (Are you missing a semicolon?)";
        throw ParseException(stream.str(),
                             pState->m_currentSource,
                             pState->m_currentLine,
                             pState->m_currentPosition);
    }
}


%type start { Value::Ptr* }
start ::= nodeList(A).
{
    for (std::list<ValueInBlock>::iterator iter = A->begin();
         iter != A->end();
         ++iter)
    {
        pState->m_pRoot->appendValue(iter->m_name, iter->m_pValue);
    }
    delete A;
}

%type nodeList { std::list<ValueInBlock>* }
nodeList(R) ::= nodeList(A) node(B).
{
    R = A;
    R->push_back(*B);
    delete B;
}
nodeList(R) ::= .
{
    R = new std::list<ValueInBlock>();
}

%type node { ValueInBlock* }
node(R) ::= nodeName(A) ASSIGN nodeValue(B) SEMICOLON.
{
    R = new ValueInBlock(*B, A->textValue(), false);
    delete B;
}
node(R) ::= INCLUDE STRING(A) SEMICOLON.
{
    R = new ValueInBlock(Value::Ptr(new Value()), A->textValue(), true);
    R->m_pValue->typeName().push_back("include");
}

%type nodeName { Token* }
nodeName(R) ::= STRING(A).
{
    R = A;
}
nodeName(R) ::= IDENTIFIER(A).
{
    R = A;
}

%type typeSequence { TypeName* }
typeSequence(R) ::= nodeName(A).
{
    R = new TypeName();
    R->push_back(A->textValue());
}
typeSequence(R) ::= typeSequence(A) DOT nodeName(B).
{
    R = A;
    R->push_back(B->textValue());
}

%type type { TypeName* }
type(R) ::= AT typeSequence(A).
{
    R = A;
}

%type nodeValue { Value::Ptr* }
nodeValue(R) ::= type(A) value(B).
{
    R = B;
    (*R)->setTypeName(*A);
    delete A;
}
nodeValue(R) ::= value(A).
{
    R = A;
}

%type value { Value::Ptr* }
value(R) ::= array(A).
{
    R = A;
}
value(R) ::= macro(A).
{
    R = new Value::Ptr(new Value(*A));
    delete A;
    (*R)->setLine(pState->m_currentLine);
    (*R)->setPos(pState->m_currentPosition);
}
value(R) ::= reference(A).
{
    R = new Value::Ptr(new Value(*A));
    delete A;
    (*R)->setLine(pState->m_currentLine);
    (*R)->setPos(pState->m_currentPosition);
}
value(R) ::= block(A).
{
    R = A;
}
value(R) ::= INTEGER(A).
{
    R = new Value::Ptr(new Value(A->integerValue()));
    (*R)->setLine(A->line());
    (*R)->setPos(A->pos());
}
value(R) ::= FLOAT(A).
{
    R = new Value::Ptr(new Value(A->floatValue()));
    (*R)->setLine(A->line());
    (*R)->setPos(A->pos());
}
value(R) ::= STRING(A).
{
    R = new Value::Ptr(new Value(A->textValue()));
    (*R)->setLine(A->line());
    (*R)->setPos(A->pos());
}
value(R) ::= BOOLEAN(A).
{
    R = new Value::Ptr(new Value(A->booleanValue()));
    (*R)->setLine(A->line());
    (*R)->setPos(A->pos());
}

%type block { Value::Ptr* }
block(R) ::= LEFTCURLYBRACKET(A) nodeList(B) RIGHTCURLYBRACKET.
{
    std::vector<std::string> valueNames;
    ValueArray values;
    for (std::list<ValueInBlock>::iterator iter = B->begin();
         iter != B->end();
         ++iter)
    {
        valueNames.push_back(iter->m_name);
        values.push_back(iter->m_pValue);
    }
    R = new Value::Ptr(new Value(valueNames, values));
    delete B;
    (*R)->setLine(A->line());
    (*R)->setPos(A->pos());
}
block(R) ::= COLON reference(A) LEFTCURLYBRACKET(B) nodeList(C) RIGHTCURLYBRACKET.
{
    std::vector<std::string> valueNames;
    ValueArray values;
    for (std::list<ValueInBlock>::iterator iter = C->begin();
         iter != C->end();
         ++iter)
    {
        valueNames.push_back(iter->m_name);
        values.push_back(iter->m_pValue);
    }
    R = new Value::Ptr(new Value(valueNames, values));
    delete C;
    Value::Ptr pRef(new Value(A));
    delete A;
    (*R)->setLine(B->line());
    (*R)->setPos(B->pos());
    (*R)->setInheritedBlock(pRef);
}

%type array { Value::Ptr* }
array(R) ::= LEFTSQUAREBRACKET(A) valueList(B) RIGHTSQUAREBRACKET.
{
    ValueArray arrayValues;
    for (std::list<Value::Ptr>::iterator iter = B->begin();
         iter != B->end();
         ++iter)
    {
        arrayValues.push_back(*iter);
    }
    R = new Value::Ptr(new Value(arrayValues));
    delete B;
    (*R)->setLine(A->line());
    (*R)->setPos(A->pos());
}
array(R) ::= LEFTSQUAREBRACKET(A) RIGHTSQUAREBRACKET.
{
    ValueArray emptyArrayValues;
    R = new Value::Ptr(new Value(emptyArrayValues));
    (*R)->setLine(A->line());
    (*R)->setPos(A->pos());
}

%type valueList { std::list<Value::Ptr>* }
valueList(R) ::= nodeValue(A).
{
    R = new std::list<Value::Ptr>();
    R->push_back(*A);
    delete A;
}
valueList(R) ::= valueList(A) COMMA nodeValue(B).
{
    R = A;
    R->push_back(*B);
    delete B;
}

%type macro { MacroInvocation::Ptr* }
macro(R) ::= IDENTIFIER(A) LEFTPAREN RIGHTPAREN.
{
    R = new MacroInvocation::Ptr(new MacroInvocation());
    (*R)->setName(A->textValue());
}
macro(R) ::= IDENTIFIER(A) LEFTPAREN keywordArgumentList(B) RIGHTPAREN.
{
    R = B;
    (*R)->setName(A->textValue());
}

%type keywordArgumentList { MacroInvocation::Ptr* }
keywordArgumentList(R) ::= IDENTIFIER(A) COLON nodeValue(B).
{
    R = new MacroInvocation::Ptr(new MacroInvocation());
    (*R)->arguments().insert(std::pair<std::string, Value::Ptr>(A->textValue(), *B));
    delete B;
}
keywordArgumentList(R) ::= keywordArgumentList(A) COMMA IDENTIFIER(B) COLON nodeValue(C).
{
    R = A;
    (*R)->arguments().insert(std::pair<std::string, Value::Ptr>(B->textValue(), *C));
    delete C;
}

%type reference { Reference::Ptr* }
reference(R) ::= complexIdentifier(A).
{
    R = new Reference::Ptr(new Reference());
    (*R)->parts() = *A;
    delete A;
}
reference(R) ::= reference(A) DOT complexIdentifier(B).
{
    R = A;
    (*R)->parts().splice((*R)->parts().end(), *B);
    delete B;
}

%type complexIdentifier { std::list<Reference::Part>* }
complexIdentifier(R) ::= IDENTIFIER(A).
{
    R = new std::list<Reference::Part>();
    Reference::Part p;
    p.m_identifier = A->textValue();
    R->push_back(p);
}
complexIdentifier(R) ::= IDENTIFIER(A) subscriptSequence(B).
{
    R = B;
    Reference::Part p;
    p.m_identifier = A->textValue();
    R->push_front(p);
}

%type subscriptSequence { std::list<Reference::Part>* }
subscriptSequence(R) ::= LEFTSQUAREBRACKET subscriptValue(A) RIGHTSQUAREBRACKET.
{
    R = new std::list<Reference::Part>();
    Reference::Part p;
    p.m_pSubscriptValue = *A;
    R->push_back(p);
    delete A;
}
subscriptSequence(R) ::= subscriptSequence(A) LEFTSQUAREBRACKET subscriptValue(B) RIGHTSQUAREBRACKET.
{
    R = A;
    Reference::Part p;
    p.m_pSubscriptValue = *B;
    R->push_back(p);
    delete B;
}

%type subscriptValue { Value::Ptr* }
subscriptValue(R) ::= macro(A).
{
    R = new Value::Ptr(new Value(*A));
    delete A;
    (*R)->setLine(pState->m_currentLine);
    (*R)->setPos(pState->m_currentPosition);
}
subscriptValue(R) ::= reference(A).
{
    R = new Value::Ptr(new Value(*A));
    delete A;
    (*R)->setLine(pState->m_currentLine);
    (*R)->setPos(pState->m_currentPosition);
}
subscriptValue(R) ::= INTEGER(A).
{
    R = new Value::Ptr(new Value(A->integerValue()));
    (*R)->setLine(A->line());
    (*R)->setPos(A->pos());
}
subscriptValue(R) ::= STRING(A).
{
    R = new Value::Ptr(new Value(A->textValue()));
    (*R)->setLine(A->line());
    (*R)->setPos(A->pos());
}
