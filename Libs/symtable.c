/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    symtable.c
 * \brief   Symbol table implementation
 * \author  Petr Fusek (xfusek08)
 * \date    21.11.2017 - Petr Fusek
 */
/******************************************************************************/

// references
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

// node of AVL tree main root with no parent reprezenting one symbol table
typedef struct SymTable *TSymTable;
struct SymTable {
  TSTNode root;         // root of table
  bool isTransparent;   // true if finding symbol is suppose to continue to lower table on stack (scope)
};

// global internal instance of symbol table stack
TPStack GLBSymbTabStack;

// =============================================================================
// ====================== TArgument implementation ==============================
// =============================================================================

// constructor of TArgList
TArgument TArgument_create(const char *ident, DataType dataType)
{
  TArgument newArg = (TArgument)mmng_safeMalloc(sizeof(struct Argument));
  newArg->ident = util_StrHardCopy(ident);
  newArg->dataType = dataType;
  newArg->next = NULL;
  return newArg;
}

// destructor of TArgument
void TArgument_destroy(TArgument self)
{
  mmng_safeFree(self->ident);
  mmng_safeFree(self);
}

// =============================================================================
// ====================== TArgList implementation ==============================
// =============================================================================

// destructor of TArgList
void TArgList_destroy(TArgList self)
{
  TArgument actArg = self->head;
  while (actArg != NULL)
  {
    TArgument toDelArg = actArg;
    actArg = actArg->next;
    TArgument_destroy(toDelArg);
  }
  mmng_safeFree(self);
}

TArgument TArgList_get(TArgList self, int index)
{
  TArgument actArg = self->head;
  for (; index >= 0 && actArg != NULL; index--)
    actArg = actArg->next;
  return actArg;
}

TArgument TArgList_insert(TArgList self, const char *ident, DataType dataType)
{
  TArgument newArg = TArgument_create(ident, dataType);
  self->tail->next = newArg;
  self->tail = newArg;
  self->count++;
  return newArg;
}

