/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    symtable.c
 * \brief   Symbol table implementation
 * \author  Petr Fusek (xfusek08)
 * \date    30.10.2017 - Petr Fusek
 */
/******************************************************************************/

// references
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "appErr.h"
#include "MMng.h"
#include "utils.h"
#include "stacks.h"
#include "symtable.h"

// =============================================================================
// ================= Iternal data structures definition ========================
// =============================================================================

// node of AVL tree main root with no parent reprezenting one symbol table
typedef struct STNode *TSTNode;
struct STNode {
  int balance;      // balance factor of the tree
  char *key;        // searching key string
  TSymbol symbol;   // instance of symbol
  TSTNode parent;   // parent node
  TSTNode left;     // root of lft sub-tree
  TSTNode right;    // root of right sub-tree
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
  return newSymb;
}

// destructor of TSymbol
void TSymbol_destroy(TSymbol symbol)
{
  if (symbol != NULL)
    mmng_safeFree(symbol);
}

// =============================================================================
// ======================= TSTNode implementation ==============================
// =============================================================================

// Constructor of TSTNode
TSTNode TSTNode_create(char *key)
{
  TSTNode newNode = (TSTNode)mmng_safeMalloc(sizeof(struct STNode));
  newNode->balance = 0;
  newNode->key = util_StrHardCopy(key); // new hard copy of string
  newNode->symbol = NULL;
  newNode->parent = NULL;
  newNode->left = NULL;
  newNode->right = NULL;
  return newNode;
}

// destructor of TSTNode
void TSTNode_destroy(TSTNode node)
{
  // delete subtrees recursively
  if (node->left != NULL)
    TSTNode_destroy(node->left);
  if (node->right != NULL)
    TSTNode_destroy(node->right);

  // destroy symbol
  TSymbol_destroy(node->symbol);
  // free key string
  mmng_safeFree(node->key);
  // free self
  mmng_safeFree(node);
}

// counts height of tree
int TSTNode_height(TSTNode self)
{
  if (self == NULL)
    return 0;
  if (self->symbol == NULL)
    return 0;
  int height_left = 1;
  int height_right = 1;
  if (self->right != NULL)
    height_left += TSTNode_height(self->right);
  if (self->left != NULL)
    height_right += TSTNode_height(self->left);

  // return bigger of heights
  return (height_left > height_right) ? height_left : height_right;
}

// return root of tree where self is
TSTNode TSTNode_getRoot(TSTNode self)
{
  while (self->parent != NULL)
    self = self->parent;
  return self;
}

// prints simple diagram reprezenting structure of tree
void TSTNode_print(TSTNode node, int depht)
{
  if (node != NULL)
  {
    if (node->right != NULL)
      TSTNode_print(node->right, depht + 1);

    for (int i = 0; i < depht; i++)
    {
      if (i + 1 == depht)
        printf(" •--");
      else
        printf("    ");
    }
    printf("[%s]\n", node->key);

    if (node->left != NULL)
      TSTNode_print(node->left, depht + 1);
  }
}

// Creates and rewrites symbol on concrete TSTNode with key,
// key of node is changed
TSymbol TSTNode_newSymbol(TSTNode self, char *key)
{
  if (self == NULL)
    apperr_runtimeError("Symbol table: Invalid NULL parameter while calling newSymbol method.");

  if (self->symbol != NULL)
    TSymbol_destroy(self->symbol);

  if (self->key != NULL)
  {
    mmng_safeFree(self->key);
    self->key = util_StrHardCopy(key);
  }
  self->symbol = TSymbol_create(self->key);
  return self->symbol;
}

// operation RR
void TSTNode_rotateRight(TSTNode self)
{
  if (self == NULL || self->left == NULL)
    return;

  printf("RotRight: %s\n", self->key);
  // change parent pointer to left child
  if (self->parent != NULL)
  {
    if (self->parent->left == self)         // self is on left
      self->parent->left = self->left;
    else                                    // self is on right
      self->parent->right = self->left;
  }

  // switch position with your left child
  self->left->parent  = self->parent;
  self->parent        = self->left;
  self->left          = self->parent->right;
  self->parent->right = self;
  if (self->left != NULL)
    self->left->parent  = self;
}

// operation LL
void TSTNode_rotateLeft(TSTNode self)
{
  if (self == NULL || self->right == NULL)
    return;

  printf("RotLeft: %s\n", self->key);
  // change parent pointer to left child
  if (self->parent != NULL)
  {
    if (self->parent->right == self)         // self is on left
      self->parent->right = self->right;
    else                                    // self is on right
      self->parent->left = self->right;
  }

  // switch position with you left child
  self->right->parent = self->parent;
  self->parent        = self->right;
  self->right         = self->parent->left;
  self->parent->left  = self;
  if (self->right != NULL)
    self->right->parent  = self;
}


