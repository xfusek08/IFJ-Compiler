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

#define kWordNumber 32
#define dTypeNumber 4

//LAnalyzer
typedef struct LAnalyzer *TLAnalyzer;
struct LAnalyzer {
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
void scan_raiseCodeError(ErrType typchyby)
{
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
  //Delete line comment
  if(isLine == true)
  {
    get_line();
  }
  //Looking for ending characters of multi-line comment
  else
  {
    int charCounter = 0;
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

//Function for hashing string
void hash_string(char *string, char *hashString)
{
  int value = 0;
  int exp = 1;
  int i = 0;
  while(string[i] != '\0')
  {
    value += exp * string[i];
    exp *= 10;
    i++;
  }
  sprintf(hashString, "sh@%d", value);
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
  char *str = mmng_safeMalloc(sizeof(char) * CHUNK * Scanner->lineSize);
  //ToLower
  int k = 0;
  while(tokenID[k] != '\0')
  {
    str[k] = tolower(tokenID[k]);
    k++;
  }
  str[k] = '\0';
  //Compare keyWords
  while(i < kWordNumber)
  {
    if(strcmp(str, wArray[i]) == 0)
    {
      tokenType = i + 20;
    }
    i++;
  }
  mmng_safeFree(str);
  return tokenType;
}

//Comparing given string with dataTypes
DataType isDataType(char *tokenID)
{
  //Array of keyWords
  char *sArray[] = {"integer", "double", "string", "boolean"}; //4 dTypeNumber
  int i = 0;
  DataType dType = dtUnspecified;
  char *str = mmng_safeMalloc(sizeof(char) * CHUNK * Scanner->lineSize);
  //ToLower
  int k = 0;
  while(tokenID[k] != '\0')
  {
    str[k] = tolower(tokenID[k]);
    k++;
  }
  str[k] = '\0';
  //Compare dataType
  while(i < dTypeNumber)
  {
    if(strcmp(str, sArray[i]) == 0) //maybe as long as != Until
    {
      dType = i + 1;
    }
    i++;
  }
  mmng_safeFree(str);
  return dType;
}

bool isEndChar()
{
  return Scanner->line[Scanner->position - 1] == '+' || Scanner->line[Scanner->position - 1] == '-'
  || isspace(Scanner->line[Scanner->position - 1]) || Scanner->line[Scanner->position - 1] == '*'
  || Scanner->line[Scanner->position - 1] == '/' || Scanner->line[Scanner->position - 1] == '\\'
  || Scanner->line[Scanner->position - 1] == '=' || Scanner->line[Scanner->position - 1] == EOF
  || Scanner->line[Scanner->position - 1] == 33 || Scanner->line[Scanner->position - 1] == '('
  || Scanner->line[Scanner->position - 1] == ')';
}


//Function that return next token
SToken scan_GetNextToken()
{
  char *tokenID = mmng_safeMalloc(sizeof(char) * CHUNK * Scanner->lineSize);
  TSymbol symbol = NULL;
  SymbolType type = symtUnknown;
  DataType dataType = dtUnspecified;
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
    switch(tokenID[position++] = Scanner->line[Scanner->position++])
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
          scan_raiseCodeError(lexicalErr);
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
            switch(state)
            {
              case 1:
              tokenID[position++] = Scanner->line[Scanner->position++];
                if((tokenID[position - 1]) == '\"'  || (tokenID[position - 1]) == EOF)
                {
                  state = 5;
                  position--;
                }
                else if((tokenID[position - 1]) == '\\')
                {
                  state = 2;
                }
                else if(isprint(tokenID[position - 1]) && !isspace(tokenID[position - 1])
                && tokenID[position - 1] != '#')
                {
                  state = 1;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
                }
                break;
              case 2:
                tokenID[position++] = Scanner->line[Scanner->position++];
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
                  scan_raiseCodeError(lexicalErr);
                }
                break;
              case 3:
                tokenID[position++] = Scanner->line[Scanner->position++];
                if(isdigit(tokenID[position - 1]))
                {
                  state = 4;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
                }
                break;
              case 4:
                tokenID[position++] = Scanner->line[Scanner->position++];
                if(isdigit(tokenID[position - 1]))
                {
                  state = 1;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
                }
                break;
            }
          }
          tokenType = ident;
          type = symtConstant;
          dataType = dtString;
          tokenID[position] = '\0';
          if(position < 10)
          {
            hasStr = mmng_safeMalloc(sizeof(char) * position * 3 + 4);//asci az 127(3 znaky) za znak + sh@(3 znaky) + '\0'
            hash_string(tokenID, hasStr);
          }
          else
          {
            hasStr = mmng_safeMalloc(sizeof(char) * floor(log10(abs(Scanner->alocStr))) + 4); //s@'number'\0
            sprintf(hasStr, "s@%d", Scanner->alocStr);
            Scanner->alocStr++;
          }
          stringVal = util_StrHardCopy(tokenID);
        }
        else if(Scanner->line[Scanner->position] == '=')
        {
          Scanner->position++;
          tokenType = opNotEq;
        }
        else
        {
          scan_raiseCodeError(lexicalErr);
        }
        break;
      //End of line
      case '\n':
        tokenType = eol;
        allowed = true;
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
        if((Scanner->line[Scanner->position - 1] > 64 && Scanner->line[Scanner->position - 1] < 91) //value of a-z
        || (Scanner->line[Scanner->position - 1] > 96 && Scanner->line[Scanner->position - 1] < 123)) //value of A-Z
        {
          state = 0;
          tokenType = ident;
          type = symtUnknown;
          while(state != 5)
          {
            tokenID[position++] = Scanner->line[Scanner->position++];
            switch(state)
            {
              case 0:
                if(Scanner->line[Scanner->position - 1] == '@')
                {
                  state = 1;
                }
                else if((Scanner->line[Scanner->position - 1] > 64 && Scanner->line[Scanner->position - 1] < 91) //value of a-z
                || (Scanner->line[Scanner->position - 1] > 96 && Scanner->line[Scanner->position - 1] < 123) //value of A-Z
                || ((Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)) //value of 0-9
                || Scanner->line[Scanner->position - 1] == '_')
                {
                  state = 0;
                }
                else if(isEndChar())
                {
                  position--;
                  Scanner->position--;
                  state = 5;
                  tokenID[position] = '\0';
                  tokenType = isKeyWord(tokenID);
                  dataType = isDataType(tokenID);
                  if(tokenType == kwTrue || tokenType == kwFalse)
                  {
                    type = symtConstant;
                    dataType = dtBool;
                    if(tokenType == kwTrue)
                    {
                      boolVal = true;
                    }
                    else
                    {
                      boolVal = false;
                    }
                  }
                  else if(dataType != dtUnspecified)
                  {
                    tokenType = dataType;
                  }
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
                }
                break;
              case 1:
                if((Scanner->line[Scanner->position - 1] > 64 && Scanner->line[Scanner->position - 1] < 91) //value of a-z
                || (Scanner->line[Scanner->position - 1] > 96 && Scanner->line[Scanner->position - 1] < 123) //value of A-Z)
                || Scanner->line[Scanner->position - 1] == '\\')
                {
                  state = 4;
                }
                else if(((Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)) //value of 0-9
                || Scanner->line[Scanner->position - 1] == '+' || Scanner->line[Scanner->position - 1] == '-')
                {
                  state = 2;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
                }
                break;
              case 2:
                if(Scanner->line[Scanner->position - 1] == '.') //value of A-Z)
                {
                  state = 3;
                }
                else if(Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58) //value of 0-9
                {
                  state = 2;
                }
                else if(isEndChar())
                {
                  position--;
                  Scanner->position--;
                  state = 5;
                  tokenID[position] = '\0';
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
                }
                break;
              case 3:
                if(Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58) //value of 0-9
                {
                  state = 3;
                }
                else if(Scanner->line[Scanner->position - 1] == '+' || Scanner->line[Scanner->position - 1] == '-'
                || isspace(Scanner->line[Scanner->position - 1]) || Scanner->line[Scanner->position - 1] == '*'
                || Scanner->line[Scanner->position - 1] == '/' || Scanner->line[Scanner->position - 1] == '\\'
                || Scanner->line[Scanner->position - 1] == '=' || Scanner->line[Scanner->position - 1] == EOF
                || Scanner->line[Scanner->position - 1] == 33) //!
                {
                  position--;
                  state = 5;
                  tokenID[position] = '\0';
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
                }
                break;
              case 4:
                if((Scanner->line[Scanner->position - 1] > 64 && Scanner->line[Scanner->position - 1] < 91) //value of a-z
                || (Scanner->line[Scanner->position - 1] > 96 && Scanner->line[Scanner->position - 1] < 123) //value of A-Z
                || ((Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)) //value of 0-9
                || Scanner->line[Scanner->position - 1] == '_' || Scanner->line[Scanner->position - 1] == '\\')
                {
                  state = 4;
                }
                else if(isEndChar() && Scanner->line[Scanner->position - 1] != '\\')
                {
                  position--;
                  Scanner->position--;
                  state = 5;
                  tokenID[position] = '\0';
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
                }
                break;
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
                  dataType = dtInt;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
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
                  dataType = dtFloat;
                }
                else
                {
                  scan_raiseCodeError(lexicalErr);
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
                  scan_raiseCodeError(lexicalErr);
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
          scan_raiseCodeError(lexicalErr);
        }
    }
  }
  //Filing returning token with values
  if(tokenType == ident || tokenType == kwTrue || tokenType == kwFalse)
  {
    if(type == symtConstant && dataType == dtString && hasStr != NULL)
    {
      symbol = symbt_findOrInsertSymb(hasStr);
      mmng_safeFree(hasStr);
    }
    else
    {
      symbol = symbt_findOrInsertSymb(tokenID);
    }
    symbol->type = type;
    symbol->dataType = dataType;
    if(dataType == dtInt)
    {
      symbol->data.intVal = intVal;
    }
    else if(dataType == dtFloat)
    {
      symbol->data.doubleVal = doubleVal;
    }
    else if(dataType == dtString)
    {
      symbol->data.stringVal = stringVal;
    }
    else if(dataType == dtBool)
    {
      symbol->data.boolVal = boolVal;
    }
  }
  SToken token;
  token.dataType = dataType;
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
    mmng_safeFree(Scanner);
  }
}