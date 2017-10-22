/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    appErr.c
 * \brief   Application Errors
 * \author  Radim Blaha (xblaha28)
 * \date    22.10.2017 - Radim Blaha
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "appErr.h"
#include "MMng.h"

void apperr_runtimeError(char *errMsg){
    fprintf( stderr, "\033[31;1mError:\033[0m %s\n", errMsg);
    mmng_freeAll();
    exit(10);
}

void apperr_codeError(ErrType type, int row, int col, char *line){
    
    switch (type)
    {
        case lexicalErr:
            fprintf( stderr, "\033[31;1mError %d:\033[0m Lexical error:\n%s. ", type, line);
            break;
        case syntaxErr:
            fprintf( stderr, "\033[31;1mError %d:\033[0m Syntax error:\n%s. ", type, line);
            break;
        case semanticErr:
            fprintf( stderr, "\033[31;1mError %d:\033[0m Semantic error:\n%s. ", type, line);
            break;
        case typeCompatibilityErr:
            fprintf( stderr, "\033[31;1mError %d:\033[0m Error of type compatibility:\n%s. ", type, line);
            break;
        case anotherSemanticErr:
            fprintf( stderr, "\033[31;1mError %d:\033[0m Another semantic error:\n%s. ", type, line);
            break;
        case internalErr:
            fprintf( stderr, "\033[31;1mError %d:\033[0m Internal compiler error:\n%s. ", type, line);
            break;
        default:
            return;
    }

    fprintf( stderr, "\033[33mOn Ln %d, Col %d.\033[0m\n", row, col);
    exit(type);
}