// constructor of TArgList
TArgList TArgList_create()
{
  TArgList newArgList = (TArgList)mmng_safeMalloc(sizeof(struct ArgList));
  newArgList->count = 0;
  newArgList->head = NULL;
  newArgList->tail = NULL;
  newArgList->get = TArgList_get;
  newArgList->insert = TArgList_insert;
  return newArgList;
}

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
  {
    if (symbol->type == symtFuction)
    {
      if (symbol->data.funcData.arguments != NULL)
        TArgList_destroy(symbol->data.funcData.arguments);
      if (symbol->data.funcData.label != NULL)
        mmng_safeFree(symbol->data.funcData.label);
    }
    else if (symbol->type == symtConstant || symbol->dataType == dtString)
    {
      if (symbol->data.stringVal != NULL)
        mmng_safeFree(symbol->data.stringVal);
    }
    mmng_safeFree(symbol);
  }
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
  newNode->symbol = TSymbol_create(newNode->key);
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

  // delete subtrees recursively
  if (recursively)
  {
    TSTNode_destroy(node->left, true);
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
  int height_left = 1 + TSTNode_height(self->right);
  int height_right = 1 + TSTNode_height(self->left);
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
  if (key == NULL)
    apperr_runtimeError("Symbol table: NULL parameter while calling find method.");

  if (self == NULL)
    return NULL;

  int compRes = strcmp(self->key, key);

  // key is self
  if (compRes == 0)
    return self;
  else if (compRes > 0) // key is smaller than self key
    return TSTNode_find(self->left, key);
  else  // key is greater than self key
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

// Delete node with key ident
// Returns self, but if self is deleted, returns new root of whole tree
TSTNode TSTNode_delete(TSTNode self, char *key, bool *deleted)
{
  if (self == NULL || key == NULL)
    apperr_runtimeError("Symbol table: NULL parameter while calling TSTNode_delete().");

  // find node to be delete
  TSTNode toDelNode = TSTNode_find(self, key);

  if (deleted != NULL)
    *deleted = false;

  if (toDelNode == NULL)
  {
    return self;
  }

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
  if (deleted != NULL)
    *deleted = true;
  return TSTNode_getRoot(kritNode);
}

// =============================================================================
// ====================== TSymTable implementation =============================
// =============================================================================

// constructor of TSymTable
TSymTable TSymTable_create(bool transparent)
{
  TSymTable newST = (TSymTable)mmng_safeMalloc(sizeof(struct SymTable));
  newST->isTransparent = transparent;
  newST->root = NULL;
  return newST;
}

// destructor of TSymTable
void TSymTable_destroy(TSymTable self)
{
  TSTNode_destroy(self->root, true);
  mmng_safeFree(self);
}

// finds symbol by identifier, NULL if not found
TSymbol TSymTable_find(TSymTable self, char *ident)
{
  TSTNode resNode = TSTNode_find(self->root, ident);
  if (resNode != NULL)
    return resNode->symbol;
  return NULL;
}

// inserts symbol with identifier, NULL if identifier exists
TSymbol TSymTable_insert(TSymTable self, char *ident)
{
  if (self->root == NULL)
  {
    self->root = TSTNode_create(ident);
    return self->root->symbol;
  }

  TSTNode newNode = TSTNode_insert(self->root, ident);
  if (newNode == NULL)
    return NULL;
  self->root = TSTNode_getRoot(newNode);
  return newNode->symbol;
}

// deletes symbol witch given identifier
void TSymTable_detete(TSymTable self, char *ident)
{
  if (self->root != NULL)
    self->root = TSTNode_delete(self->root, ident, NULL);
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
  symbt_pushFrame(false); // insert global frame
}

//  Free of all symbol table stack
void symbt_destroy()
{
  symbt_assertIfNotInit();
  // make sure nothing stays in stack
  while (GLBSymbTabStack->count > 1)
    symbt_popFrame();

  // delete last left global table on stack (pop left null)
  TSymTable_destroy(GLBSymbTabStack->top(GLBSymbTabStack));
  // destroy stack
  GLBSymbTabStack->pop(GLBSymbTabStack);
  GLBSymbTabStack->destroy(GLBSymbTabStack); // destroy stack
  GLBSymbTabStack = NULL; // null global reference
}

// Creates new instance of symbol table on top of the stack
void symbt_pushFrame(bool transparent)
{
  symbt_assertIfNotInit();
  GLBSymbTabStack->push(GLBSymbTabStack, TSymTable_create(transparent));
}

// Frees destroys symbol table on top of the stack.
void symbt_popFrame()
{
  symbt_assertIfNotInit();
  TSymTable_destroy(GLBSymbTabStack->top(GLBSymbTabStack));
  GLBSymbTabStack->pop(GLBSymbTabStack);
  if (GLBSymbTabStack->count == 0) // if nothing left, push global frame
    symbt_pushFrame(false);
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
    apperr_runtimeError("Symbol table: NULL identifier while calling symbt_findSymb().");


  // load act table on top
  TSymTable actTable = GLBSymbTabStack->top(GLBSymbTabStack);
  TSymbol foundSymb = TSymTable_find(actTable, ident);
  if (foundSymb != NULL)
    return foundSymb;

  // searching from top until searched table is not transparent or next on stack is global
  for (int i = GLBSymbTabStack->count - 2; i > 0 && actTable->isTransparent; i--)
  {
    actTable = GLBSymbTabStack->ptArray[i];
    foundSymb = TSymTable_find(actTable, ident);
    if (foundSymb != NULL)
      return foundSymb;
  }
  // search global table
  return TSymTable_find(GLBSymbTabStack->ptArray[0], ident);
}

// Finds symbol by identifier or creates new symbol if it's not found.
TSymbol symbt_findOrInsertSymb(char *ident)
{
  symbt_assertIfNotInit();
  if (ident == NULL)
    apperr_runtimeError("Symbol table: NULL identifier while calling symbt_findOrInsertSymb().");

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
    apperr_runtimeError("Symbol table: NULL identifier while calling symbt_insertSymbOnTop().");

  return TSymTable_insert(GLBSymbTabStack->top(GLBSymbTabStack), ident);
}

// Removes symbol from top table in stack with identifier ident
void symbt_deleteSymb(char *ident)
{
  symbt_assertIfNotInit();
  if (ident == NULL)
    apperr_runtimeError("Symbol table: NULL identifier while calling symbt_deleteSymb().");

  TSymTable_detete(GLBSymbTabStack->top(GLBSymbTabStack), ident);
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
  for (int i = GLBSymbTabStack->count - 1; i >= 0; i--)
  {
    printf("\nSymbol table [%d] -----------------------------------------------\n\n", i);
    TSTNode_print(((TSymTable)GLBSymbTabStack->ptArray[i])->root, 0);
  }
}

// Prints instance of TSymbol into stdout
void symbt_printSymb(TSymbol symbol)
{
  char *stype;
  switch(symbol->type)
  {
    case symtUnknown:  stype = "unspecified"; break;
    case symtFuction:  stype = "function"; break;
    case symtVariable: stype = "variable"; break;
    case symtConstant: stype = "constant"; break;
  }
  printf("Symbol: %p\n", symbol);
  printf("  identifier:   %s\n", symbol->ident);
  printf("  type:         %s\n", stype);
  printf("  data type:    %s\n", util_dataTypeToString(symbol->dataType));
  printf("  data: ");
  switch(symbol->type)
  {
    case symtUnknown:
    case symtVariable:
      printf("NULL\n");
      break;
    case symtConstant:
      switch(symbol->dataType)
      {
        case dtUnspecified: printf("NULL\n"); break;
        case dtInt: printf("%d (integer) \n", symbol->data.intVal); break;
        case dtFloat: printf("%lf (double)\n", symbol->data.doubleVal); break;
        case dtString: printf("\"%s\" (string)\n", symbol->data.stringVal); break;
        case dtBool: printf("%s (boolean)\n", (symbol->data.boolVal) ? "True" : "False"); break;
      }
      break;
    case symtFuction:
      printf(" Function:\n");
      printf("\n    Label: \"%s\"\n", symbol->data.funcData.label);
      printf("    return type: %s\n", util_dataTypeToString(symbol->data.funcData.returnType));
      printf("    Defined: %s\n", (symbol->data.funcData.isDefined) ? "True" : "False");
      printf("    Argument count: %d\n", symbol->data.funcData.arguments->count);
      printf("    Arguments: [");
      TArgList arguments = symbol->data.funcData.arguments;
      for (int i = 0; i < arguments->count; i++)
      {
        printf("(%s as %s)",
          arguments->get(arguments, i)->ident,
          util_dataTypeToString(arguments->get(arguments, i)->dataType));
        if (i + 1 < arguments->count)
          printf("; ");
      }
      printf("]\n");
      break;
  }
}
#endif // DEBUG
