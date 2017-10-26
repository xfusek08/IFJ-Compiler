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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "appErr.h"
#include "MMng.h"
#include "utils.h"
#include "stacks.h"
#include "symtable.h"

// =============================================================================
// ================= Iternal data structures definition ========================
// =============================================================================

// node of AVL tree main root with no parend reprezenting one symbol table
typedef struct STNode *TSTNode;
struct STNode {
  char *key;        // searching key string
  TSymbol symbol;   // instance of symbol
  TSTNode left;     // root of lft sub-tree
  TSTNode right;    // root of right sub-tree

  // methods
  TSymbol (*find)(TSTNode, char *); // finds symbol with corresponding key
  TSymbol (*insert)(TSTNode, char *); // insert into tree new symbol with key ident, NULL if this key exists
};

// global internal instance of symbol table stack
TPStack GLBSymbTabStack;

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

  newSymb->find = stnode_find;
  newSymb->insert = stnode_insert;
  return newSymb;
}

// destructor of TSymbol
void TSymbol_destroy(TSymbol symbol)
{
  if (symbol != NULL)
    mmng_safeFree(symbol);
}

TSymbol stnode_find(TSTNode root, char *key)
{

}

TSymbol stnode_insert(TSTNode root, char *key)
{

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
// ====================== support function ====================================
// =============================================================================

// error if table is not initialized
void symbt_assertIfNotInit()
{
  if (GLBSymbTabStack != NULL)
    apperr_runtimeError("symbt_init(): Symbol table is not initialized.");
}

// =============================================================================
// ====================== Interface implementation =============================
// =============================================================================

// Initialization of global symbol table stack
void symbt_init()
{
  if (GLBSymbTabStack != NULL)
    apperr_runtimeError("symbt_init(): Symbol table is already initialized.");

  GLBSymbTabStack = TPStack_create();
  symbt_pushFrame(); // insert initial frame
}

//  Free of all symbol table stack
void symbt_destroy()
{
  symbt_assertIfNotInit();
  while (GLBSymbTabStack->count > 0)
    symbt_popFrame();
  GLBSymbTabStack->destroy(GLBSymbTabStack);
  GLBSymbTabStack = NULL;
}

// Creates new instance of symbol table on top of the stack
void symbt_pushFrame()
{
  symbt_assertIfNotInit();
  GLBSymbTabStack->push(GLBSymbTabStack, TSTNode_create(""));
}

// Frees destroys symbol table on top of the stack.
void symbt_popFrame()
{
  symbt_assertIfNotInit();
  TSTNode actST = GLBSymbTabStack->top(GLBSymbTabStack);
  TSTNode_destroy(actST);
  GLBSymbTabStack->pop(GLBSymbTabStack);
}

// Count frames
int symbt_cntFrames()
{
  symbt_assertIfNotInit();
  return GLBSymbTabStack->count;
}

// Finds symbol by indentifier
TSymbol symbt_findSymb(char *ident)
{
  symbt_assertIfNotInit();
  if (ident == NULL)
    return NULL;

  int index = GLBSymbTabStack->count - 1;
  while (index >= 0) // searching from top of the stack
  {
    // we storimg pointers on TSTNode because NULL is also symbol table instance
    TSTNode actTable = GLBSymbTabStack->ptArray[index];
    TSymbol foundSymb = actTable->find(actTable, ident);
    if (foundSymb != NULL)
      return foundSymb;
    index--;
  }
  return NULL;
}

// Finds symbol by identifier or creates new symbol if it's not found.
TSymbol symbt_findOrInsertSymb(char *ident)
{
  symbt_assertIfNotInit();
  if (ident == NULL)
    return NULL;

  TSymbol foundSymb = symbt_findSymb(ident);
  if (foundSymb != NULL)
    return foundSymb;
  return symbt_insertSymbOnTop(ident);
}

// Creates new symbol on top table frame.
TSymbol symbt_insertSymbOnTop(char *ident)
{
  symbt_assertIfNotInit();
  if (ident == NULL)
    return NULL;

  TSTNode topTab = GLBSymbTabStack->top(GLBSymbTabStack);
  return topTab->insert(topTab, ident);
}


// Removes symbol first occurrence of symbol with
void symbt_deleteSymb(char *ident);
