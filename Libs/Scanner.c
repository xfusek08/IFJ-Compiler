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

// global internal instance of lexical analyzer
TLAnalyzer Scanner;

//Lexical analyzer constructor
TLAnalyzer Scanner_create()
{
  TLAnalyzer newScanner = (TLAnalyzer)mmng_safeMalloc(sizeof(struct LAnalyzer));
  if (newScanner == NULL)
    apperr_runtimeError("Allocation error in Scanner");

  newScanner->curentLine = 0;
  newScanner->position = 0;
  newScanner->lineSize = 1;
  newScanner->line = mmng_safeMalloc(sizeof(char) * CHUNK * newScanner->lineSize);
  if (newScanner->line == NULL)
    apperr_runtimeError("Allocation error in Scanner");

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
    if(charCounter == (Scanner->lineSize * CHUNK - 1))
    {
      Scanner->lineSize++;
      Scanner->line = mmng_safeRealloc(Scanner->line, sizeof(char) * Scanner->lineSize * CHUNK);//Mozna chyba
    }
    if(Scanner->line[charCounter] == EOF)
      break;
    charCounter++;
  }
  Scanner->line[charCounter] = '\0';
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

//Function that return next token
SToken scan_GetNextToken()
{
  char *tokenID = mmng_safeMalloc(sizeof(char) * CHUNK * Scanner->lineSize);
  TSymbol symbol = NULL;
  SymbolType type = symtUnknown;
  DataType dataType = dtUnspecified;
  EGrSymb symbolType = eol;
  int intVal = 0;
  double doubleVal = 0;
  char *stringVal = NULL;
  bool boolVal = true;
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
        symbolType = opEq;
        mmng_safeFree(tokenID);
        break;
      case '+':
        symbolType = opPlus;
        if(Scanner->line[Scanner->position] == '=')
        {
          symbolType = opPlusEq;
          Scanner->position++;
        }
        allowed = true;
        mmng_safeFree(tokenID);
        break;
      case '-':
        symbolType = opMns;
        if(Scanner->line[Scanner->position] == '=')
        {
          symbolType = opMnsEq;
          Scanner->position++;
        }
        allowed = true;
        mmng_safeFree(tokenID);
        break;
      case '*':
        symbolType = opMul;
        if(Scanner->line[Scanner->position] == '=')
        {
          symbolType = opMulEq;
          Scanner->position++;
        }
        allowed = true;
        mmng_safeFree(tokenID);
        break;
      case '/':
        symbolType = opDiv;
        if(Scanner->line[Scanner->position] == '\'')
        {
          position--;
          delete_comment(false);
          allowed = false;
          symbolType = eol;
        }
        else if(Scanner->line[Scanner->position] == '=')
        {
          tokenID[position++] = Scanner->line[Scanner->position++];
          tokenID[position] = '\0';
          allowed = true;
          symbolType = opDivEq;
          mmng_safeFree(tokenID);
        }
        else
        {
          mmng_safeFree(tokenID);
        }
        break;
      case '\\':
        symbolType = opDivFlt;
        if(Scanner->line[Scanner->position] == '=')
        {
          symbolType = opDivFltEq;
          Scanner->position++;
        }
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case '<':
        symbolType = opLes;
        if(Scanner->line[Scanner->position] == '=')
        {
          symbolType = opLessEq;
          Scanner->position++;
        }
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case '>':
        symbolType = opGrt;
        if(Scanner->line[Scanner->position] == '=')
        {
          symbolType = opGrtEq;
          Scanner->position++;
        }
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case '(':
        symbolType = opLeftBrc;
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case ')':
        symbolType = opRightBrc;
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case ';':
        symbolType = opSemcol;
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case ',':
        symbolType = opComma;
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case ':':
        if(Scanner->line[Scanner->position] == '=')
        {
          symbolType = asng;
          Scanner->position++;
        }
        else
        {
          //error
        }
        mmng_safeFree(tokenID);
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
        && Scanner->line[Scanner->position - 1] != '\\'))
        {
          tokenID[position++] = Scanner->line[Scanner->position++];
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
                if(Scanner->line[Scanner->position] > 47 && Scanner->line[Scanner->position] < 58
                && Scanner->line[Scanner->position + 1] > 47 && Scanner->line[Scanner->position + 1] < 58
                && Scanner->line[Scanner->position + 2] > 47 && Scanner->line[Scanner->position + 2] < 58)
                {
                  //Getting value of a number after '\\'
                  char cNumber = 100 * (Scanner->line[Scanner->position] - '0') + 
                  10 * (Scanner->line[Scanner->position + 1] - '0') +
                  (Scanner->line[Scanner->position + 2] - '0');
                  tokenID[position - 1] = cNumber;
                  Scanner->position = Scanner->position + 3;
                }
                else
                {
                  tokenID[position - 1] = Scanner->line[Scanner->position];
                  Scanner->position++;
                }
            }
          }
        }
        type = symtConstant;
        stringVal = tokenID;
        Scanner->position++;
        tokenID[position++] = '\"';
        tokenID[position] = '\0';
        allowed = true;
        break;
      //End of line
      case '\0':
        symbolType = eol;
        get_line();
        allowed = true;
        break;
      //End of File
      case EOF:
        symbolType = eof;
        tokenID[position] = '\0';
        allowed = true;
        break; 
      default:
        //Identifires
        //value of a-Z
        if((Scanner->line[Scanner->position - 1] > 64 && Scanner->line[Scanner->position - 1] < 91)
        || (Scanner->line[Scanner->position - 1] > 96 && Scanner->line[Scanner->position - 1] < 123))
        {
          while((Scanner->line[Scanner->position - 1] > 64 && Scanner->line[Scanner->position - 1] < 91)
          || (Scanner->line[Scanner->position - 1] > 96 && Scanner->line[Scanner->position - 1] < 123))
          {
            tokenID[position++] = Scanner->line[Scanner->position++];  
          }
          tokenID[position] = '\0';
          allowed = true;
          symbolType = ident;
          //Compare 
          if(strcmp(tokenID, "And") == 0)
          {
            symbolType = kwAnd;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "As") == 0)
          {
            symbolType = kwAs;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "Asc") == 0)
          {
            symbolType = kwAsc;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "Declare") == 0)
          {
            symbolType = kwDeclare;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Dim") == 0)
          {
            symbolType = kwDim;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Do") == 0)
          {
            symbolType = kwDo;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Double") == 0)
          {
            symbolType = dataType;
            mmng_safeFree(tokenID);
            dataType = dtFloat;  
          }
          else if(strcmp(tokenID, "Else") == 0)
          {
            symbolType = kwElse;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "End") == 0)
          {
            symbolType = kwEnd;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "Chr") == 0)
          {
            symbolType = dataType;
            mmng_safeFree(tokenID);
            dataType = dtString; 
          }
          else if(strcmp(tokenID, "Function") == 0)
          {
            symbolType = kwFunction;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "If") == 0)
          {
            symbolType = kwIf;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Input") == 0)
          {
            symbolType = kwInput;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Integer") == 0)
          {
            symbolType = dataType;
            mmng_safeFree(tokenID);
            dataType = dtInt;  
          }
          else if(strcmp(tokenID, "Length") == 0)
          {
            symbolType = kwLength;
            mmng_safeFree(tokenID); 
          }
          else if(strcmp(tokenID, "Loop") == 0)
          {
            symbolType = kwLoop;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Print") == 0)
          {
            symbolType = kwPrint;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Return	") == 0)
          {
            symbolType = kwReturn;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Scope") == 0)
          {
            symbolType = kwScope;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "String") == 0)
          {
            symbolType = dataType;
            mmng_safeFree(tokenID);
            dataType = dtString; 
          }
          else if(strcmp(tokenID, "SubStr") == 0)
          {
            symbolType = kwSubStr;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Then") == 0)
          {
            symbolType = kwThen;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "While") == 0)
          {
            symbolType = kwWhile;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Boolean") == 0)
          {
            symbolType = dataType;
            mmng_safeFree(tokenID);
            dataType = dtBool; 
          }
          else if(strcmp(tokenID, "Continue") == 0)
          {
            symbolType = kwContinue;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "Elseif") == 0)
          {
            symbolType = kwElseif;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "Exit") == 0)
          {
            symbolType = kwExit;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "False") == 0)
          {
            symbolType = kwFalse;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "For") == 0)
          {
            symbolType = kwFor;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Next") == 0)
          {
            symbolType = kwNext;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Not") == 0)
          {
            symbolType = kwNot;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Or") == 0)
          {
            symbolType = kwOr;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Shared") == 0)
          {
            symbolType = kwShared;
            mmng_safeFree(tokenID);  
          }
          else if(strcmp(tokenID, "Static") == 0)
          {
            symbolType = kwStatic;
            mmng_safeFree(tokenID);   
          }
          else if(strcmp(tokenID, "True") == 0)
          {
            symbolType = kwTrue;
            mmng_safeFree(tokenID);  
          }
        }
        //Numbers
        else if(Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)
        {
        symbolType = ident;
          while((Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58) 
          || Scanner->line[Scanner->position - 1] == 46)
          {
            tokenID[position++] = Scanner->line[Scanner->position++];  
          }
          tokenID[position] = '\0';
          allowed = true;
          //Getting value
          if(tokenID[0] > 47 && tokenID[0] < 58)
          {
            bool siInt = true;
            int amount = 0;
            while(tokenID[amount] != '\0')
            {
              if(tokenID[amount] == 46)
              {
                siInt = false;
              }
              amount++;
            }
            if(!siInt)
            {
              doubleVal = strtod(tokenID, NULL);
              type = symtConstant; 
            }
            else
            {
              intVal = strtol(tokenID, NULL, 10);
              type = symtConstant;
            }
          }
        }
        //Space,...
        else if(Scanner->line[Scanner->position - 1] == 9 || Scanner->line[Scanner->position - 1] == 32
        || Scanner->line[Scanner->position - 1] == 0)
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
  if(symbolType == ident)
  {
    symbol = symbt_findOrInsertSymb(tokenID);
    symbol->type = type;
    symbol->data.intVal = intVal;
    symbol->data.doubleVal = doubleVal;
    symbol->data.stringVal = stringVal;
    symbol->data.boolVal = boolVal;
  }
  SToken token;
  token.dataType = dataType;
  token.type = symbolType;
  token.symbol = symbol;
  return token;
}

//destructor of LAnalyzer
void Scanner_destroy()
{
  if (Scanner != NULL)
    mmng_safeFree(Scanner);
}