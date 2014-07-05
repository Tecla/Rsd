/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>

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

/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    ParseTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 38
#define YYACTIONTYPE unsigned char
#define ParseTOKENTYPE  Token* 
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  TypeName* yy2;
  MacroInvocation::Ptr* yy9;
  std::list<Value::Ptr>* yy10;
  std::list<Reference::Part>* yy14;
  ValueInBlock* yy26;
  std::list<ValueInBlock>* yy35;
  Value::Ptr* yy36;
  Reference::Ptr* yy38;
  Token* yy42;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 10000
#endif
#define ParseARG_SDECL  ParserState* pState ;
#define ParseARG_PDECL , ParserState* pState 
#define ParseARG_FETCH  ParserState* pState  = yypParser->pState 
#define ParseARG_STORE yypParser->pState  = pState 
#define YYNSTATE 70
#define YYNRULE 40
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    53,   15,   18,   12,   51,   52,   54,   19,   17,   13,
 /*    10 */     1,   61,   62,   20,    6,   49,   44,   45,   27,   50,
 /*    20 */    23,   22,   58,   53,   15,   58,   12,   51,   52,   54,
 /*    30 */    19,   42,   13,    1,   25,   28,    6,   49,   44,   45,
 /*    40 */    27,   50,   68,   15,   58,   63,   67,    6,   49,   44,
 /*    50 */    45,   27,   50,   21,   60,   58,    6,   49,   44,   45,
 /*    60 */    27,   50,   46,   64,   58,    6,   49,   44,   45,   27,
 /*    70 */    50,   53,   15,   58,    8,   51,   52,   54,   19,    2,
 /*    80 */    13,    1,   43,   44,   45,   27,   50,   66,   34,   58,
 /*    90 */    66,   34,   58,   33,   36,   58,   70,   35,   10,   31,
 /*   100 */    39,   40,   11,   31,   39,   40,   47,   31,   39,   40,
 /*   110 */   111,    9,   56,   37,   24,    7,   57,   39,   40,   38,
 /*   120 */    41,   30,   26,    8,   14,   17,   16,   29,   32,   48,
 /*   130 */    59,    4,   55,    3,    5,  112,  112,   65,   69,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     4,    5,    5,    7,    8,    9,   10,   11,    6,   13,
 /*    10 */    14,   15,   24,   11,   26,   27,   28,   29,   30,   31,
 /*    20 */    32,   30,   34,    4,    5,   34,    7,    8,    9,   10,
 /*    30 */    11,   23,   13,   14,   24,   35,   26,   27,   28,   29,
 /*    40 */    30,   31,    4,    5,   34,   24,    8,   26,   27,   28,
 /*    50 */    29,   30,   31,   33,   24,   34,   26,   27,   28,   29,
 /*    60 */    30,   31,   34,   24,   34,   26,   27,   28,   29,   30,
 /*    70 */    31,    4,    5,   34,   14,    8,    9,   10,   11,    1,
 /*    80 */    13,   14,   27,   28,   29,   30,   31,   29,   30,   34,
 /*    90 */    29,   30,   34,    5,   36,   34,    0,   36,   21,    3,
 /*   100 */     4,    5,   21,    3,    4,    5,   18,    3,    4,    5,
 /*   110 */    20,   21,   12,   22,   23,   14,   12,    4,    5,    2,
 /*   120 */    23,    5,   25,   14,    6,    6,   17,   16,    4,   18,
 /*   130 */    15,   16,    2,   13,   13,   37,   37,   15,   15,
};
#define YY_SHIFT_USE_DFLT (-5)
#define YY_SHIFT_MAX 36
static const short yy_shift_ofst[] = {
 /*     0 */    -5,   -4,   19,   19,   19,   19,   67,   38,   38,   96,
 /*    10 */   100,  104,  113,   -3,  113,  109,   88,   -3,   60,   -5,
 /*    20 */    -5,  111,    2,  115,   78,  117,  118,  119,  101,  116,
 /*    30 */   120,  124,  130,  121,  119,  122,  123,
};
#define YY_REDUCE_USE_DFLT (-13)
#define YY_REDUCE_MAX 20
static const signed char yy_reduce_ofst[] = {
 /*     0 */    90,  -12,   10,   21,   30,   39,   55,   58,   61,   91,
 /*    10 */    91,   91,   97,   -9,    8,    0,   20,   28,    0,   77,
 /*    20 */    81,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */    72,  110,  110,  110,  110,  110,  110,  110,  110,  110,
 /*    10 */   110,  110,  110,  110,  110,  102,  110,  110,  102,   72,
 /*    20 */    72,  110,  110,  110,  110,  110,   79,   84,  103,  110,
 /*    30 */   110,  110,  110,  110,  107,  110,  110,   71,   73,   75,
 /*    40 */    76,   77,   78,   80,   82,   83,  101,   96,   97,   81,
 /*    50 */    85,   86,   87,   88,   89,   74,   90,   91,  100,   92,
 /*    60 */    95,   93,   94,   99,   98,  104,  106,  108,  109,  105,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  ParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void ParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "ASSIGN",        "SEMICOLON",     "INCLUDE",     
  "STRING",        "IDENTIFIER",    "DOT",           "AT",          
  "INTEGER",       "FLOAT",         "BOOLEAN",       "LEFTCURLYBRACKET",
  "RIGHTCURLYBRACKET",  "COLON",         "LEFTSQUAREBRACKET",  "RIGHTSQUAREBRACKET",
  "COMMA",         "LEFTPAREN",     "RIGHTPAREN",    "error",       
  "start",         "nodeList",      "node",          "nodeName",    
  "nodeValue",     "typeSequence",  "type",          "value",       
  "array",         "macro",         "reference",     "block",       
  "valueList",     "keywordArgumentList",  "complexIdentifier",  "subscriptSequence",
  "subscriptValue",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "start ::= nodeList",
 /*   1 */ "nodeList ::= nodeList node",
 /*   2 */ "nodeList ::=",
 /*   3 */ "node ::= nodeName ASSIGN nodeValue SEMICOLON",
 /*   4 */ "node ::= INCLUDE STRING SEMICOLON",
 /*   5 */ "nodeName ::= STRING",
 /*   6 */ "nodeName ::= IDENTIFIER",
 /*   7 */ "typeSequence ::= nodeName",
 /*   8 */ "typeSequence ::= typeSequence DOT nodeName",
 /*   9 */ "type ::= AT typeSequence",
 /*  10 */ "nodeValue ::= type value",
 /*  11 */ "nodeValue ::= value",
 /*  12 */ "value ::= array",
 /*  13 */ "value ::= macro",
 /*  14 */ "value ::= reference",
 /*  15 */ "value ::= block",
 /*  16 */ "value ::= INTEGER",
 /*  17 */ "value ::= FLOAT",
 /*  18 */ "value ::= STRING",
 /*  19 */ "value ::= BOOLEAN",
 /*  20 */ "block ::= LEFTCURLYBRACKET nodeList RIGHTCURLYBRACKET",
 /*  21 */ "block ::= COLON reference LEFTCURLYBRACKET nodeList RIGHTCURLYBRACKET",
 /*  22 */ "array ::= LEFTSQUAREBRACKET valueList RIGHTSQUAREBRACKET",
 /*  23 */ "array ::= LEFTSQUAREBRACKET RIGHTSQUAREBRACKET",
 /*  24 */ "valueList ::= nodeValue",
 /*  25 */ "valueList ::= valueList COMMA nodeValue",
 /*  26 */ "macro ::= IDENTIFIER LEFTPAREN RIGHTPAREN",
 /*  27 */ "macro ::= IDENTIFIER LEFTPAREN keywordArgumentList RIGHTPAREN",
 /*  28 */ "keywordArgumentList ::= IDENTIFIER COLON nodeValue",
 /*  29 */ "keywordArgumentList ::= keywordArgumentList COMMA IDENTIFIER COLON nodeValue",
 /*  30 */ "reference ::= complexIdentifier",
 /*  31 */ "reference ::= reference DOT complexIdentifier",
 /*  32 */ "complexIdentifier ::= IDENTIFIER",
 /*  33 */ "complexIdentifier ::= IDENTIFIER subscriptSequence",
 /*  34 */ "subscriptSequence ::= LEFTSQUAREBRACKET subscriptValue RIGHTSQUAREBRACKET",
 /*  35 */ "subscriptSequence ::= subscriptSequence LEFTSQUAREBRACKET subscriptValue RIGHTSQUAREBRACKET",
 /*  36 */ "subscriptValue ::= macro",
 /*  37 */ "subscriptValue ::= reference",
 /*  38 */ "subscriptValue ::= INTEGER",
 /*  39 */ "subscriptValue ::= STRING",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Parse and ParseFree.
*/
void *ParseAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  ParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
      /* TERMINAL Destructor */
    case 1: /* ASSIGN */
    case 2: /* SEMICOLON */
    case 3: /* INCLUDE */
    case 4: /* STRING */
    case 5: /* IDENTIFIER */
    case 6: /* DOT */
    case 7: /* AT */
    case 8: /* INTEGER */
    case 9: /* FLOAT */
    case 10: /* BOOLEAN */
    case 11: /* LEFTCURLYBRACKET */
    case 12: /* RIGHTCURLYBRACKET */
    case 13: /* COLON */
    case 14: /* LEFTSQUAREBRACKET */
    case 15: /* RIGHTSQUAREBRACKET */
    case 16: /* COMMA */
    case 17: /* LEFTPAREN */
    case 18: /* RIGHTPAREN */
{
 pState->m_numTokensDestroyed++; /* Don't need to actually delete anything */ 
}
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from ParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_MAX || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( j>=0 && j<YY_SZ_ACTTAB && yy_lookahead[j]==YYWILDCARD ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_MAX ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_MAX );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_SZ_ACTTAB );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   ParseARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
   ParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 20, 1 },
  { 21, 2 },
  { 21, 0 },
  { 22, 4 },
  { 22, 3 },
  { 23, 1 },
  { 23, 1 },
  { 25, 1 },
  { 25, 3 },
  { 26, 2 },
  { 24, 2 },
  { 24, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 31, 3 },
  { 31, 5 },
  { 28, 3 },
  { 28, 2 },
  { 32, 1 },
  { 32, 3 },
  { 29, 3 },
  { 29, 4 },
  { 33, 3 },
  { 33, 5 },
  { 30, 1 },
  { 30, 3 },
  { 34, 1 },
  { 34, 2 },
  { 35, 3 },
  { 35, 4 },
  { 36, 1 },
  { 36, 1 },
  { 36, 1 },
  { 36, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* start ::= nodeList */
{
    for (std::list<ValueInBlock>::iterator iter = yymsp[0].minor.yy35->begin();
         iter != yymsp[0].minor.yy35->end();
         ++iter)
    {
        pState->m_pRoot->appendValue(iter->m_name, iter->m_pValue);
    }
    delete yymsp[0].minor.yy35;
}
        break;
      case 1: /* nodeList ::= nodeList node */
{
    yygotominor.yy35 = yymsp[-1].minor.yy35;
    yygotominor.yy35->push_back(*yymsp[0].minor.yy26);
    delete yymsp[0].minor.yy26;
}
        break;
      case 2: /* nodeList ::= */
{
    yygotominor.yy35 = new std::list<ValueInBlock>();
}
        break;
      case 3: /* node ::= nodeName ASSIGN nodeValue SEMICOLON */
{
    yygotominor.yy26 = new ValueInBlock(*yymsp[-1].minor.yy36, yymsp[-3].minor.yy42->textValue(), false);
    delete yymsp[-1].minor.yy36;
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
        break;
      case 4: /* node ::= INCLUDE STRING SEMICOLON */
{
    yygotominor.yy26 = new ValueInBlock(Value::Ptr(new Value()), yymsp[-1].minor.yy0->textValue(), true);
    yygotominor.yy26->m_pValue->typeName().push_back("include");
  yy_destructor(yypParser,3,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
        break;
      case 5: /* nodeName ::= STRING */
      case 6: /* nodeName ::= IDENTIFIER */ yytestcase(yyruleno==6);
{
    yygotominor.yy42 = yymsp[0].minor.yy0;
}
        break;
      case 7: /* typeSequence ::= nodeName */
{
    yygotominor.yy2 = new TypeName();
    yygotominor.yy2->push_back(yymsp[0].minor.yy42->textValue());
}
        break;
      case 8: /* typeSequence ::= typeSequence DOT nodeName */
{
    yygotominor.yy2 = yymsp[-2].minor.yy2;
    yygotominor.yy2->push_back(yymsp[0].minor.yy42->textValue());
  yy_destructor(yypParser,6,&yymsp[-1].minor);
}
        break;
      case 9: /* type ::= AT typeSequence */
{
    yygotominor.yy2 = yymsp[0].minor.yy2;
  yy_destructor(yypParser,7,&yymsp[-1].minor);
}
        break;
      case 10: /* nodeValue ::= type value */
{
    yygotominor.yy36 = yymsp[0].minor.yy36;
    (*yygotominor.yy36)->setTypeName(*yymsp[-1].minor.yy2);
    delete yymsp[-1].minor.yy2;
}
        break;
      case 11: /* nodeValue ::= value */
      case 12: /* value ::= array */ yytestcase(yyruleno==12);
      case 15: /* value ::= block */ yytestcase(yyruleno==15);
{
    yygotominor.yy36 = yymsp[0].minor.yy36;
}
        break;
      case 13: /* value ::= macro */
      case 36: /* subscriptValue ::= macro */ yytestcase(yyruleno==36);
{
    yygotominor.yy36 = new Value::Ptr(new Value(*yymsp[0].minor.yy9));
    delete yymsp[0].minor.yy9;
    (*yygotominor.yy36)->setLine(pState->m_currentLine);
    (*yygotominor.yy36)->setPos(pState->m_currentPosition);
}
        break;
      case 14: /* value ::= reference */
      case 37: /* subscriptValue ::= reference */ yytestcase(yyruleno==37);
{
    yygotominor.yy36 = new Value::Ptr(new Value(*yymsp[0].minor.yy38));
    delete yymsp[0].minor.yy38;
    (*yygotominor.yy36)->setLine(pState->m_currentLine);
    (*yygotominor.yy36)->setPos(pState->m_currentPosition);
}
        break;
      case 16: /* value ::= INTEGER */
      case 38: /* subscriptValue ::= INTEGER */ yytestcase(yyruleno==38);
{
    yygotominor.yy36 = new Value::Ptr(new Value(yymsp[0].minor.yy0->integerValue()));
    (*yygotominor.yy36)->setLine(yymsp[0].minor.yy0->line());
    (*yygotominor.yy36)->setPos(yymsp[0].minor.yy0->pos());
}
        break;
      case 17: /* value ::= FLOAT */
{
    yygotominor.yy36 = new Value::Ptr(new Value(yymsp[0].minor.yy0->floatValue()));
    (*yygotominor.yy36)->setLine(yymsp[0].minor.yy0->line());
    (*yygotominor.yy36)->setPos(yymsp[0].minor.yy0->pos());
}
        break;
      case 18: /* value ::= STRING */
      case 39: /* subscriptValue ::= STRING */ yytestcase(yyruleno==39);
{
    yygotominor.yy36 = new Value::Ptr(new Value(yymsp[0].minor.yy0->textValue()));
    (*yygotominor.yy36)->setLine(yymsp[0].minor.yy0->line());
    (*yygotominor.yy36)->setPos(yymsp[0].minor.yy0->pos());
}
        break;
      case 19: /* value ::= BOOLEAN */
{
    yygotominor.yy36 = new Value::Ptr(new Value(yymsp[0].minor.yy0->booleanValue()));
    (*yygotominor.yy36)->setLine(yymsp[0].minor.yy0->line());
    (*yygotominor.yy36)->setPos(yymsp[0].minor.yy0->pos());
}
        break;
      case 20: /* block ::= LEFTCURLYBRACKET nodeList RIGHTCURLYBRACKET */
{
    std::vector<std::string> valueNames;
    ValueArray values;
    for (std::list<ValueInBlock>::iterator iter = yymsp[-1].minor.yy35->begin();
         iter != yymsp[-1].minor.yy35->end();
         ++iter)
    {
        valueNames.push_back(iter->m_name);
        values.push_back(iter->m_pValue);
    }
    yygotominor.yy36 = new Value::Ptr(new Value(valueNames, values));
    delete yymsp[-1].minor.yy35;
    (*yygotominor.yy36)->setLine(yymsp[-2].minor.yy0->line());
    (*yygotominor.yy36)->setPos(yymsp[-2].minor.yy0->pos());
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
        break;
      case 21: /* block ::= COLON reference LEFTCURLYBRACKET nodeList RIGHTCURLYBRACKET */
{
    std::vector<std::string> valueNames;
    ValueArray values;
    for (std::list<ValueInBlock>::iterator iter = yymsp[-1].minor.yy35->begin();
         iter != yymsp[-1].minor.yy35->end();
         ++iter)
    {
        valueNames.push_back(iter->m_name);
        values.push_back(iter->m_pValue);
    }
    yygotominor.yy36 = new Value::Ptr(new Value(valueNames, values));
    delete yymsp[-1].minor.yy35;
    Value::Ptr pRef(new Value(yymsp[-3].minor.yy38));
    delete yymsp[-3].minor.yy38;
    (*yygotominor.yy36)->setLine(yymsp[-2].minor.yy0->line());
    (*yygotominor.yy36)->setPos(yymsp[-2].minor.yy0->pos());
    (*yygotominor.yy36)->setInheritedBlock(pRef);
  yy_destructor(yypParser,13,&yymsp[-4].minor);
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
        break;
      case 22: /* array ::= LEFTSQUAREBRACKET valueList RIGHTSQUAREBRACKET */
{
    ValueArray arrayValues;
    for (std::list<Value::Ptr>::iterator iter = yymsp[-1].minor.yy10->begin();
         iter != yymsp[-1].minor.yy10->end();
         ++iter)
    {
        arrayValues.push_back(*iter);
    }
    yygotominor.yy36 = new Value::Ptr(new Value(arrayValues));
    delete yymsp[-1].minor.yy10;
    (*yygotominor.yy36)->setLine(yymsp[-2].minor.yy0->line());
    (*yygotominor.yy36)->setPos(yymsp[-2].minor.yy0->pos());
  yy_destructor(yypParser,15,&yymsp[0].minor);
}
        break;
      case 23: /* array ::= LEFTSQUAREBRACKET RIGHTSQUAREBRACKET */
{
    ValueArray emptyArrayValues;
    yygotominor.yy36 = new Value::Ptr(new Value(emptyArrayValues));
    (*yygotominor.yy36)->setLine(yymsp[-1].minor.yy0->line());
    (*yygotominor.yy36)->setPos(yymsp[-1].minor.yy0->pos());
  yy_destructor(yypParser,15,&yymsp[0].minor);
}
        break;
      case 24: /* valueList ::= nodeValue */
{
    yygotominor.yy10 = new std::list<Value::Ptr>();
    yygotominor.yy10->push_back(*yymsp[0].minor.yy36);
    delete yymsp[0].minor.yy36;
}
        break;
      case 25: /* valueList ::= valueList COMMA nodeValue */
{
    yygotominor.yy10 = yymsp[-2].minor.yy10;
    yygotominor.yy10->push_back(*yymsp[0].minor.yy36);
    delete yymsp[0].minor.yy36;
  yy_destructor(yypParser,16,&yymsp[-1].minor);
}
        break;
      case 26: /* macro ::= IDENTIFIER LEFTPAREN RIGHTPAREN */
{
    yygotominor.yy9 = new MacroInvocation::Ptr(new MacroInvocation());
    (*yygotominor.yy9)->setName(yymsp[-2].minor.yy0->textValue());
  yy_destructor(yypParser,17,&yymsp[-1].minor);
  yy_destructor(yypParser,18,&yymsp[0].minor);
}
        break;
      case 27: /* macro ::= IDENTIFIER LEFTPAREN keywordArgumentList RIGHTPAREN */
{
    yygotominor.yy9 = yymsp[-1].minor.yy9;
    (*yygotominor.yy9)->setName(yymsp[-3].minor.yy0->textValue());
  yy_destructor(yypParser,17,&yymsp[-2].minor);
  yy_destructor(yypParser,18,&yymsp[0].minor);
}
        break;
      case 28: /* keywordArgumentList ::= IDENTIFIER COLON nodeValue */
{
    yygotominor.yy9 = new MacroInvocation::Ptr(new MacroInvocation());
    (*yygotominor.yy9)->arguments().insert(std::pair<std::string, Value::Ptr>(yymsp[-2].minor.yy0->textValue(), *yymsp[0].minor.yy36));
    delete yymsp[0].minor.yy36;
  yy_destructor(yypParser,13,&yymsp[-1].minor);
}
        break;
      case 29: /* keywordArgumentList ::= keywordArgumentList COMMA IDENTIFIER COLON nodeValue */
{
    yygotominor.yy9 = yymsp[-4].minor.yy9;
    (*yygotominor.yy9)->arguments().insert(std::pair<std::string, Value::Ptr>(yymsp[-2].minor.yy0->textValue(), *yymsp[0].minor.yy36));
    delete yymsp[0].minor.yy36;
  yy_destructor(yypParser,16,&yymsp[-3].minor);
  yy_destructor(yypParser,13,&yymsp[-1].minor);
}
        break;
      case 30: /* reference ::= complexIdentifier */
{
    yygotominor.yy38 = new Reference::Ptr(new Reference());
    (*yygotominor.yy38)->parts() = *yymsp[0].minor.yy14;
    delete yymsp[0].minor.yy14;
}
        break;
      case 31: /* reference ::= reference DOT complexIdentifier */
{
    yygotominor.yy38 = yymsp[-2].minor.yy38;
    (*yygotominor.yy38)->parts().splice((*yygotominor.yy38)->parts().end(), *yymsp[0].minor.yy14);
    delete yymsp[0].minor.yy14;
  yy_destructor(yypParser,6,&yymsp[-1].minor);
}
        break;
      case 32: /* complexIdentifier ::= IDENTIFIER */
{
    yygotominor.yy14 = new std::list<Reference::Part>();
    Reference::Part p;
    p.m_identifier = yymsp[0].minor.yy0->textValue();
    yygotominor.yy14->push_back(p);
}
        break;
      case 33: /* complexIdentifier ::= IDENTIFIER subscriptSequence */
{
    yygotominor.yy14 = yymsp[0].minor.yy14;
    Reference::Part p;
    p.m_identifier = yymsp[-1].minor.yy0->textValue();
    yygotominor.yy14->push_front(p);
}
        break;
      case 34: /* subscriptSequence ::= LEFTSQUAREBRACKET subscriptValue RIGHTSQUAREBRACKET */
{
    yygotominor.yy14 = new std::list<Reference::Part>();
    Reference::Part p;
    p.m_pSubscriptValue = *yymsp[-1].minor.yy36;
    yygotominor.yy14->push_back(p);
    delete yymsp[-1].minor.yy36;
  yy_destructor(yypParser,14,&yymsp[-2].minor);
  yy_destructor(yypParser,15,&yymsp[0].minor);
}
        break;
      case 35: /* subscriptSequence ::= subscriptSequence LEFTSQUAREBRACKET subscriptValue RIGHTSQUAREBRACKET */
{
    yygotominor.yy14 = yymsp[-3].minor.yy14;
    Reference::Part p;
    p.m_pSubscriptValue = *yymsp[-1].minor.yy36;
    yygotominor.yy14->push_back(p);
    delete yymsp[-1].minor.yy36;
  yy_destructor(yypParser,14,&yymsp[-2].minor);
  yy_destructor(yypParser,15,&yymsp[0].minor);
}
        break;
      default:
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  ParseARG_FETCH;
#define TOKEN (yyminor.yy0)

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
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  ParseTOKENTYPE yyminor       /* The value for the token */
  ParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  ParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
