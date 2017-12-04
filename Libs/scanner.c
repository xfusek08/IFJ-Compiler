/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    scanner.c
 * \brief   Lexical analyzer
 * \author  Jaromír Franěk (xfrane16)
 * \date    22.11.2017 - Jaromír Franěk
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "scanner.h"
#include "symtable.h"
#include "MMng.h"
#include "appErr.h"

#define KWORDNUMBER 30
#define DTYPENUMBER 4

//LAnalyzer
typedef struct LAnalyzer *TLAnalyzer;
struct LAnalyzer {
  SToken lastToken;
  int alocStr;
  int curentLine;
  int prevPosition;
  int position;
  int lineSize; //in CHUNKS
  char *line;
};

// global internal instance of lexical analyzer
TLAnalyzer GLBScanner;

//Lexical analyzer constructor
TLAnalyzer TLAnalyzer_create()
{
  TLAnalyzer newScanner = (TLAnalyzer)mmng_safeMalloc(sizeof(struct LAnalyzer));

  newScanner->alocStr = 1;
  newScanner->curentLine = 0;
  newScanner->prevPosition = 0;
  newScanner->position = 0;
  newScanner->lineSize = 1;
  newScanner->line = mmng_safeMalloc(sizeof(char) * CHUNK * newScanner->lineSize);
  newScanner->line[0] = '\0';
  newScanner->lastToken.type = eol;

  return newScanner;
}

//Initialization of scanner
void scan_init()
{
  if (GLBScanner != NULL)
    apperr_runtimeError("Scanner is already initialized.");
  GLBScanner = TLAnalyzer_create();
}

//Error function
void scan_raiseCodeError(ErrType typchyby, char *message, SToken *token)
{
  apperr_codeError(
    typchyby,
    GLBScanner->curentLine,
    GLBScanner->prevPosition,
    GLBScanner->line,
    message,
    token);
}


//Function for alocating line for scanner
void get_line()
{
  GLBScanner->curentLine++;
  int charCounter = 0;
  while(((GLBScanner->line[charCounter] = getchar()) != '\n'))
  {
    if(charCounter == (GLBScanner->lineSize * CHUNK - 2))
    {
      GLBScanner->lineSize++;
      GLBScanner->line = mmng_safeRealloc(GLBScanner->line, sizeof(char) * GLBScanner->lineSize * CHUNK);
    }
    if(GLBScanner->line[charCounter] == EOF)
      break;
    charCounter++;
  }
  GLBScanner->line[++charCounter] = '\0';
  GLBScanner->position = 0;
}

//Function for deleting comments
void delete_comment(bool isLine)
{

  int charCounter = GLBScanner->position;
  //Delete line comment
  if(isLine == true)
  {
    if(GLBScanner->lastToken.type != eol)
    {
      while(GLBScanner->line[charCounter] != '\0' && GLBScanner->line[charCounter] != EOF && GLBScanner->line[charCounter] != '\n')
        charCounter++;
      GLBScanner->position = charCounter;
    }
    else
    {
      get_line();
    }
  }
  //Looking for ending characters of multi-line comment
  else
  {
    while(!(GLBScanner->line[charCounter] == '\'' && GLBScanner->line[charCounter + 1] == '/'))
    {
      if(GLBScanner->line[charCounter + 1] == '\0')
      {
        get_line(GLBScanner);
        charCounter = -1;
      }
      else if(GLBScanner->line[charCounter + 1] == EOF)
        scan_raiseCodeError(lexicalErr, "Multiline comment not closed.", NULL);
      charCounter++;
    }
    GLBScanner->position = charCounter + 2;
  }
}

