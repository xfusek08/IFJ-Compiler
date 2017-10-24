/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    appErr.c
 * \brief   Application Errors
 * \author  Radim Blaha (xblaha28)
 * \date    24.10.2017 - Radim Blaha
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appErr.h"
#include "MMng.h"

void apperr_runtimeError(char *errMsg){
  fprintf( stderr, "\033[31;1mError:\033[0m %s\n", errMsg);
  mmng_freeAll();
  exit(internalErr);
}

void apperr_codeError(ErrType type, int row, int col, char *line){
    
  switch (type)
  {
    case lexicalErr:
      fprintf( stderr, "\033[31;1mError %d:\033[0m Lexical error ", type);
      break;
    case syntaxErr:
      fprintf( stderr, "\033[31;1mError %d:\033[0m Syntax error ", type);
      break;
    case semanticErr:
      fprintf( stderr, "\033[31;1mError %d:\033[0m Semantic error ", type);
      break;
    case typeCompatibilityErr:
      fprintf( stderr, "\033[31;1mError %d:\033[0m Error of type compatibility ", type);
      break;
    case anotherSemanticErr:
      fprintf( stderr, "\033[31;1mError %d:\033[0m Another semantic error ", type);
      break;
    default:
      return;
  }

  fprintf( stderr, "\033[33m[%d:%d]\033[0m:\n", row, col);

  unsigned int i = 1;
  int consoleRowNum = 0;

  while(i <= strlen(line)){
    int j = 0;
    for(; j < 80 && i <= strlen(line); j++){
      printf("%c", line[i+j-1]);
    }

    i += j;

    printf("\n");

    int rowFirstCharPosition = (consoleRowNum)*80 + 1;

    if(rowFirstCharPosition <= col && col < rowFirstCharPosition + 80){
      for(int j = 1; j <= (col-1)%80; j++){
        printf(" ");
      }
      printf("\x1B[31m^\x1B[0m\n");
    }
        
    consoleRowNum++;
  }

  mmng_freeAll();
  exit(type);
}