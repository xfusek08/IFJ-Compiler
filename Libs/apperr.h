/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    appErr.h
 * \brief   Application Errors
 *
 * Library what processes application errors.
 *
 * \author  Radim Blaha (xblaha28)
 * \date    22.10.2017 - Radim Blaha
 */
/******************************************************************************/

#include "scanner.h"

#ifndef appErr
#define appErr

/*! Enum of error types and error codes */
typedef enum
{
  lexicalErr = 1, /*!< incorrect structure of the current lex */
  syntaxErr = 2,  /*!< wrong program syntax */
  semanticErr = 3,    /*!< undefined function/variable, attempt to redefine function/variable, etc. */
  typeCompatibilityErr = 4,   /*!< semantic error of type compatibility in arithmetic, string and relational */
                              /*!< expressions, or poor number of parameter types for function calls */
  anotherSemanticErr = 6, /*!< another semantic error */
  internalErr = 99    /*!< internal compiler error - unaffected by the input program (eg. memory allocation error, etc.) */
} ErrType;

/**
 * \brief Prints errors
 *
 * Print errors another then defined and terminates program with error code 10.
 *
 * \param errMsg array of chars (string) containg error message
 */
void apperr_runtimeError(char *errMsg);

/**
 * \brief Print error and terminates program
 *
 * Function prints compiling error and terminates program.
 *
 * \param type defined error codes (ErrType enum)
 * \param row row where the error occurred
 * \param col column where the error occurred
 * \param line wrong section of code
 * \param message print given message to output
 * \param token define token that caused error
 */
void apperr_codeError(ErrType type, int row, int col, char *line, char *message, SToken *token);

/**
 * Write error on output
 *
 * Write number of column and row, where the error has occurred.
 * Use appErr to printf errors on stderr.
 */
void scan_raiseCodeError(ErrType typchyby, char *message, SToken *token);

#endif // _appErr
