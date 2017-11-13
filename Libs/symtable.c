/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    symtable.c
 * \brief   Symbol table implementation
 * \author  Petr Fusek (xfusek08)
 * \date    10.11.2017 - Petr Fusek
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
  if (ident == NULL)
    apperr_runtimeError("Symbol table: Invalid NULL parameter while creating symbol.");

  TSymbol newSymb = (TSymbol)mmng_safeMalloc(sizeof(struct Symbol));
  newSymb->ident = ident;
  newSymb->type = symtUnknown;
  newSymb->dataType = dtUnspecified;
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
  if (key == NULL)
    apperr_runtimeError("Symbol table: Invalid NULL parameter while creating tableNode.");

  TSTNode newNode = (TSTNode)mmng_safeMalloc(sizeof(struct STNode));
  newNode->balance = 0;
  newNode->key = util_StrHardCopy(key); // new hard copy of string
  newNode->symbol = TSymbol_create(key);
  newNode->parent = NULL;
  newNode->left = NULL;
  newNode->right = NULL;
  return newNode;
}

// destructor of TSTNode
void TSTNode_destroy(TSTNode node, bool recursively)
{
  if (node == NULL)
    return;

  if (recursively)
  {
    // delete subtrees recursively
    if (node->left != NULL)
      TSTNode_destroy(node->left, true);
    if (node->right != NULL)
      TSTNode_destroy(node->right, true);
  }

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
  if (self != NULL)
  {
    while (self->parent != NULL)
      self = self->parent;
  }
  return self;
}

