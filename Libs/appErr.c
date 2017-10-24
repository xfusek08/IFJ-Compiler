#include "appErr.h"
#include <stdlib.h>
#include <stdio.h>

void apperr_codeError(ErrType type, int row, int col, char *line)
{
  fprintf(stderr, "Your code is shit! Deal with it! %d%d%d%s", type, row, col, line);
  exit(1);
}

void apperr_runtimeError(char *errMsg)
{
  fprintf(stderr, "Our code is shit! Deal with it! %s", errMsg);
  exit(1);
}
