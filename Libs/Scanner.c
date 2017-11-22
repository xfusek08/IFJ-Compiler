/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Scanner.c
 * \brief   Lexical analyzer
 * \author  Jaromír Franěk (xfrane16)
 * \date    21.11.2017 - Jaromír Franěk
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "Scanner.h"
#include "symtable.h"
#include "MMng.h"

#define KWORDNUMBER 32
#define DTYPENUMBER 4

//LAnalyzer
typedef struct LAnalyzer *TLAnalyzer;
struct LAnalyzer {
  SToken lastToken;
  int alocStr;
  int curentLine;
  int position;
  int lineSize; //in CHUNKS
  char *line;
};

// global internal instance of lexical analyzer
TLAnalyzer Scanner;

//Lexical analyzer constructor
TLAnalyzer Scanner_create()
{
  TLAnalyzer newScanner = (TLAnalyzer)mmng_safeMalloc(sizeof(struct LAnalyzer));

  newScanner->alocStr = 1;
  newScanner->curentLine = 0;
  newScanner->position = 0;
  newScanner->lineSize = 1;
  newScanner->line = mmng_safeMalloc(sizeof(char) * CHUNK * newScanner->lineSize);
  newScanner->line[0] = '\0';
  newScanner->lastToken.type = eol;
  
  return newScanner;
}

//Initialization of scanner
void Scanner_init()
{
  if (Scanner != NULL)
    apperr_runtimeError("Scanner is already initialized.");
  Scanner = Scanner_create();
}

//Error function
void scan_raiseCodeError(ErrType typchyby, char *message)
{
  (void)*message;
  apperr_codeError(typchyby, Scanner->curentLine, Scanner->position, Scanner->line);
}


//Function for alocating line for scanner
void get_line()
{
  Scanner->curentLine++;
  int charCounter = 0;
  while(((Scanner->line[charCounter] = getchar()) != '\n'))
  {
    if(charCounter == (Scanner->lineSize * CHUNK - 2))
    {
      Scanner->lineSize++;
      Scanner->line = mmng_safeRealloc(Scanner->line, sizeof(char) * Scanner->lineSize * CHUNK);//Mozna chyba
    }
    if(Scanner->line[charCounter] == EOF)
      break;
    charCounter++;
  }
  Scanner->line[++charCounter] = '\0';
  Scanner->position = 0;
}

//Function for deleting comments
void delete_comment(bool isLine)
{
  int charCounter = 0;
  //Delete line comment
  if(isLine == true)
  {
    if(Scanner->lastToken.type != eol)
    {
      while(Scanner->line[charCounter] != '\0' && Scanner->line[charCounter] != EOF && Scanner->line[charCounter] != '\n')
      {
        charCounter++;  
      }
      Scanner->position = charCounter;      
    }
    else
    {
      get_line();
    }
  }
  //Looking for ending characters of multi-line comment
  else
  {
    while(!(Scanner->line[charCounter] == '\'' && Scanner->line[charCounter + 1] == '/') && Scanner->line[charCounter + 1] != EOF)
    {
      if(Scanner->line[charCounter + 1] == '\0')
      {
        get_line(Scanner);
        charCounter = -1;
      }
      charCounter++;
    }
    Scanner->position = charCounter + 2;
  }
}

//Comparing given string with keyWords
EGrSymb isKeyWord(char *tokenID)
{
  //Array of keyWords
  char *wArray[] = {"not", "and", "or", "as", "asc", "declare", "dim", "do", "else", "end", "function", "if", "input", "length", "loop",
  "print", "return", "scope", "subStr", "then", "while", "continue", "elseif", "exit", "false", "for",
  "next", "shared", "static", "true", "to", "until"};  //32 kWordNumber
  int i = 0;
  EGrSymb tokenType = ident;
  //Compare keyWords
  while(i < KWORDNUMBER)
  {
    if(strcmp(tokenID, wArray[i]) == 0)
    {
      tokenType = i + 20;
    }
    i++;
  }
  return tokenType;
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
    {
      dType = i + 1;
    }
    i++;
  }
  return dType;
}

