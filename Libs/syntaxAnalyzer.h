
#ifndef _SYNTAXANALYZER
#define _SYNTAXANALYZER

#include "grammar.h"

void syntx_processExpression(
  SToken *actToken,
  const char *frame,
  const char *ident,
  DataType dataType);

#endif