// methode starts from some node in tree and balances tree from node up to its root node
void TSTNode_balanceFromBottom(TSTNode node)
{
  #ifdef ST_DEBUG
  printf("Balance %s\n", node->key);
  #endif

  int balanceFactor = 0;
  int balanceFactorpPrev = 0;
  while (node != NULL)
  {
    balanceFactor = TSTNode_height(node->left) - TSTNode_height(node->right);

    if (balanceFactor < -1)       // -2 - right side is taller
    {
      if (balanceFactorpPrev > 0) // 1 - left side of right child is taller => RL
        TSTNode_rotateRight(node->right);
      TSTNode_rotateLeft(node);
    }
    else if (balanceFactor > 1)   // 2 - left side is taller
    {
      if (balanceFactorpPrev < 0) // -1 - right side of left child is taller => LR
        TSTNode_rotateLeft(node->left);
      TSTNode_rotateRight(node);
    }

    // we need remember previous child node for resove type of operation
    balanceFactorpPrev = balanceFactor;
    node = node->parent;
  }
}

// Finds symbol with corresponding key, NULL if not found
TSymbol TSTNode_find(TSTNode self, char *key)
{
  if (self == NULL)
    apperr_runtimeError("Symbol table: Invalid NULL parameter while calling find method.");

  if (self->symbol == NULL)
    return NULL;

  int compRes = strcmp(self->key, key);

  // key is self
  if (compRes == 0)
    return self->symbol;

  // key is smaller than self key
  else if (compRes > 0 && self->left != NULL)
    return TSTNode_find(self->left, key);

  // key is greater than self key
  else if (self->right != NULL)
    return TSTNode_find(self->right, key);

  return NULL;
}

// Insert into tree new symbol with key ident, return pointer to that symbol NULL if this key exists
TSymbol TSTNode_insert(TSTNode self, char *key)
{
  if (self == NULL)
    apperr_runtimeError("Symbol table: Invalid NULL parameter while calling insert method.");

  // symbol is inserted in self when self is empty tree
  if (self->symbol == NULL)
    return TSTNode_newSymbol(self, key);

  // navigate throuth tree recursively
  int compRes = strcmp(self->key, key);

  #ifdef ST_DEBUG
  printf(" \"%s\" ", key);
  if (compRes == 0)
    printf("is equal to");
  else if (compRes > 0)
    printf("is smaller then");
  else
    printf("is bigger then");
  printf(" \"%s\"\n", self->key);
  #endif

  if (compRes == 0)                             // key is self
    return NULL;
  else if (compRes > 0 && self->left != NULL)   // key is smaller than self key
    return TSTNode_insert(self->left, key);
  else if (compRes < 0 && self->right != NULL)  // key is greater than self key
    return TSTNode_insert(self->right, key);

  // create record
  TSTNode newNode = TSTNode_create(key); // key string is copied here
  TSTNode_newSymbol(newNode, key);
  newNode->parent = self;

  // register record
  if (compRes > 0)          // key is smaller than self key
    self->left = newNode;
  else                      //  key is greater than self key
    self->right = newNode;

  TSTNode_balanceFromBottom(self);

  // return record
  return newNode->symbol;
}

// Insert into tree new symbol with key ident, return pointer to that symbol NULL if this key exists
TSymbol TSTNode_delete(TSTNode self, char *key);

// =============================================================================
// ====================== support function =====================================
// =============================================================================

// error if table is not initialized
void symbt_assertIfNotInit()
{
  if (GLBSymbTabStack == NULL)
    apperr_runtimeError("Symbol table is not initialized.");
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
    TSymbol foundSymb =  TSTNode_find(actTable, ident);
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

// Creates new symbol in top table frame.
TSymbol symbt_insertSymbOnTop(char *ident)
{
  symbt_assertIfNotInit();
  if (ident == NULL)
    return NULL;

  TSTNode topTab = GLBSymbTabStack->top(GLBSymbTabStack);
  TSymbol resSymb = TSTNode_insert(topTab, ident);
  GLBSymbTabStack->pop(GLBSymbTabStack);
  GLBSymbTabStack->push(GLBSymbTabStack, TSTNode_getRoot(topTab));
  return resSymb;
}


// Removes symbol first occurrence of symbol with
void symbt_deleteSymb(char *ident);


void symbt_print()
{
  TSTNode_print(GLBSymbTabStack->top(GLBSymbTabStack), 0);
}