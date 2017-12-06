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
#include "apperr.h"
#include "mmng.h"

void apperr_runtimeError(char *errMsg)
{
  fprintf( stderr, "\033[31;1mInternal runtime error (99):\033[0m %s\n", errMsg);
  mmng_freeAll();
  exit(internalErr);
}

void apperr_codeError(ErrType type, int row, int col, char *line, char *message, SToken *token)
{
  switch (type)
  {
    case lexicalErr:
      fprintf( stderr, "\033[31;1mLexical error (%d):\033[0m ", type);
      break;
    case syntaxErr:
      fprintf( stderr, "\033[31;1mSyntax error (%d):\033[0m ", type);
      break;
    case semanticErr:
      fprintf( stderr, "\033[31;1mSematic error (%d): \033[0m Undefined symbol or invalid definition or redefinition ", type);
      break;
    case typeCompatibilityErr:
      fprintf( stderr, "\033[31;1mSematic error (%d):\033[0m type incompatibility ", type);
      break;
    case anotherSemanticErr:
      fprintf( stderr, "\033[31;1mSematic error (%d):\033[0m ", type);
      break;
    default:
      return;
  }
  fprintf(stderr, "\033[33m%d/%d\033[0m\n", row, col);
  if(token != NULL)
    fprintf(stderr, "Caused by token: \"%s\"\n", grammarToString(token->type));
  if (message != NULL)
    fprintf(stderr, "Error message: %s\n", message);

  fprintf(stderr, " \033[2m% 3d |\033[0m ", row);
  col += 7;
  int consoleRowNum = 0;

  unsigned int i = 0;
  unsigned int linewidth = strlen(line);
  while (i < linewidth)
  {
    int j = 0;
    for (; j < 80 && (i + j) < linewidth; j++)
    {
      if (line[i + j] == '\t')
        fprintf(stderr, "  ");
      else
        fprintf(stderr, "%c", line[i + j]);
    }

    i += j;

    int rowFirstCharPosition = (consoleRowNum)*80;

    if(rowFirstCharPosition < col && col < rowFirstCharPosition + 80)
    {
      for(int j = 1; j <= (col-1)%80; j++)
        fprintf(stderr, " ");
      fprintf(stderr, "\x1B[31m^\x1B[0m\n");
    }

    consoleRowNum++;
  }
  mmng_freeAll();
  exit(type);
}
