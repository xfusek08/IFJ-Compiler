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

EGrSymb isKeyWord(char *tokenID)
{
  //Array of keyWords
  char *sArray[] = {"As", "Asc", "Declare", "Dim", "Do", "Else", "End", "Function", "If", "Input", "Length", "Loop",
  "Print", "Return", "Scope", "SubStr", "Then", "While", "And", "Continue", "Elseif", "Exit", "False", "For",
  "Next", "Not", "Or", "Shared", "Static", "True", "To", "Until"};
  int arrayLeght = 40;
  int i = 18;
  EGrSymb tokenType = ident;
  //Compare
  while(i++ < arrayLeght)
  {
    if(strcmp(tokenID, sArray[i]) == 0)
    {
      tokenType = i;
      mmng_safeFree(tokenID);   
    }
  }
  return tokenType;
}

//Function that return next token
SToken scan_GetNextToken()
{
  char *tokenID = mmng_safeMalloc(sizeof(char) * CHUNK * Scanner->lineSize);
  TSymbol symbol = NULL;
  tokenType type = symtUnknown;
  DataType dataType = dtUnspecified;
  EGrSymb tokenType = eol;
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
        tokenType = opEq;
        mmng_safeFree(tokenID);
        break;
      case '+':
        tokenType = opPlus;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opPlusEq;
          Scanner->position++;
        }
        allowed = true;
        mmng_safeFree(tokenID);
        break;
      case '-':
        tokenType = opMns;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opMnsEq;
          Scanner->position++;
        }
        allowed = true;
        mmng_safeFree(tokenID);
        break;
      case '*':
        tokenType = opMul;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opMulEq;
          Scanner->position++;
        }
        allowed = true;
        mmng_safeFree(tokenID);
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
          mmng_safeFree(tokenID);
        }
        else
        {
          mmng_safeFree(tokenID);
        }
        break;
      case '\\':
        tokenType = opDivFlt;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opDivFltEq;
          Scanner->position++;
        }
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case '<':
        tokenType = opLes;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opLessEq;
          Scanner->position++;
        }
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case '>':
        tokenType = opGrt;
        if(Scanner->line[Scanner->position] == '=')
        {
          tokenType = opGrtEq;
          Scanner->position++;
        }
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case '(':
        tokenType = opLeftBrc;
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case ')':
        tokenType = opRightBrc;
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case ';':
        tokenType = opSemcol;
        mmng_safeFree(tokenID);
        allowed = true;
        break;
      case ',':
        tokenType = opComma;
        mmng_safeFree(tokenID);
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
        tokenType = eol;
        get_line();
        allowed = true;
        break;
      //End of File
      case EOF:
        tokenType = eof;
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
          //Compare
          tokenType = isKeyWord(tokenID);
        }
        //Numbers
        else if(Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)
        {
        tokenType = ident;
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
  if(tokenType == ident)
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
  token.type = tokenType;
  token.symbol = symbol;
  return token;
}

//destructor of LAnalyzer
void Scanner_destroy()
{
  if (Scanner != NULL)
    mmng_safeFree(Scanner);
}