//Comparing given string with keyWords
EGrSymb isKeyWord(char *tokenID)
{
  //Array of keyWords
  char *wArray[] = {"not", "and", "or", "as", "declare", "dim", "do", "else", "end", "function", "if", "input", "loop",
  "print", "return", "scope", "then", "while", "continue", "elseif", "exit", "false", "for",
  "next", "shared", "static", "true", "to", "until", "step"};  //30 kWordNumber
  int i = 0;
  //Compare keyWords
  while(i < KWORDNUMBER)
  {
    if(strcmp(tokenID, wArray[i]) == 0)
    {
      EGrSymb tokenType = ident;
      if (i < 3)
        tokenType = i + 21;
      else
        tokenType = i + 25;
      return tokenType;
    }
    i++;
  }
  return ident;
}

//Comparing given string with dataTypes
DataType isDataType(char *tokenID)
{
  //Array of keyWords
  char *sArray[] = {"integer", "double", "string", "boolean"}; //4 dTypeNumber
  int i = 0;
  DataType dType = dtUnspecified;
  //ToLower
  //Compare dataType
  while(i < DTYPENUMBER)
  {
    if(strcmp(tokenID, sArray[i]) == 0) //maybe as long as != Until
      dType = i + 1;
    i++;
  }
  return dType;
}

bool isEndChar(char endChar)
{
  return endChar == '+' || endChar == '-'
  || isspace(endChar) || endChar == '*'
  || endChar == '/' || endChar == '\\'
  || endChar == '=' || endChar == EOF
  || endChar == 33 || endChar == '('
  || endChar == ')' || endChar == ','
  || endChar == ';' || endChar == '\''
  || endChar == '<' || endChar == '>';
}


