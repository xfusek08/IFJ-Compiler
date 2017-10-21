/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    appErr.h
 * \brief   Application Errors
 *
 * Long decsription will be placed here.
 *
 * \author  Radim Blaha (xblaha28)
 * \date    21.10.2017 - Radim Blaha
 */
/******************************************************************************/

#ifndef appErr
#define appErr

typedef enum {
    lexicalErr = 1,
    syntaxErr = 2,
    semanticErr = 3,
    typeCompatibilityErr = 4,
    anotherSemanticErr = 6,
    internalErr = 99
} ErrType;

/**
 * \brief Initialization
 *
 * Long desc.
 */
void apperr_runtimeError(char *errMsg);

/**
 * \brief Initialization
 *
 * * Long desc.
 */
void apperr_codeError(ErrType type, int row, int col, char *line);

#endif // _appErr
