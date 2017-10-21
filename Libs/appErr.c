/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    appErr.c
 * \brief   Application Errors
 * \author  Radim Blaha (xblaha28)
 * \date    21.10.2017 - Radim Blaha
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "appErr.h"

void apperr_runtimeError(char *errMsg){
    fprintf( stderr, "%s\n", errMsg);
    // TODO: dealloc memory
    exit(2);
}

void apperr_codeError(ErrType type, int row, int col, char *line){
    switch (type)
    {
        case lexicalErr:
            fprintf( stderr, "Error %d: Lexical error caused by %s.\n", type, line);
        case syntaxErr:
            fprintf( stderr, "Error %d: Syntax error caused by %s.\n", type, line);
        case semanticErr:
            fprintf( stderr, "Error %d: Semantic error caused by %s.\n", type, line);
        case typeCompatibilityErr:
            fprintf( stderr, "Error %d: Error of type compatibility caused by %s.\n", type, line);
        case anotherSemanticErr:
            fprintf( stderr, "Error %d: Another semantic error caused by %s.\n", type, line);
        case internalErr:
            fprintf( stderr, "Error %d: Internal compiler error caused by %s.\n", type, line);
        default:
            return 0;
    }

    fprintf( stderr, "On Ln %d, Col %d.\n", row, col);
    exit(type);
}