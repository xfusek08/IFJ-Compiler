/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    symtable.h
 * \brief   Symbol table implementation interface
 *
 * This package provides functions over global stack of symbol tables.
 *
 * \author  Petr Fusek (xfusek08)
 * \date    18.10.2017 - Petr Fusek
 */
/******************************************************************************/

#ifndef _SymbTab
#define _SymbTab

#include <stdbool.h>
#include "utils.h"

/**
 * Types of symbols evided in symbol table.
 */
typedef enum {
  symtUnknown,  // unknown type
  symtFuction,  // data type function
  symtInt,      // data type int
  symtFloat,    // data type float
  symtBool,     // data type bool
  symtString    // data type string
} SymbolType;

/**
 * Object reprezenting one symbol
 *
 * There is no public constructor to this object. It is created by symbol table itself while inserting item.
 * Same case is with destroying object
 *
 * \TODO: is identificator nesessary ?
 */
typedef struct Symbol *TSymbol;
struct Symbol {
  char *ident;      // Identificator string
  SymbolType type;  // data type of symbol
  bool IsConstant;  // flag, true whenever simbol is comstant and is relevant to remember some value
  SData value;      // Internal struct, filled in case of constant
};

/**
 * Initialization of global symbol table stack
 *
 * Do new allocation of stack of table and it is supposed that function
 * is called only for initialization.
 * If function is called and symbol table is already initialized error is thrown
 * to prevent memory leaks.
 */
void symbt_init();

/**
 * Free of all symbol table stack
 *
 * Frees all data allocated in all tables on stack and clears stack.
 */
void symbt_destroy();

/**
 * Creates new instance of symbol table on top of the stack
 */
void symbt_pushFrame();

/**
 * Frees destroys symbol table on top of the stack.
 *
 * If the is only one table left to be poped,
 * it will be deleted and there is created new empty table insted of it
 */
void symbt_popFrame();

/**
 * Count frames
 *
 * \returns unsigned int number or frames (tables on stack)
 */
int symbt_cntFrames();

/**
 * Finds symbol by indentifier
 *
 * It is search from top to bottom of stack.
 * \param  char* ident string identifier used as key
 * \returns TSymbol Fisrt occurrence of symbol with corresponding identifier, NULL if not exist
 */
TSymbol symbt_findSymb(char *ident);

/**
 * Finds symbol by identifier or creates new symbol if it's not found.
 *
 * Searching for existing identifier is through all tables on stack, hence new
 * symbol is created only if it doesn't exists at all.
 * \param  char* ident string identifier used as key
 * \returns TSymbol First occurrence or new symbol in table by indetifier.
 * \note To hold identifier ident as key is created new string and key value is copied.
 * so ident string is parameter could be safely freed.
 */
TSymbol symbt_findOrInsertSymb(char *ident);


/**
 * Creates new symbol on top table frame.
 *
 * Symbol is created only if it doesn't exists on top table frame.
 * If symbol with same identifier already exists returns NULL.
 * \param  char* ident string identifier used as key
 * \returns TSymbol newly created symbol, NULL if symbol exists
 */
TSymbol symbt_insertSymbOnTop(char *ident);

/**
 * Removes symbol first occurrence of symbol with
 *
 * \param  char* ident string identifier used as key
 */
void symbt_deleteSymb(char *ident);

#endif // _SymbTab
