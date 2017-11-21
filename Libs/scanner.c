
#include <stdlib.h>
#include "scanner.h"

void scanner_init() {  }

SToken scan_GetNextToken() { SToken tk; return tk; }

void scan_raiseCodeError(ErrType typchyby, char *message)
{
  (void)typchyby;
  (void)message;
}

void scanner_destroy() {  }
