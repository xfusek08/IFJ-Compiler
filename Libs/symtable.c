/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    symtable.c
 * \brief   Symbol table implementation
 * \author  Petr Fusek (xfusek08)
 * \date    18.10.2017 - Petr Fusek
 */
/******************************************************************************/

// references
#include "symtable.h"

// =============================================================================
// ======================= Constant Definition ================================
// =============================================================================

// size of array of symbol table stack, stack is extended by
// this size when it's full and push is called
#define CHUNK_SIZE 50

// =============================================================================
// ================= Iternal data structures definition ========================
// =============================================================================

// root node of AVL tree main root with no parend reprezenting one symbol table
typedef struct STNode *TSTNode;
struct STNode {
  char *key;        // searching key string
  TSymbol symbol;   // instance of symbol
  TSTNode left;     // root of lft sub-tree
  TSTNode right;    // root of right sub-tree
};

// stack of AVL trees reprezenting final stack of symbol tables
typedef struct {
  TSTNode *tables;          // pointer to first item of array of symbol tables
  unsigned int maxHeight;   // actual real allocated size of tables array
  int top;                  // index of top table on stack, -1 if empty
} STStack;

// global internal instance of symbol table stack
STStack symbolTableStack;

// =============================================================================
// ======================= TSymbol implementation ==============================
// =============================================================================

// constructor of TSymbol
TSymbol TSymbol_create(char *ident)
{
  TSymbol newSymb = (TSymbol)mmng_safeMalloc(sizeof(struct Symbol));
  newSymb->ident = ident;
  newSymb->type = symtUnknown;
  newSymb->IsConstant = false;
  newSymb->value.intVal = 0;
  newSymb->value.doubleVal = 0.0;
  newSymb->value.stringVal = NULL;
  return newSymb;
}

// destructor of TSymbol
void TSymbol_destroy(TSymbol symbol)
{
  (void)symbol;
  exit(99);
}

// =============================================================================
// ======================= TSTNode implementation ==============================
// =============================================================================

// constructor of TSTNode
TSTNode TSTNode_create(char *key)
{
  TSTNode newNode = (TSTNode)mmng_safeMalloc(sizeof(struct STNode));
  newNode->key = util_StrHardCopy(key); // new hard copy of string
  newNode->symbol = NULL;
  newNode->left = NULL;
  newNode->right = NULL;
  return newNode;
}

// destructor of TSTNode
void TSTNode_destroy(TSTNode node)
{
  // delete subtrees recursively
  if (node->left != NULL)
    TSTNode_destroy(node->right);
  if (node->right != NULL)
    TSTNode_destroy(node->right);

  // destroy symbol
  TSymbol_destroy(node->symbol);
  // free key string
  mmng_safeFree(node->key);
  // free self
  mmng_safeFree(node);
}


// =============================================================================
// ======================= TSTStack implementation =============================
// =============================================================================



// =============================================================================
// ====================== support function ====================================
// =============================================================================

// error if table is not initialized
void assertIfNotInit()
{
  if (symbolTableStack.tables = NULL)
    exit(99);
}

// =============================================================================
// ====================== Interface implementation =============================
// =============================================================================

// Initialization of global symbol table stack
void symbt_init()
{
  if (symbolTableStack.tables != NULL)
    exit(99);

  symbolTableStack.tables = (TSTNode *)mmng_safeMalloc(sizeof(TSTNode) * CHUNK_SIZE);
  symbolTableStack.top = -1;
  symbolTableStack.maxHeight = CHUNK_SIZE;
}

//  Free of all symbol table stack
void symbt_destroy()
{
  while(symbolTableStack.top > 0)
    symbt_popFrame();

  // is needed to destroy last frame because poping ceeps atleast one initialized
  TSTNode_destroy(symbolTableStack.tables[symbolTableStack.top]);
  mmng_safeFree(symbolTableStack.tables); // free tables array
  symbolTableStack.tables = NULL;
  symbolTableStack.top = -1;
}

// Creates new instance of symbol table on top of the stack
void symbt_pushFrame()
{

}

// Frees destroys symbol table on top of the stack.
void symbt_popFrame()
{
  if (symbolTableStack.top >= 0)
  {
    TSTNode_destroy(symbolTableStack.tables[symbolTableStack.top]);
    symbolTableStack.top--;
  }
}

// Count frames
unsigned int symbt_cntFrames();

// Finds symbol by indentifier
TSymbol symbt_findSymb(char *ident);

// Finds symbol by identifier or creates new symbol if it's not found.
TSymbol symbt_findOrCreateSymb(char *ident);


// Removes symbol first occurrence of symbol with
void symbt_deleteSymb(char *ident);
