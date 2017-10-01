
#include <stdio.h>
#include <stdlib.h>
#include "Libs/MMng.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();

  // ... spousta mallocu a frees ... 

  mmng_freeAll();
  return 0;
}
