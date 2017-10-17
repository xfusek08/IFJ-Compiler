/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    symtable.h
 * \brief   Symbol table
 *
 * This package provides functions over global stack of symbol tables.
 *
 * \author  Petr Fusek (xfusek08)
 * \date    17.10.2017 - Petr Fusek
 */
/******************************************************************************/

#ifndef _SymbTab
#define _SymbTab

/**
 * \brief types of symbols evided in symbol table 
 */
enum SymbolType
{
  /// data type function
  symtFuction,  
  /// data type int 
  symtInt,      
  /// data type float 
  symtFloat,    
  /// data type bool
  symtBool,     
  /// data type string
  symtString    
}

/**
 * \brief Object reprezenting one symbol
 * 
 * There is no public constructor to this object. It is created by symbol table itself while inserting item.
 * Same case is with destroying object 
 * 
 * \TODO: is identificator nesessary ?
 */
typedef struct Symbol *TSymbol;
struct Symbol {
  ///Identificator string
  char *ident;
  /// data type of symbol 
  SymbolType type;
  /// flag, true whenever simbol is comstant and is relevant to remember some value
  bool IsConstant;
  /// Internal struct, filled in case of constant 
  SData value;
};

/**
 * \brief Initialization of global symbol table stack
 */
void symbt_init();

/**
 * \brief Free of all symbol table stack
 * 
 * Frees all data allocated in all tables on stack and clears stack. 
 */
void symbt_destroy();


/**
 * \brief Creates new instance of symbol table on top of the stack
 */
void symbt_pushFrame();

/**
 * \brief Frees destroys symbol table on top of the stack.
 * 
 * If the is only one table left to be poped, 
 * it will be deleted and there is created new empty table insted of it 
 */
void symbt_popFrame();

/**
 * \brief Count frames
 * \returns unsigned int number or frames (tables on stack) 
 */
unsigned int symbt_cntFrames();


/**
 * \brief Finds symbol by indentifier
 * 
 * It is search from top to bottom of stack.
 * \param  char* ident string if identifier used as key
 * \returns TSymbol Fisrt occurrence of symbol with corresponding identifier, NULL if not exist 
 */
TSymbol symbt_findSymb(char *ident);

/**
 * \brief Finds symbol by identifier or creates new symbol if it's not found.
 * \returns TSymbol First occurrence or new symbol in table by indetifier. 
 * \note To hold identifier ident as key is created new string and key value is copied.
 * so ident as string could be safely freed.    
 */
TSymbol symbt_findOrCreateSymb(char *ident);


/**
 * \brief 
 */
void symbt_deleteSymb(char *ident)

Vymaže a uvolní symbol podle klíče. (neuvolní parametr ident)

#endif // _SymbTab
