/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    scanner.h
 * \brief   Lexical analyzer
 *
 * It takes code from language preprocessors that are written in the form of sentences.
 * Breaks these syntaxes into a series of tokens, by removing any whitespace or comments in the source code.
 * If the lexical analyzer finds a token invalid, it generates an error.
 *
 * \author  Jaromír Franěk (xfrane16)
 * \date    22.11.2017 - Jaromír Franěk
 */
/******************************************************************************/

#include "utils.h"
#include "grammar.h"
#include "appErr.h"
#include "symtable.h"

#ifndef _scanner
#define _scanner

//Chunk of alocated character for line
#define CHUNK 20

/**
 * Struct representing one token of analysis
 */
typedef struct {
  EGrSymb type;       /*!< terminal lextype from grammar */
  TSymbol symbol;     /*!< Symbol evided in symbol table, NULL if there is no need of additional information */
  DataType dataType;  /*!< This attribute is used only if token type is dataType (we need remember wich data type) and symbol of such of token is unnecessary */
} SToken;

/**
 * Initialization
 *
 * Function prepares internal data structures and allows using another functions of lexical analyzer.
 * This function has to be called before first call of scan_GetNextToken() otherwise error is occured.
 */
void scan_init();

/**
 * Get next token from input
 *
 * Function takes token from input and return it as SToken.
 * Ignore comments.
 * //TODO
 */
SToken scan_GetNextToken();

/**
 * Write error on output
 *
 * Write number of column and row, where the error has occurred.
 * Use appErr to printf errors on stderr.
 */
void scan_raiseCodeError(ErrType typchyby, char *message);

/**
 * Free LAnalyzer
 *
 * Frees all data allocated in LAnalyzer.
 */
void scan_destroy();

#endif // _scanner