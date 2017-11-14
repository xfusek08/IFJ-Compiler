/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Scanner.c
 * \brief   Lexical analyzer
 * \author  Jaromír Franěk (xfrane16)
 * \date    14.11.2017 - Jaromír Franěk
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
typedef struct LAnalyzer *LAPointer;
struct LAnalyzer {
  int curentLine;
  int position;
  int lineSize; //in CHUNKS
  char *line; 
};

// global internal instance of lexical analyzer
LAPointer Scanner;

//Lexical analyzer constructor
LAPointer Scanner_create()
{
  LAPointer newScanner = (LAPointer)malloc(sizeof(struct LAnalyzer));
  if (newScanner == NULL)
    apperr_runtimeError("Allocation error in Scanner");

  newScanner->curentLine = 0;
  newScanner->position = 0;
  newScanner->lineSize = 1;
  newScanner->line = malloc(sizeof(char) * CHUNK * newScanner->lineSize);
  if (newScanner->line == NULL)
    apperr_runtimeError("Allocation error in Scanner");

  return newScanner;
}

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

void delete_comment(bool isLine)
{
  if(isLine == true)
  {
    Scanner->curentLine++;
    get_line();
  }  
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

SToken scan_GetNextToken()
{
  char *tokenID = malloc(sizeof(char) * CHUNK * Scanner->lineSize);
  TSymbol symbol = NULL;
  SymbolType type = symtUnknown;
  DataType dataType = dtUnspecified;
  EGrSymb EG = eol;
  int intVal = 0;
  double doubleVal = 0;
  char *stringVal = NULL;
  bool boolVal = true;
  int position = 0;
  bool allowed = false;
  //Getting next token (retezec)
  while(!allowed)
  {
    switch(tokenID[position++] = Scanner->line[Scanner->position++])
    {
      case '=':
        allowed = true;
        EG = opEq;
        free(tokenID);
        break;
      case '+':
        EG = opPlus;
        if(Scanner->line[Scanner->position] == '=')
        {
          EG = opPlusEq;
          Scanner->position++;
        }
        allowed = true;
        free(tokenID);
        break;
      case '-':
        EG = opMns;
        if(Scanner->line[Scanner->position] == '=')
        {
          EG = opMnsEq;
          Scanner->position++;
        }
        allowed = true;
        free(tokenID);
        break;
      case '*':
        EG = opMul;
        if(Scanner->line[Scanner->position] == '=')
        {
          EG = opMulEq;
          Scanner->position++;
        }
        allowed = true;
        free(tokenID);
        break;
      case '/':
        EG = opDiv;
        if(Scanner->line[Scanner->position] == '\'')
        {
          position--;
          delete_comment(false);
          allowed = false;
          EG = eol;
        }
        else if(Scanner->line[Scanner->position] == '=')
        {
          tokenID[position++] = Scanner->line[Scanner->position++];
          tokenID[position] = '\0';
          allowed = true;
          EG = opDivEq;
          free(tokenID);
        }
        else
        {
          free(tokenID);
        }
        break;
      case '\\':
        EG = opDivFlt;
        if(Scanner->line[Scanner->position] == '=')
        {
          EG = opDivFltEq;
          Scanner->position++;
        }
        free(tokenID);
        allowed = true;
        break;
      case '<':
        EG = opLes;
        if(Scanner->line[Scanner->position] == '=')
        {
          EG = opLessEq;
          Scanner->position++;
        }
        free(tokenID);
        allowed = true;
        break;
      case '>':
        EG = opGrt;
        if(Scanner->line[Scanner->position] == '=')
        {
          EG = opGrtEq;
          Scanner->position++;
        }
        free(tokenID);
        allowed = true;
        break;
      case '(':
        EG = opLeftBrc;
        free(tokenID);
        allowed = true;
        break;
      case ')':
        EG = opRightBrc;
        free(tokenID);
        allowed = true;
        break;
      case ';':
        EG = opSemcol;
        free(tokenID);
        allowed = true;
        break;
      case ',':
        EG = opComma;
        free(tokenID);
        allowed = true;
        break;
      case ':':
        if(Scanner->line[Scanner->position] == '=')
        {
          EG = asng;
          Scanner->position++;
        }
        else
        {
          //error
        }
        free(tokenID);
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
        EG = eol;
        get_line();
        allowed = true;
        break;
      //End of File
      case EOF:
        EG = eof;
        tokenID[position] = '\0';
        allowed = true;
        break; 
      default:
        //Identifires
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
          EG = ident;
          //Compare 
          if(strcmp(tokenID, "And") == 0)
          {
            EG = kwAnd;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "As") == 0)
          {
            EG = kwAs;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "Asc") == 0)
          {
            EG = kwAsc;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "Declare") == 0)
          {
            EG = kwDeclare;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Dim") == 0)
          {
            EG = kwDim;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Do") == 0)
          {
            EG = kwDo;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Double") == 0)
          {
            EG = dataType;
            free(tokenID);
            dataType = dtFloat;  
          }
          else if(strcmp(tokenID, "Else") == 0)
          {
            EG = kwElse;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "End") == 0)
          {
            EG = kwEnd;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "Chr") == 0)
          {
            EG = dataType;
            free(tokenID);
            dataType = dtString; 
          }
          else if(strcmp(tokenID, "Function") == 0)
          {
            EG = kwFunction;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "If") == 0)
          {
            EG = kwIf;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Input") == 0)
          {
            EG = kwInput;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Integer") == 0)
          {
            EG = dataType;
            free(tokenID);
            dataType = dtInt;  
          }
          else if(strcmp(tokenID, "Length") == 0)
          {
            EG = kwLength;
            free(tokenID); 
          }
          else if(strcmp(tokenID, "Loop") == 0)
          {
            EG = kwLoop;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Print") == 0)
          {
            EG = kwPrint;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Return	") == 0)
          {
            EG = kwReturn;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Scope") == 0)
          {
            EG = kwScope;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "String") == 0)
          {
            EG = dataType;
            free(tokenID);
            dataType = dtString; 
          }
          else if(strcmp(tokenID, "SubStr") == 0)
          {
            EG = kwSubStr;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Then") == 0)
          {
            EG = kwThen;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "While") == 0)
          {
            EG = kwWhile;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Boolean") == 0)
          {
            EG = dataType;
            free(tokenID);
            dataType = dtBool; 
          }
          else if(strcmp(tokenID, "Continue") == 0)
          {
            EG = kwContinue;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "Elseif") == 0)
          {
            EG = kwElseif;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "Exit") == 0)
          {
            EG = kwExit;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "False") == 0)
          {
            EG = kwFalse;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "For") == 0)
          {
            EG = kwFor;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Next") == 0)
          {
            EG = kwNext;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Not") == 0)
          {
            EG = kwNot;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Or") == 0)
          {
            EG = kwOr;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Shared") == 0)
          {
            EG = kwShared;
            free(tokenID);  
          }
          else if(strcmp(tokenID, "Static") == 0)
          {
            EG = kwStatic;
            free(tokenID);   
          }
          else if(strcmp(tokenID, "True") == 0)
          {
            EG = kwTrue;
            free(tokenID);  
          }
        }
        //Numbers
        else if(Scanner->line[Scanner->position - 1] > 47 && Scanner->line[Scanner->position - 1] < 58)
        {
        EG = ident;
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
  if(EG == ident)
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
  token.type = EG;
  token.symbol = symbol;
  return token;
}

//destructor of LAnalyzer
void Scanner_destroy()
{
  if (Scanner != NULL)
    free(Scanner);
}