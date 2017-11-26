
#ifndef _SYNTAXANALYZER
#define _SYNTAXANALYZER

#include "grammar.h"
#include "symtable.h"
#include "scanner.h"

TSymbol syntx_processExpression(SToken *actToken, TSymbol outsymb);

#endif