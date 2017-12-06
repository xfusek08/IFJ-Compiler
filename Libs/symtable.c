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
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
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

// structure holding
typedef struct RedefSymb *TRedefSymb;
struct RedefSymb {
  TSymbol symbol;
  SymbolType origType;
  DataType origDataType;
};

// node of AVL tree main root with no parent reprezenting one symbol table
typedef struct SymTable *TSymTable;
struct SymTable {
  TSTNode root;               // root of table
  bool isTransparent;         // true if finding symbol is suppose to continue to lower table on stack (scope)
  bool isForLoop;             // flag if frame is for...next loop
  bool isDoLoop;              // flag if frame is do...loop loop
  char *frameLabel;           // label of frame
  unsigned int localLabelCnt; // counter of local labels
  TPStack redefStack;         // stack of redefined symbols to be pops and frame end
  TPStack definedIdentVars;   // stact of defined variable identifiers
  unsigned int tmpCnt;        // counter of generated temporaly symbols
};

// global internal instance of symbol table stack
TPStack GLBSymbTabStack;

// =============================================================================
// ====================== TRedefSymb implementation ==============================
// =============================================================================

// constructor of TRedefSymb
TRedefSymb TRedefSymb_create(TSymbol symb)
{
  TRedefSymb new = mmng_safeMalloc(sizeof(struct RedefSymb));
  new->symbol = symb;
  new->origType = symb->type;
  new->origDataType = symb->dataType;
  return new;
}

// destructor of TRedefSymb
void TRedefSymb_destroy(TRedefSymb self)
{
  mmng_safeFree(self);
}

// =============================================================================
// ====================== TRedefSymb implementation ==============================
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
  while (actArg->next != NULL && index-- > 0)
    actArg = actArg->next;
  return actArg;
}

TArgument TArgList_insert(TArgList self, char *ident, DataType dataType)
{
  TArgument newArg = TArgument_create(ident, dataType);
  if (self->tail != NULL)
    self->tail->next = newArg;
  else
    self->head = newArg;
  self->tail = newArg;
  self->count++;
  return newArg;
}

bool TArgList_equals(TArgList list1, TArgList list2)
{
  if (list1 == list2)
    return true;
  if (list1->count != list2->count)
    return false;

  TArgument arg1 = list1->head;
  TArgument arg2 = list2->head;

  while(arg1 != NULL || arg2 != NULL)
  {
    if (arg1->dataType != arg2->dataType)
      return false;
    arg1 = arg1->next;
    arg2 = arg2->next;
  }
  return arg1 == arg2; // both has to be NULL
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
  newArgList->equals = TArgList_equals;
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
  newSymb->ident = util_StrHardCopy(ident);
  newSymb->key = ident;
  newSymb->type = symtUnknown;
  newSymb->dataType = dtUnspecified;
  newSymb->isTemp = false;
  return newSymb;
}

