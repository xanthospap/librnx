#include "doris_rinex.hpp"
#include <cstdio>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

using namespace dso;

int main(int argc, char *argv[]) {
  if (argc!=2) {
    fprintf(stderr, "Error. Usage %s [DORIS RINEX]\n", argv[0]);
    return 1;
  }

  DorisObsRinex rnx (argv[1]);

  int epochs = 0;
  for (auto it = rnx.begin(); it != rnx.end(); ++it) {
    ++epochs;
  }

  assert(epochs > 0);
  printf("Num of epochs read: %d\n", epochs);

  return 0;
}
