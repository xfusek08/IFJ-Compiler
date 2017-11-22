/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    symtable.h
 * \brief   Symbol table implementation interface
 *
 * This package provides functions over global stack of symbol tables.
 *
 * \author  Petr Fusek (xfusek08)
 * \date    21.11.2017 - Petr Fusek
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
  symtUnknown,
  symtFuction,
  symtVariable,
  symtConstant
} SymbolType;

/**
 * Object reprezenting one argument in argument list (TArgList)
 */
typedef struct Argument *TArgument;
struct Argument {
  char *ident;        /*!< identifier of argument */
  DataType dataType;  /*!< data type of argument */
  TArgument next;     /*!< next argument in list */
};

/**
 * Object reprezenting list of arguments
 */
typedef struct ArgList *TArgList;
struct ArgList {
  int count;        /*!< count of arguments in list */
  TArgument head;   /*!< first argument in list */
  TArgument tail;   /*!< last argument in list */
  /**
   * Get n-th arument from head in TArgList;
   *
   * \param TArgList self self reference
   * \param int index n = index - position of agrument from begining
   * \returns TArgument argument on position index
   */
  TArgument (*get)(TArgList self, int index);

  /**
   * Create new argument on the end of the list
   *
   * \param TArgList self self reference
   * \param const char *ident identifier of new argument
   * \param DataType dataType data type of new argument
   * \returns TArgument new created argument
   */
  TArgument (*insert)(TArgList self, char *ident, DataType dataType);
};

/**
 * Constructor of argument list
 * \returns TArgList new argument list
 */
TArgList TArgList_create();

/**
 * Destructor of argument list
 * \param TArgList self self reference
 */
void TArgList_destroy(TArgList self);

/**
 * Structure used in data, containg information about Function symbol.
 */
typedef struct {
  char *label;          /*!< label of function used in output code */
  DataType returnType;  /*!< return type of function */
  bool isDefined;       /*!< flag, true if function has been defined */
  TArgList arguments;   /*!< List of arguments, NULL on initialization */
} SFuncData;

/**
 * Union of mutually excluded attributes wich are used for different SymbolTypes
 */
typedef union {
  int intVal;           /*!< integer value used when  symbol type is symtConstInt */
  double doubleVal;     /*!< double value used when  symbol type is symtConstDouble */
  char *stringVal;      /*!< pointer to first char of string literal value used when symbol type is symtConstString */
  bool boolVal;         /*!< boolean value used when symbol type is symtConstBool */
  SFuncData funcData;   /*!< structure containg inforamtion aboud function used when symbol type is symtFuction */
} Data;

/**
 * Object reprezenting one symbol
 *
 * There is no public constructor to this object. It is created by symbol table itself while inserting item.
 * Same case is with destroying object
 */
typedef struct Symbol *TSymbol;
struct Symbol {
  char *ident;          /*!< identifier of symbol (is also used as key to symbol table) */
  SymbolType type;      /*!< Type of symbol */
  DataType dataType;    /*!< Data type of symbol, in case of constant type desides wich attribute from Data union will be used to store information. */
  Data data;            /*!< Union of attributes containg right data for concrete type of symbol. */
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
 * \param bool transparent flag true if created frame is suppose to be transparent
 * (searching for symbols continues on next table on stack)
 */
void symbt_pushFrame(bool transparent);

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
 * It is search in top frame and if there is not fouded searchs in global frame
 * If table frame is transparent
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
 * Creates new symbol in top table frame.
 *
 * Symbol is created only if it doesn't exists on top table frame.
 * If symbol with same identifier already exists returns NULL.
 * \param  char* ident string identifier used as key
 * \returns TSymbol newly created symbol, NULL if symbol exists
 */
TSymbol symbt_insertSymbOnTop(char *ident);

/**
 * Removes symbol from top table in stack with identifier ident
 *
 * Does nothing when such symbol not exists.
 * \param  char* ident string identifier used as key
 */
void symbt_deleteSymb(char *ident);

// funkce pouze pro testovaci programy

#ifdef DEBUG

/**
 * Prints top table as binary tree into stdout
 */
void symbt_print();

/**
 * Prints instance of TSymbol into stdout
 */
void symbt_printSymb(TSymbol symbol);

#endif // DEBUG

#endif // _SymbTab
