
#include <stdio.h>
#include "new.h"

unsigned refsCount = 0;
refMap refNames[0];

int main(int argc, char *argv[]) {
  hvmInit(1024);

  // TODO: add tests here

  hvmFree();
  return 0;
}