bool isEndChar()
{
  return Scanner->line[Scanner->position - 1] == '+' || Scanner->line[Scanner->position - 1] == '-'
  || isspace(Scanner->line[Scanner->position - 1]) || Scanner->line[Scanner->position - 1] == '*'
  || Scanner->line[Scanner->position - 1] == '/' || Scanner->line[Scanner->position - 1] == '\\'
  || Scanner->line[Scanner->position - 1] == '=' || Scanner->line[Scanner->position - 1] == EOF
  || Scanner->line[Scanner->position - 1] == 33 || Scanner->line[Scanner->position - 1] == '('
  || Scanner->line[Scanner->position - 1] == ')' || Scanner->line[Scanner->position - 1] == ';';
}


//Function that return next token
SToken scan_GetNextToken()
{
  char *tokenID = mmng_safeMalloc(sizeof(char) * CHUNK * Scanner->lineSize);
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
    //Finding type of token
    switch(tokenID[position++] = tolower(Scanner->line[Scanner->position++]))
    {
      case '=':
        allowed = true;
        tokenType = opEq;
        break;
      case '+':
        tokenType = opPlus;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opPlusEq;
          Scanner->position++;
        }
        allowed = true;
        break;
      case '-':
        tokenType = opMns;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opMnsEq;
          Scanner->position++;
        }
        allowed = true;
        break;
      case '*':
        tokenType = opMul;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opMulEq;
          Scanner->position++;
        }
        allowed = true;
        break;
      case '/':
        tokenType = opDiv;
        if(Scanner->line[Scanner->position] == '\'')
        {
          position--;
          delete_comment(false);
          allowed = false;
          tokenType = eol;
        }
        else if(Scanner->line[Scanner->position] == '=')
        {
          tokenID[position++] = Scanner->line[Scanner->position++];
          tokenID[position] = '\0';
          allowed = true;
          tokenType = opDivEq;
        }
        break;
      case '\\':
        tokenType = opDivFlt;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opDivFltEq;
          Scanner->position++;
        }
        allowed = true;
        break;
      case '<':
        tokenType = opLes;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opLessEq;
          Scanner->position++;
        }
        allowed = true;
        break;
      case '>':
        tokenType = opGrt;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opGrtEq;
          Scanner->position++;
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
      case ':':
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = asng;
          Scanner->position++;
        }
        else
        {
          scan_raiseCodeError(lexicalErr, NULL);
        }
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
        if(Scanner->line[Scanner->position] == '\"')
        {
          state = 1;
          tokenID[position] = Scanner->line[Scanner->position++];
          while(state != 5)
          {
            tokenID[position++] = Scanner->line[Scanner->position++];
            switch(state)
            {
              case 1:
                if((tokenID[position - 1]) == '\"'  || (tokenID[position - 1]) == EOF)
                {
                  state = 5;
                  position--;
                }
                else if((tokenID[position - 1]) == '\\')
                {
                  state = 2;
                }
                else if(/*isprint(tokenID[position - 1]) &&*/ (!isspace(tokenID[position - 1] || tokenID[position - 1] == ' '))
                && tokenID[position - 1] != '#')
                {
                  state = 1;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr, NULL);
                }
                break;
              case 2:
                if(isdigit(tokenID[position - 1]))
                {
                  state = 3;
                }
                else if(tokenID[position - 1] == 'n' || tokenID[position - 1] == 't' ||
                tokenID[position - 1] == '\"' || tokenID[position - 1] == '\\')
                {
                  state = 1;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr, NULL);
                }
                break;
              case 3:
                if(isdigit(tokenID[position - 1]))
                {
                  state = 4;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr, NULL);
                }
                break;
              case 4:
                if(isdigit(tokenID[position - 1]))
                {
                  state = 1;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr, NULL);
                }
                break;
            }
          }
          tokenType = ident;
          type = symtConstant;
          dType = dtString;
          tokenID[position] = '\0';
          hasStr = mmng_safeMalloc(sizeof(char) * floor(log10(abs(Scanner->alocStr))) + 4); //s@'number'\0
          sprintf(hasStr, "s@%d", Scanner->alocStr);
          Scanner->alocStr++;
          stringVal = util_StrHardCopy(tokenID);
        }
        else if(Scanner->line[Scanner->position] == '=')
        {
          Scanner->position++;
          tokenType = opNotEq;
        }
        else
        {
          scan_raiseCodeError(lexicalErr, NULL);
        }
        break;
      //End of line
      case '\n':
        tokenType = eol;
        allowed = true;
        if(Scanner->lastToken.type == eol)
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
          type = symtUnknown;
          while(state != 1)
          {
            tokenID[position++] = tolower(Scanner->line[Scanner->position++]);
            if((tokenID[position - 1] > 96 && tokenID[position - 1] < 123) //value of a-z
            || (tokenID[position - 1] > 47 && tokenID[position - 1] < 58) //value of 0-9
            || tokenID[position - 1] == '_')
            {
              state = 0;
            }
            else if(isEndChar())
            {
              position--;
              Scanner->position--;
              state = 1;
              tokenID[position] = '\0';
              tokenType = isKeyWord(tokenID);
              dType = isDataType(tokenID);
              if(tokenType == kwTrue || tokenType == kwFalse)
              {
                type = symtConstant;
                dType = dtBool;
                if(tokenType == kwTrue)
                {
                  boolVal = true;
                }
                else
                {
                  boolVal = false;
                }
              }
              else if(dType != dtUnspecified)
              {
                tokenType = dataType;
              }
            }
            else
            {
              scan_raiseCodeError(lexicalErr, NULL);
            }
            allowed = true;
          }
        }
        //Numbers
        else if(Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58) // 0-9
        {
          state = 0;
          tokenType = ident;
          type = symtConstant;
          while(state != 3)
          {
            tokenID[position++] = Scanner->line[Scanner->position++];
            switch(state)
            {
              case 0:
                if(Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)
                {
                  state = 0;
                }
                else if(Scanner->line[Scanner->position - 1] == '.')
                {
                  state = 1;
                }
                else if(Scanner->line[Scanner->position - 1] == 'e' || Scanner->line[Scanner->position - 1] == 'E')
                {
                  state = 2;
                }
                else if(isEndChar())
                {
                  state = 3;
                  Scanner->position--;
                  tokenID[--position] = '\0';
                  intVal = strtol(tokenID, NULL, 10);
                  dType = dtInt;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr, NULL);
                }
                break;
              case 1:
                if(Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)
                {
                  state = 1;
                }
                else if(isEndChar())
                {
                  state = 3;
                  Scanner->position--;
                  tokenID[--position] = '\0';
                  doubleVal = strtod(tokenID, NULL);
                  dType = dtFloat;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr, NULL);
                }
                break;
              case 2:
                if((Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)
                || Scanner->line[Scanner->position - 1] == '+' || Scanner->line[Scanner->position - 1] == '-')
                {
                  state = 1;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr, NULL);
                }
                break;
            }
            allowed = true;
          }
        }
        //Space,...
        else if(isspace(Scanner->line[Scanner->position - 1]))
        {
          position--;
          allowed = false;
        }
        //Error
        else
        {
          scan_raiseCodeError(lexicalErr, NULL);
        }
    }
  }
  //Filing returning token with values
  Scanner->lastToken.type = tokenType;
  if(tokenType == ident || tokenType == kwTrue || tokenType == kwFalse)
  {
    if(type == symtConstant && dType == dtString && hasStr != NULL)
    {
      symbol = symbt_findOrInsertSymb(hasStr);
      mmng_safeFree(hasStr);
    }
    else
    {
      symbol = symbt_findOrInsertSymb(tokenID);
    }
    symbol->type = type;
    symbol->dataType = dType;
    if(dType == dtInt)
    {
      symbol->data.intVal = intVal;
    }
    else if(dType == dtFloat)
    {
      symbol->data.doubleVal = doubleVal;
    }
    else if(dType == dtString)
    {
      symbol->data.stringVal = stringVal;
    }
    else if(dType == dtBool)
    {
      symbol->data.boolVal = boolVal;
    }
  }
  SToken token;
  token.dataType = dType;
  token.type = tokenType;
  token.symbol = symbol;
  mmng_safeFree(tokenID);
  return token;
}

//destructor of LAnalyzer
void Scanner_destroy()
{
  if (Scanner != NULL)
  {
    mmng_safeFree(Scanner->line);
    mmng_safeFree(Scanner);
  }
}