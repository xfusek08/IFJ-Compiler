
#include "utils.h"
#include "grammar.h"
#include "appErr.h"

void scanner_init();

SToken scan_GetNextToken();

void scan_raiseCodeError(ErrType typchyby, char *message);

void scanner_destroy();
