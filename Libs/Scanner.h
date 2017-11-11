/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    Scanner.h
* \brief   This is test module
*
*
*
* \author  Pavel Vosyka (xvosyk00)
* \date    28.10.2017 - Pavel Vosyka
*/
/******************************************************************************/

#ifndef _SCANNER
#define _SCANNER

#include "grammar.h"

typedef struct {
  EGrSymb type; // typ tokenu

} SToken;

int scan_GetNextToken(SToken *token);




#endif // _SCANNER