// operation RR
void TSTNode_rotateRight(TSTNode self)
{
  if (self == NULL || self->left == NULL)
    return;

  #ifdef ST_DEBUG
  printf("RotRight: %s\n", self->key);
  #endif // ST_DEBUG

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

  #ifdef ST_DEBUG
  printf("RotLeft: %s\n", self->key);
  #endif // ST_DEBUG

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


// methode starts from given node in tree and balance tree up to root node
void TSTNode_balanceFromBottom(TSTNode node)
{
  int balanceFactor = 0;
  int balanceFactorpPrev = 0;
  while (node != NULL)
  {
    #ifdef ST_DEBUG
    printf("Balancing: %s\n", node->key);
    #endif // ST_DEBUG

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
TSTNode TSTNode_find(TSTNode self, char *key)
{
  if (self == NULL || key == NULL)
    apperr_runtimeError("Symbol table: NULL parameter while calling find method.");

  int compRes = strcmp(self->key, key);

  // key is self
  if (compRes == 0)
    return self;

  // key is smaller than self key
  else if (compRes > 0 && self->left != NULL)
    return TSTNode_find(self->left, key);

  // key is greater than self key
  else if (self->right != NULL)
    return TSTNode_find(self->right, key);

  return NULL;
}

// Insert into tree new node with key ident, return pointer to that node NULL if this key already exists
TSTNode TSTNode_insert(TSTNode self, char *key)
{
  if (self == NULL || key == NULL)
    apperr_runtimeError("Symbol table: NULL parameter while calling insert method.");

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
  #endif // ST_DEBUG

  if (compRes == 0)                             // key is self
    return NULL;
  else if (compRes > 0 && self->left != NULL)   // key is smaller than self key
    return TSTNode_insert(self->left, key);
  else if (compRes < 0 && self->right != NULL)  // key is greater than self key
    return TSTNode_insert(self->right, key);

  // create record
  TSTNode newNode = TSTNode_create(key); // key string is copied here
  newNode->parent = self;

  // register record
  if (compRes > 0)          // key is smaller than self key
    self->left = newNode;
  else                      //  key is greater than self key
    self->right = newNode;

  TSTNode_balanceFromBottom(self);

  // return record
  return newNode;
}

// Insert into tree new symbol with key ident, return pointer to that symbol NULL if this key exists
// Returns self, if self is deleted, returns new root of whole tree
TSTNode TSTNode_delete(TSTNode self, char *key)
{
  if (self == NULL || key == NULL)
    apperr_runtimeError("Symbol table: NULL parameter while calling node delete method.");

  // find node to be delete
  TSTNode toDelNode = TSTNode_find(self, key);

  if (toDelNode == NULL)
    return NULL;

  TSTNode kritNode = NULL;

  // 1) Node is terminating
  if (toDelNode->right == NULL && toDelNode->left == NULL)
  {
    kritNode = toDelNode->parent;
    if (kritNode != NULL)
    {
      if (kritNode->left == toDelNode)
        kritNode->left = NULL;
      else
        kritNode->right = NULL;
    }
  }
  // 2) node has only one child
  else if (toDelNode->right == NULL || toDelNode->left == NULL)
  {
    // choose existing child as kritical node
    if (toDelNode->right == NULL)
      kritNode = toDelNode->left;
    else
      kritNode = toDelNode->right;

    kritNode->parent = toDelNode->parent;

    if (toDelNode->parent->left == toDelNode)
      toDelNode->parent->left = kritNode;
    else
      toDelNode->parent->right = kritNode;
  }
  // 3) node has both children
  else
  {
    // lets find replacement in mostlef node in right child
    TSTNode replacementNode = toDelNode->right;
    while (replacementNode->left != NULL)
      replacementNode = replacementNode->left;

    // specify kritical node
    // special case is when replacement is direct child of toDelNode
    if (replacementNode == toDelNode->right)
    {
      kritNode = replacementNode;
    }
    else
    {
      // kritnode for balancing is parent of replacementNode
      kritNode = replacementNode->parent;
      kritNode->left = replacementNode->right;  // we know that replacement doesnt have left child but there may be on right
      if (kritNode->left != NULL)
        kritNode->left->parent = kritNode;

      // replacement inherits its right child only if it is not direct child of toDelNode
      replacementNode->right = toDelNode->right;
      replacementNode->right->parent = replacementNode;
    }

    // replacement inherites position in tree from to del node
    replacementNode->left = toDelNode->left;
    replacementNode->parent = toDelNode->parent;
    replacementNode->left->parent = replacementNode;

    // fix parent of replacement
    if (toDelNode->parent != NULL)
    {
      if (toDelNode->parent->left == toDelNode)
        toDelNode->parent->left = replacementNode;
      else
        toDelNode->parent->right = replacementNode;
    }
  }

  // destroy node non recursively (keep subtrees)
  TSTNode_destroy(toDelNode, false);
  TSTNode_balanceFromBottom(kritNode);
  return TSTNode_getRoot(kritNode);
}

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
  // make sure nothing stays in stack
  while (!(GLBSymbTabStack->count == 1 && GLBSymbTabStack->top(GLBSymbTabStack) == NULL))
    symbt_popFrame();

  // delete last left table on stack (pop left null)
  GLBSymbTabStack->pop(GLBSymbTabStack);
  GLBSymbTabStack->destroy(GLBSymbTabStack); // destroy stack
  GLBSymbTabStack = NULL; // null global reference
}

// Creates new instance of symbol table on top of the stack
void symbt_pushFrame()
{
  symbt_assertIfNotInit();
  GLBSymbTabStack->push(GLBSymbTabStack, NULL);
}

// Frees destroys symbol table on top of the stack.
void symbt_popFrame()
{
  symbt_assertIfNotInit();
  TSTNode actST = GLBSymbTabStack->top(GLBSymbTabStack);
  TSTNode_destroy(actST, true);
  if (GLBSymbTabStack->count > 1)
    GLBSymbTabStack->pop(GLBSymbTabStack);
  else
    GLBSymbTabStack->ptArray[0] = NULL;
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
    apperr_runtimeError("Symbol table: NULL identifier while calling delete symbol method.");

  // searching from top of the stack
  for (int i = GLBSymbTabStack->count - 1; i >= 0; i--)
  {
    // we storimg pointers on TSTNode NULL in stack means empty table but in existing frame
    TSTNode actTable = GLBSymbTabStack->ptArray[i];
    if (actTable != NULL)
    {
      TSTNode foundNode = TSTNode_find(actTable, ident);
      if (foundNode != NULL)
        return foundNode->symbol;
    }
  }
  return NULL;
}

// Finds symbol by identifier or creates new symbol if it's not found.
TSymbol symbt_findOrInsertSymb(char *ident)
{
  symbt_assertIfNotInit();
  if (ident == NULL)
    apperr_runtimeError("Symbol table: NULL identifier while calling delete symbol method.");

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
    apperr_runtimeError("Symbol table: NULL identifier while calling delete symbol method.");

  TSymbol resSymb = NULL;
  TSTNode topTab = GLBSymbTabStack->top(GLBSymbTabStack);
  if (topTab == NULL)
  {
    topTab = TSTNode_create(ident); // symbol is also created in this constructor
    resSymb = topTab->symbol;
  }
  else
    resSymb = TSTNode_insert(topTab, ident)->symbol;

  // make sure that pointer on top of table stact is pointin on to root of tree
  GLBSymbTabStack->ptArray[GLBSymbTabStack->count - 1] = TSTNode_getRoot(topTab);
  return resSymb;
}

// Removes first occurrence of symbol from top of stack with given identifier
void symbt_deleteSymb(char *ident)
{
  symbt_assertIfNotInit();
  if (ident == NULL)
    apperr_runtimeError("Symbol table: NULL identifier while calling delete symbol method.");

  // searching from top of the stack
  for (int i = GLBSymbTabStack->count - 1; i >= 0; i--)
  {
    // we storimg pointers on TSTNode NULL in stack means empty table but in existing frame
    TSTNode actTable = GLBSymbTabStack->ptArray[i];
    if (actTable != NULL)
    {
      // Delete function returns new root of tree because order of nodes could be changed for balance
      // and act root node could be deleted of shifted deeper into the tree
      // or node on top could be only one in table in that case null is returned.
      GLBSymbTabStack->ptArray[i] = TSTNode_delete(actTable, ident);
      return;
    }
  }
}

// =============================================================================
// ====================== funkce pro testovaci programy ========================
// =============================================================================

#ifdef DEBUG

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

// Prints top table as binary tree into stdout
void symbt_print()
{
  TSTNode_print(GLBSymbTabStack->top(GLBSymbTabStack), 0);
}

// Prints instance of TSymbol into stdout
void symbt_printSymb(TSymbol symbol)
{
  char *stype;
  switch(symbol->type)
  {
    case symtUnknown:     stype = "unspecified"; break;
    case symtFuction:     stype = "function"; break;
    case symtInt:         stype = "integer variable"; break;
    case symtFloat:       stype = "floating point variable "; break;
    case symtString:      stype = "string variable"; break;
    case symtBool:        stype = "boolean variable"; break;
    case symtConstInt:    stype = "integer constant"; break;
    case symtConstDouble: stype = "floating point constant"; break;
    case symtConstString: stype = "string constant"; break;
    case symtConstBool:   stype = "boolean constant"; break;
  }
  printf("Symbol: %p\n", symbol);
  printf("  identifier:   %s\n", symbol->ident);
  printf("  type:         %s\n", stype);
  printf("  data:\n");
  printf("    integer value:  %d\n", symbol->value.intVal);
  printf("    double value:   %lf\n", symbol->value.doubleVal);
  printf("    string value:   %s\n", symbol->value.stringVal);
  printf("    bool value:     %s\n", (symbol->value.boolVal) ? "True" : "False");
}
#endif // DEBUG