// destructor of TSymbol
void TSymbol_destroy(TSymbol self)
{
  if (self != NULL)
  {
    if (self->type == symtFuction)
    {
      if (self->data.funcData.arguments != NULL)
        TArgList_destroy(self->data.funcData.arguments);
      if (self->data.funcData.label != NULL)
        mmng_safeFree(self->data.funcData.label);
    }
    else if (self->type == symtConstant && self->dataType == dtString)
    {
      if (self->data.stringVal != NULL)
        mmng_safeFree(self->data.stringVal);
    }
    mmng_safeFree(self->ident);
    mmng_safeFree(self);
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
  fprintf(stderr, "RotRight: %s\n", self->key);
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
  fprintf(stderr, "RotLeft: %s\n", self->key);
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
    fprintf(stderr, "Balancing: %s\n", node->key);
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

// Finds node with corresponding key, NULL if not found
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
  fprintf(stderr, " \"%s\" ", key);
  if (compRes == 0)
    fprintf(stderr, "is equal to");
  else if (compRes > 0)
    fprintf(stderr, "is smaller then");
  else
    fprintf(stderr, "is bigger then");
  fprintf(stderr, " \"%s\"\n", self->key);
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
    return self;

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

    if (toDelNode->parent != NULL)
    {
      if (toDelNode->parent->left == toDelNode)
        toDelNode->parent->left = kritNode;
      else
        toDelNode->parent->right = kritNode;
    }
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

// fills stack witl in order symbols
void TSTNode_inOrder(TSTNode self, TPStack nodeStack)
{
  if (nodeStack == NULL)
    return;
  if (self->left != NULL)
    TSTNode_inOrder(self->left, nodeStack);
  nodeStack->push(nodeStack, self);
  if (self->right != NULL)
    TSTNode_inOrder(self->right, nodeStack);
}

// =============================================================================
// ====================== TSymTable implementation =============================
// =============================================================================

// constructor of TSymTable
TSymTable TSymTable_create(char *frameLabel, bool transparent, bool isForLoop, bool isDoLoop)
{
  if (isForLoop && isDoLoop)
    apperr_runtimeError("Frame cannot be both, do and for loop.");

  TSymTable newST = (TSymTable)mmng_safeMalloc(sizeof(struct SymTable));
  newST->isTransparent = transparent;
  newST->root = NULL;
  newST->frameLabel = util_StrHardCopy(frameLabel);
  newST->localLabelCnt = 0;
  newST->isForLoop = isForLoop;
  newST->isDoLoop = isDoLoop;
  newST->redefStack = TPStack_create();
  newST->definedIdentVars = TPStack_create();
  newST->tmpCnt = 0;
  return newST;
}

// destructor of TSymTable
void TSymTable_destroy(TSymTable self)
{
  while (self->definedIdentVars->count > 0)
  {
    mmng_safeFree(self->definedIdentVars->top(self->definedIdentVars));
    self->definedIdentVars->pop(self->definedIdentVars);
  }
  self->definedIdentVars->destroy(self->definedIdentVars);
  self->redefStack->destroy(self->redefStack);
  TSTNode_destroy(self->root, true);
  mmng_safeFree(self->frameLabel);
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

TSymTable getFirstNonTransparetFrame()
{
  int i = GLBSymbTabStack->count - 1;
  TSymTable actTable = GLBSymbTabStack->ptArray[i];
  for(;actTable->isTransparent && i >= 0; i--) // serach first non transparent frame
    actTable = GLBSymbTabStack->ptArray[i];
  return actTable;
}

void deleteTempSymbols()
{
  int i = GLBSymbTabStack->count - 1;
  TPStack nodeStack = TPStack_create();
  while(i >= 0) // serach firs
  {
    TSymTable actTable = GLBSymbTabStack->ptArray[i];
    TSTNode rootNode = actTable->root;
    if (rootNode != NULL)
    {
      TSTNode_inOrder(rootNode, nodeStack);
      while (nodeStack->count > 0)
      {
        TSTNode actNode = nodeStack->top(nodeStack);
        nodeStack->pop(nodeStack);
        if (actNode->symbol != NULL)
          if (actNode->symbol->isTemp)
            TSymTable_detete(actTable, actNode->key);
      }
    }
    i--;
    if (!actTable->isTransparent)
      break;
  }
  nodeStack->destroy(nodeStack);
}

// =============================================================================
// ====================== Interface implementation =============================
// =============================================================================

// Initialization of global symbol table stack
void symbt_init(char *mainLabel)
{
  if (GLBSymbTabStack != NULL)
    apperr_runtimeError("symbt_init(): Symbol table is already initialized.");

  GLBSymbTabStack = TPStack_create();
  symbt_pushFrame(mainLabel, false, false, false); // insert global frame
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
void symbt_pushFrame(char *label, bool transparent, bool isForLoop, bool isDoLoop)
{
  symbt_assertIfNotInit();
  deleteTempSymbols();
  printInstruction("CREATEFRAME\n");
  GLBSymbTabStack->push(GLBSymbTabStack, TSymTable_create(label, transparent, isForLoop, isDoLoop));
}

// Frees destroys symbol table on top of the stack.
void symbt_popFrame()
{
  symbt_assertIfNotInit();
  if (GLBSymbTabStack->count > 1)
  {
    TSymTable table = GLBSymbTabStack->top(GLBSymbTabStack);
    while (table->redefStack->count > 0)
    {
      TRedefSymb redefSymb = table->redefStack->top(table->redefStack);
      table->redefStack->pop(table->redefStack);
      redefSymb->symbol->type = redefSymb->origType;
      redefSymb->symbol->dataType = redefSymb->origDataType;
      if (redefSymb->symbol->type == symtVariable)
        printInstruction("POPS %s\n", redefSymb->symbol->ident);
    }
    if (!table->isTransparent)
      flushCode();
    GLBSymbTabStack->pop(GLBSymbTabStack);
    TSymTable_destroy(table);
    printInstruction("CREATEFRAME\n");
    deleteTempSymbols();
  }
}

// Count frames
int symbt_cntFuncFrames()
{
  symbt_assertIfNotInit();
  int cnt = 0;
  for(int i = GLBSymbTabStack->count - 1; i >= 0; i--) // serach first non transparent frame
  {
    TSymTable actTable = GLBSymbTabStack->ptArray[i];
    if (!actTable->isTransparent)
      cnt++;
  }
  return cnt;
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

// Gets label of first non-transparent frame from top of frame stack (used as function label)
char *symbt_getActFuncLabel()
{
  symbt_assertIfNotInit();
  return getFirstNonTransparetFrame()->frameLabel;
}

// Gets label of N-th for ... nest frame from top of frame stack (used for exit and continue)
char *symbt_getNthForLoopLabel(int N)
{
  symbt_assertIfNotInit();
  int i = GLBSymbTabStack->count - 1;
  TSymTable actTable = GLBSymbTabStack->ptArray[i];
  for(; N >= 0 && i >= 0; i--) // serach first N-th loop frame
  {
    actTable = GLBSymbTabStack->ptArray[i];
    if (actTable->isForLoop)
      N--;
  }
  if (!actTable->isForLoop)
    return NULL;
  return actTable->frameLabel;
}

// Gets label of N-th do ... loop frame from top of frame stack (used for exit and continue)
char *symbt_getNthDoLoopLabel(int N)
{
  symbt_assertIfNotInit();
  int i = GLBSymbTabStack->count - 1;
  TSymTable actTable = GLBSymbTabStack->ptArray[i];
  for(; N >= 0 && i >= 0; i--) // serach first N-th loop frame
  {
    actTable = GLBSymbTabStack->ptArray[i];
    if (actTable->isDoLoop)
      N--;
  }
  if (!actTable->isDoLoop)
    return NULL;
  return actTable->frameLabel;
}

// Gets label of actual frame on top of frame stack
char *symbt_getActLocalLabel()
{
  symbt_assertIfNotInit();
  return ((TSymTable)(GLBSymbTabStack->top(GLBSymbTabStack)))->frameLabel;
}

// Gets new unique label for actual FunctionLabel
char *symbt_getNewLocalLabel()
{
  symbt_assertIfNotInit();
  TSymTable actTable = getFirstNonTransparetFrame();
  char *cntString = mmng_safeMalloc(
    sizeof(char) * (strlen(actTable->frameLabel) + 12));
  sprintf(cntString, "%s$L%u", actTable->frameLabel, actTable->localLabelCnt++);
  return cntString;
}

// Push value of variable symbol on datastack and remembers it for frame destroing
bool symbt_pushRedefinition(TSymbol symbol)
{
  symbt_assertIfNotInit();
  if (symbol == NULL)
    apperr_runtimeError("Redefined symbol is NULL.");
  if (symbol->type == symtConstant)
    apperr_runtimeError("Cannot redefine constant.");

  TSymTable actTable = GLBSymbTabStack->top(GLBSymbTabStack);
  // check if variable already exist in current frame
  if (TSymTable_find(actTable, symbol->key) != NULL)
    return false;
  actTable->redefStack->push(actTable->redefStack, TRedefSymb_create(symbol));
  if (symbol->type == symtVariable)
    printInstruction("PUSHS %s\n", symbol->ident);
  return true;
}

// adds identifier of variable to fucntion frame
void symbt_defVarIdent(char *varIdent)
{
  symbt_assertIfNotInit();
  TSymTable table = getFirstNonTransparetFrame();
  table->definedIdentVars->push(table->definedIdentVars, util_StrHardCopy(varIdent));
}

// checks if identifier already exists in function frame
bool symbt_isVarDefined(char *varIdent)
{
  symbt_assertIfNotInit();
  TSymTable table = getFirstNonTransparetFrame();
  for (int i = 0; i < table->definedIdentVars->count; i++)
    if (strcmp(table->definedIdentVars->ptArray[i], varIdent) == 0)
      return true;
  return false;
}

// checks if allfucntion symbols on the ground frame are defined.
char *symbt_getUndefinedFunc()
{
  symbt_assertIfNotInit();
  TSymTable globalTable = GLBSymbTabStack->ptArray[0]; // gets ground (global) frame
  TPStack nodeStack = TPStack_create();
  TSTNode_inOrder(globalTable->root, nodeStack);
  char *result = NULL;
  while(nodeStack->count > 0)
  {
    if (result == NULL) // if we already know that result exists
    {
      TSymbol actSymb = ((TSTNode)(nodeStack->top(nodeStack)))->symbol;
      if (actSymb != NULL)
        if (actSymb->type == symtFuction)
          if (!actSymb->data.funcData.isDefined)
            result = actSymb->key;
    }
    nodeStack->pop(nodeStack);
  }
  nodeStack->destroy(nodeStack);
  return result;
}

/**
  * Generates and returns new temp symbol on top frame with unique identifier
  */
TSymbol symbt_getUniqeTmpSymb()
{
  symbt_assertIfNotInit();
  TSymTable actTable = GLBSymbTabStack->top(GLBSymbTabStack);
  TSymbol res = NULL;
  char *cntString = mmng_safeMalloc(sizeof(char) * 16);
  while(res == NULL)
  {
    sprintf(cntString, "%%tmp%u", actTable->tmpCnt);
    res = symbt_insertSymbOnTop(cntString);
    actTable->tmpCnt++;
  }
  res->isTemp = true;
  mmng_safeFree(cntString);
  return res;
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
        fprintf(stderr, " •--");
      else
        fprintf(stderr, "    ");
    }
    fprintf(stderr, "[%s]\n", node->key);
    if (node->left != NULL)
      TSTNode_print(node->left, depht + 1);
  }
}

// Prints top table as binary tree into stdout
void symbt_print()
{
  for (int i = GLBSymbTabStack->count - 1; i >= 0; i--)
  {
    TSymTable table = GLBSymbTabStack->ptArray[i];
    fprintf(stderr, "\nSymbol table [%s]\n\n", table->frameLabel);
    TSTNode_print(table->root, 0);
    fprintf(stderr, "\n-------------------------------------------------------------------\n");
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
  fprintf(stderr, "Symbol: %p\n", symbol);
  fprintf(stderr, "  identifier:   %s %p\n", symbol->ident, symbol->ident);
  fprintf(stderr, "  key:          %s %p\n", symbol->key, symbol->key);
  fprintf(stderr, "  type:         %s\n", stype);
  fprintf(stderr, "  data type:    %s\n", util_dataTypeToString(symbol->dataType));
  fprintf(stderr, "  data: ");
  switch(symbol->type)
  {
    case symtUnknown:
    case symtVariable:
      fprintf(stderr, "NULL\n");
      break;
    case symtConstant:
      switch(symbol->dataType)
      {
        case dtUnspecified: fprintf(stderr, "NULL\n"); break;
        case dtInt: fprintf(stderr, "%d (integer) \n", symbol->data.intVal); break;
        case dtFloat: fprintf(stderr, "%lf (double)\n", symbol->data.doubleVal); break;
        case dtString: fprintf(stderr, "\"%s\" (string) %p\n", symbol->data.stringVal, symbol->data.stringVal); break;
        case dtBool: fprintf(stderr, "%s (boolean)\n", (symbol->data.boolVal) ? "True" : "False"); break;
      }
      break;
    case symtFuction:
      fprintf(stderr, " Function:\n");
      TArgList arguments = symbol->data.funcData.arguments;
      fprintf(stderr, "\n    Label: \"%s\"\n", symbol->data.funcData.label);
      fprintf(stderr, "    Return type: %s\n", util_dataTypeToString(symbol->data.funcData.returnType));
      fprintf(stderr, "    Defined: %s\n", (symbol->data.funcData.isDefined) ? "True" : "False");
      fprintf(stderr, "    Argument count: %d\n", arguments->count);
      fprintf(stderr, "    Arguments: [");
      TArgument arg = arguments->get(arguments, 0);
      for (int i = 0; i < arguments->count && arg != NULL; i++)
      {
        arg = arguments->get(arguments, i);
        fprintf(stderr, "(%s as %s)", arg->ident, util_dataTypeToString(arg->dataType));
        if (i + 1 < arguments->count)
          fprintf(stderr, "; ");
      }
      fprintf(stderr, "]\n");
      break;
  }
}
#endif // DEBUG
