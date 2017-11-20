/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Scanner.c
 * \brief   Lexical analyzer
 * \author  Jaromír Franěk (xfrane16)
 * \date    18.11.2017 - Jaromír Franěk
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "Scanner.h"
#include "symtable.h"
#include "MMng.h"

//LAnalyzer
typedef struct LAnalyzer *TLAnalyzer;
struct LAnalyzer {
  int curentLine;
  int position;
  int lineSize; //in CHUNKS
  char *line; 
};

typedef union {
  bool intVal;      
  bool doubleVal;  
  bool stringVal;   
  //bool boolVal;     //not sure if needed 
  //bool SFuncData;       
} DataSwitch;

// global internal instance of lexical analyzer
TLAnalyzer Scanner;

//Lexical analyzer constructor
TLAnalyzer Scanner_create()
{
  TLAnalyzer newScanner = (TLAnalyzer)mmng_safeMalloc(sizeof(struct LAnalyzer));

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
/*
void scan_raiseCodeError(ErrorType typchyby)
{
  //TODO
}
*/

//Function for alocating line for scanner
void get_line()
{
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
    Scanner->curentLine++;
    get_line();
  }
  //Looking for ending characters of multi-line comment  
  else
  {
    int charCounter = 0;
    while(!(Scanner->line[charCounter] == '\'' && Scanner->line[charCounter + 1] == '/') && Scanner->line[charCounter + 1] != '\0')
    {
      if(Scanner->line[charCounter + 1] == '\0')
      {
        Scanner->curentLine++;
        get_line(Scanner);
        charCounter = -1;
      }
      charCounter++;  
    }
    Scanner->position = charCounter + 1;
  }
}

//Comparing given string with keyWords
EGrSymb isKeyWord(char *tokenID)//Change name
{
  //Array of keyWords
  char *sArray[] = {"as", "asc", "declare", "dim", "do", "else", "end", "function", "if", "input", "length", "loop",
  "print", "return", "scope", "subStr", "then", "while", "and", "continue", "elseif", "exit", "false", "for",
  "next", "not", "or", "shared", "static", "true", "to", "until"};
  int arrayLeght = 32;
  int i = 0;
  EGrSymb tokenType = ident;
  //ToLower
  char *str = mmng_safeMalloc(sizeof(char) * CHUNK * Scanner->lineSize);
  int k = 0;
  while(tokenID[k] != '\0')
  {
    str[k] = tolower(tokenID[k]);
    k++;
  }
  str[k] = '\0';
  //Compare
  while(i < arrayLeght)
  {
    if(strcmp(str, sArray[i]) == 0) //maybe as long as != Until
    {
      tokenType = i + 40;   
    }
    i++;
  }
  mmng_safeFree(str);
  return tokenType;
}

//Function that return next token
SToken scan_GetNextToken()
{
  char *tokenID = mmng_safeMalloc(sizeof(char) * CHUNK * Scanner->lineSize);
  TSymbol symbol = NULL;
  SymbolType type = symtUnknown;
  DataType dataType = dtUnspecified;
  EGrSymb tokenType = eol;
  //Data and DataSwitch
  DataSwitch dataSwitch;
  int intVal = 0;
  double doubleVal = 0;
  char *stringVal = NULL;
  //bool boolVal = true;  //not sure if needed
  //helping variables
  int position = 0;
  bool allowed = false;
  bool escNum = false;
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
          //error
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
        while(!(Scanner->line[Scanner->position] == '\"' && Scanner->line[Scanner->position - 1] != '!' 
        && Scanner->line[Scanner->position - 1] != '\\') && Scanner->line[Scanner->position] != EOF)
        {
          tokenID[position++] = Scanner->line[Scanner->position++];
          //Escape sequence
          if(tokenID[position - 1] == '\\')
          {
            switch(Scanner->line[Scanner->position])
            {
              case '\"':
                tokenID[position - 1] = '\"';
                Scanner->position++;
                break;
              case 'n':
                tokenID[position - 1] = '\n';
                Scanner->position++;
                break;
              case 't':
                tokenID[position - 1] = '\t';
                Scanner->position++;
                break;
              case '\\':
                tokenID[position - 1] = '\\';
                Scanner->position++;
                break;
              default:
                if(Scanner->line[Scanner->position] > 47 && Scanner->line[Scanner->position] < 58)
                {
                  escNum = true;
                }
            }
          }
          // \xxx
          if(position > 4 && escNum)
          {
            if(tokenID[position - 3] > 47 && tokenID[position - 3] < 58
            && tokenID[position - 2] > 47 && tokenID[position - 2] < 58
            && tokenID[position - 1] > 47 && tokenID[position - 1] < 58)
            {
              //Getting value of a number after '\\'
              char cNumber = 100 * (tokenID[position - 3] - '0') + 
              10 * (tokenID[position - 2] - '0') +
              (tokenID[position - 1] - '0');
              position -= 4;
              tokenID[position++] = cNumber;
              escNum = false;
            }
          }
        }
        tokenType = ident;
        type = symtConstant;
        stringVal = tokenID;
        dataSwitch.stringVal = true;
        Scanner->position++;
        tokenID[position++] = '\"';
        tokenID[position] = '\0';
        allowed = true;
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
        //value of a-Z
        if((Scanner->line[Scanner->position - 1] > 64 && Scanner->line[Scanner->position - 1] < 91)
        || (Scanner->line[Scanner->position - 1] > 96 && Scanner->line[Scanner->position - 1] < 123))
        {
          while((Scanner->line[Scanner->position] > 64 && Scanner->line[Scanner->position] < 91)
          || (Scanner->line[Scanner->position] > 96 && Scanner->line[Scanner->position] < 123))
          {
            tokenID[position++] = Scanner->line[Scanner->position++];  
          }
          tokenID[position] = '\0';
          allowed = true;
          //Compare
          tokenType = isKeyWord(tokenID);
        }
        //Numbers
        else if(Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)
        {
          tokenType = ident;
          type = symtConstant;
          bool isInt = true;
          while((Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58) 
          || Scanner->line[Scanner->position - 1] == 46)
          {
            tokenID[position++] = Scanner->line[Scanner->position++];
            //Checking for '.'
            if(tokenID[position - 1] == 46)
              isInt = false; 
          }
          tokenID[position] = '\0';
          allowed = true;
          //Getting value
          if(!isInt)
          {
            doubleVal = strtod(tokenID, NULL);
            dataSwitch.doubleVal = true; 
          }
          else
          {
            intVal = strtol(tokenID, NULL, 10);
            dataSwitch.intVal = true;
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
          //scan_raiseCodeError(ErrorType typchyby);//TODO
        }
    }
  }
  //Filing returning token with values
  if(tokenType == ident)
  {
    symbol = symbt_findOrInsertSymb(tokenID);
    symbol->type = type;
    //symbol->data.boolVal = boolVal;  //not sure if needed
    if(dataSwitch.intVal)
    {
      symbol->data.intVal = intVal;
    }
    else if(symbol->data.doubleVal)
    {
      symbol->data.doubleVal = doubleVal;
    }
    else if(symbol->data.stringVal)
    {
      symbol->data.stringVal = stringVal;
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
    mmng_safeFree(Scanner);
}