//Function that return next token
SToken scan_GetNextToken()
{
  char *tokenID = mmng_safeMalloc(sizeof(char) * CHUNK * GLBScanner->lineSize);
  TSymbol symbol = NULL;
  SymbolType type = symtUnknown;
  DataType dType = dtUnspecified;
  EGrSymb tokenType = eol;
  //Data
  int intVal = 0;
  double doubleVal = 0;
  char *stringVal = NULL;
  bool boolVal = true;
  //helping variables
  char *hasStr = NULL;
  int state = 0;
  int position = 0;
  bool allowed = false;
  //Getting next token (retezec)
  while(!allowed)
  {
    GLBScanner->prevPosition = GLBScanner->position + 1;
    //Finding type of token
    switch(tokenID[position++] = tolower(GLBScanner->line[GLBScanner->position++]))
    {
      case '=':
        allowed = true;
        tokenType = opEq;
        break;
      case '+':
        tokenType = opPlus;
        if(GLBScanner->line[GLBScanner->position] == '=')
        {
          tokenType = opPlusEq;
          GLBScanner->position++;
        }
        allowed = true;
        break;
      case '-':
        tokenType = opMns;
        if(GLBScanner->line[GLBScanner->position] == '=')
        {
          tokenType = opMnsEq;
          GLBScanner->position++;
        }
        allowed = true;
        break;
      case '*':
        tokenType = opMul;
        if(GLBScanner->line[GLBScanner->position] == '=')
        {
          tokenType = opMulEq;
          GLBScanner->position++;
        }
        allowed = true;
        break;
      case '/':
        tokenType = opDivFlt;
        allowed = true;
        if(GLBScanner->line[GLBScanner->position] == '\'')
        {
          position--;
          delete_comment(false);
          allowed = false;
          tokenType = eol;
        }
        else if(GLBScanner->line[GLBScanner->position] == '=')
        {
          tokenID[position++] = GLBScanner->line[GLBScanner->position++];
          tokenID[position] = '\0';
          allowed = true;
          tokenType = opDivEq;
        }
        break;
      case '\\':
        tokenType = opDiv;
        if(GLBScanner->line[GLBScanner->position] == '=')
        {
          tokenType = opDivFltEq;
          GLBScanner->position++;
        }
        allowed = true;
        break;
      case '<':
        tokenType = opLes;
        if(GLBScanner->line[GLBScanner->position] == '=')
        {
          tokenType = opLessEq;
          GLBScanner->position++;
        }
        else if(GLBScanner->line[GLBScanner->position] == '>')
        {
          tokenType = opNotEq;
          GLBScanner->position++;
        }
        allowed = true;
        break;
      case '>':
        tokenType = opGrt;
        if(GLBScanner->line[GLBScanner->position] == '=')
        {
          tokenType = opGrtEq;
          GLBScanner->position++;
        }
        allowed = true;
        break;
      case '(':
        tokenType = opLeftBrc;
        allowed = true;
        break;
      case ')':
        tokenType = opRightBrc;
        allowed = true;
        break;
      case ';':
        tokenType = opSemcol;
        allowed = true;
        break;
      case ',':
        tokenType = opComma;
        allowed = true;
        break;
      case '\'':
        position--;
        delete_comment(true);
        allowed = false;
        break;
      //String
      case '!':
        state = 0;
        position--;
        allowed = true;
        if(GLBScanner->line[GLBScanner->position] == '\"')
        {
          state = 1;
          tokenID[position] = GLBScanner->line[GLBScanner->position++];
          while(state != 5)
          {
            tokenID[position++] = GLBScanner->line[GLBScanner->position++];
            if(position > ((CHUNK * GLBScanner->lineSize) - 4))
            {
              GLBScanner->lineSize++;
              tokenID = mmng_safeRealloc(tokenID, sizeof(char) * GLBScanner->lineSize * CHUNK);
            }
            switch(state)
            {
              case 1:
                if((tokenID[position - 1]) == '\"')
                {
                  state = 5;
                  position--;
                }
                else if((tokenID[position - 1]) == '\\')
                  state = 2;
                else if(tokenID[position - 1] > 31 && tokenID[position - 1] != '#')
                {
                  state = 1;
                  if(tokenID[position - 1] == 35 || tokenID[position - 1] == 32)
                  {
                    if(tokenID[position - 1] == 32)
                    {
                      tokenID[position - 1] = 92;
                      tokenID[position] = '0';
                      tokenID[position + 1] = '3';
                      tokenID[position + 2] = '2';
                      position += 3;
                    }
                    else if(tokenID[position - 1] == 35)
                    {
                      tokenID[position - 1] = 92;
                      tokenID[position] = '0';
                      tokenID[position + 1] = '3';
                      tokenID[position + 2] = '5';
                      position += 3;
                    }
                  }
                }
                else
                  scan_raiseCodeError(lexicalErr, "Wrong character inside string constant.", NULL);
                break;
              case 2:
                if(isdigit(tokenID[position - 1]))
                  state = 3;
                else if(tokenID[position - 1] == 'n')
                {
                  tokenID[position - 2] = 92;
                  tokenID[position - 1] = '0';
                  tokenID[position] = '1';
                  tokenID[position + 1] = '0';
                  position += 2;
                  state = 1;
                }
                else if(tokenID[position - 1] == 't')
                {
                  tokenID[position - 2] = 92;
                  tokenID[position - 1] = '1';
                  tokenID[position] = '1';
                  tokenID[position + 1] = '6';
                  position += 2;
                  state = 1;
                }
                else if(tokenID[position - 1] == '\"')
                {
                  tokenID[position - 2] = 92;
                  tokenID[position - 1] = '0';
                  tokenID[position] = '3';
                  tokenID[position + 1] = '4';
                  position += 2;
                  state = 1;
                }
                else if(tokenID[position - 1] == '\\')
                {
                  tokenID[position - 2] = 92;
                  tokenID[position - 1] = '0';
                  tokenID[position] = '9';
                  tokenID[position + 1] = '2';
                  position += 2;
                  state = 1;
                }
                else
                  scan_raiseCodeError(lexicalErr, "Wrong character after \\, maybe you want to write \\n.", NULL);
                break;
              case 3:
                if(isdigit(tokenID[position - 1]))
                  state = 4;
                else
                  scan_raiseCodeError(lexicalErr, "Wrong character after \\, maybe you want to write \\xxx, where x is number.", NULL);
                break;
              case 4:
                if(isdigit(tokenID[position - 1]))
                {
                  state = 1;
                  if(((tokenID[position - 3] - '0') * 100 + (tokenID[position - 2] - '0') * 10) + (tokenID[position - 1] - '0') > 255) //value after of /xxx
                    scan_raiseCodeError(lexicalErr, "Wrong character after \\, write number with values 001-255.", NULL);
                }
                else
                  scan_raiseCodeError(lexicalErr, "Wrong character after \\, maybe you want to write \\xxx, where x is number.", NULL);
                if(tokenID[position - 1] == '0' && tokenID[position - 2] == '0' && tokenID[position - 3] == '0')
                  scan_raiseCodeError(lexicalErr, "Wrong character after \\, write number with values 001-255.", NULL);
                break;
            }
          }
          tokenType = ident;
          type = symtConstant;
          dType = dtString;
          tokenID[position] = '\0';
          hasStr = mmng_safeMalloc(sizeof(char) * floor(log10(abs(GLBScanner->alocStr))) + 4); //s@'number'\0
          sprintf(hasStr, "s@%d", GLBScanner->alocStr);
          GLBScanner->alocStr++;
          stringVal = util_StrHardCopy(tokenID);
        }
        else
          scan_raiseCodeError(lexicalErr, "Wrong character after !, maybe you want to write !\"\".", NULL);
        break;
      //End of line
      case '\n':
        tokenType = eol;
        allowed = true;
        if(GLBScanner->lastToken.type == eol)
        {
          allowed = false;
          position--;
        }
        break;
      //End of File
      case EOF:
        tokenType = eof;
        tokenID[position] = '\0';
        allowed = true;
        break;
      case '\0':
        allowed = false;
        position--;
        get_line();
        break;
      default:
        //Identifires
        if((tokenID[position - 1] > 96 && tokenID[position - 1] < 123) //value of a-z
        || tokenID[position - 1] == '_')
        {
          state = 0;
          tokenType = ident;
          while(state != 1)
          {
            tokenID[position++] = tolower(GLBScanner->line[GLBScanner->position++]);
            if((tokenID[position - 1] > 96 && tokenID[position - 1] < 123) //value of a-z
            || (tokenID[position - 1] > 47 && tokenID[position - 1] < 58) //value of 0-9
            || tokenID[position - 1] == '_')
              state = 0;
            else if(isEndChar(GLBScanner->line[GLBScanner->position - 1]))
            {
              position--;
              GLBScanner->position--;
              state = 1;
              tokenID[position] = '\0';
              tokenType = isKeyWord(tokenID);
              dType = isDataType(tokenID);
              if(tokenType == kwTrue || tokenType == kwFalse)
              {
                type = symtConstant;
                dType = dtBool;
                if(tokenType == kwTrue)
                  boolVal = true;
                else
                  boolVal = false;
                tokenType = ident;
              }
              else if(dType != dtUnspecified)
                tokenType = dataType;
            }
            else
              scan_raiseCodeError(lexicalErr, "Wrong character inside identifier.", NULL);
            allowed = true;
          }
        }
        //Numbers
        else if(GLBScanner->line[GLBScanner->position - 1] > 47 && GLBScanner->line[GLBScanner->position - 1] < 58) // 0-9
        {
          state = 0;
          tokenType = ident;
          type = symtConstant;
          while(state != 4)
          {
            tokenID[position++] = GLBScanner->line[GLBScanner->position++];
            switch(state)
            {
              case 0:
                if(GLBScanner->line[GLBScanner->position - 1] > 47 && GLBScanner->line[GLBScanner->position - 1] < 58)
                  state = 0;
                else if(GLBScanner->line[GLBScanner->position - 1] == '.')
                {
                  state = 1;
                  if(!(GLBScanner->line[GLBScanner->position] > 47 && GLBScanner->line[GLBScanner->position] < 58))
                    scan_raiseCodeError(lexicalErr, "Wrong character after . , you are probably missing number there.", NULL);
                }
                else if(GLBScanner->line[GLBScanner->position - 1] == 'e' || GLBScanner->line[GLBScanner->position - 1] == 'E')
                  state = 2;
                else
                {
                  state = 4;
                  GLBScanner->position--;
                  tokenID[--position] = '\0';
                  intVal = strtol(tokenID, NULL, 10);
                  dType = dtInt;
                }
                break;
              case 1:
                if(GLBScanner->line[GLBScanner->position - 1] > 47 && GLBScanner->line[GLBScanner->position - 1] < 58)
                  state = 1;
                else if(GLBScanner->line[GLBScanner->position - 1] == 'e' || GLBScanner->line[GLBScanner->position - 1] == 'E')
                  state = 2;
                else
                {
                  state = 4;
                  GLBScanner->position--;
                  tokenID[--position] = '\0';
                  doubleVal = strtod(tokenID, NULL);
                  dType = dtFloat;
                }
                break;
              case 2:
                if((GLBScanner->line[GLBScanner->position - 1] > 47 && GLBScanner->line[GLBScanner->position - 1] < 58))
                  state = 3;
                else if(GLBScanner->line[GLBScanner->position - 1] == '+' || GLBScanner->line[GLBScanner->position - 1] == '-')
                {
                  state = 3;
                  if(!(GLBScanner->line[GLBScanner->position] > 47 && GLBScanner->line[GLBScanner->position] < 58))
                    scan_raiseCodeError(lexicalErr, "Wrong character after e , you are probably missing number there.", NULL);
                }
                else
                  scan_raiseCodeError(lexicalErr, "Wrong character after e, alowed are +,-,[0-9].", NULL);
                break;
              case 3:
                if(GLBScanner->line[GLBScanner->position - 1] > 47 && GLBScanner->line[GLBScanner->position - 1] < 58)
                  state = 3;
                else
                {
                  state = 4;
                  GLBScanner->position--;
                  tokenID[--position] = '\0';
                  doubleVal = strtod(tokenID, NULL);
                  dType = dtFloat;
                }
                break;
            }
            allowed = true;
          }
        }
        //Space,...
        else if(isspace(GLBScanner->line[GLBScanner->position - 1]))
        {
          position--;
          allowed = false;
        }
        //Error
        else
          scan_raiseCodeError(lexicalErr, "Unknown character in this context.", NULL);
    }
  }
  //Filing returning token with values
  GLBScanner->lastToken.type = tokenType;
  if(tokenType == ident || tokenType == kwTrue || tokenType == kwFalse)
  {
    if(type == symtConstant && dType == dtString && hasStr != NULL)
    {
      symbol = symbt_findOrInsertSymb(hasStr);
      mmng_safeFree(hasStr);
    }
    else
      symbol = symbt_findOrInsertSymb(tokenID);
    if(type != symtUnknown)
    {
      symbol->type = type;
      symbol->dataType = dType;
    }
    if(dType == dtInt)
      symbol->data.intVal = intVal;
    else if(dType == dtFloat)
      symbol->data.doubleVal = doubleVal;
    else if(dType == dtString)
      symbol->data.stringVal = stringVal;
    else if(dType == dtBool)
      symbol->data.boolVal = boolVal;
  }
  SToken token;
  token.dataType = dType;
  token.type = tokenType;
  token.symbol = symbol;
  mmng_safeFree(tokenID);
  return token;
}

//destructor of LAnalyzer
void scan_destroy()
{
  if (GLBScanner != NULL)
  {
    mmng_safeFree(GLBScanner->line);
    mmng_safeFree(GLBScanner);
  }
}