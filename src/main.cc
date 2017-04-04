#include "circuit.hh"
#include <string.h>
#include "util.hh"

using namespace std;

int main(int argc, char** argv) {
  Util::InitParams(argc, argv);

  int seed = 0;
  if (Util::kSeed == 0) {
    seed = time(NULL) * (Util::kLogIter + 1);
    Util::kSeed = seed;
  } else {
    seed = Util::kSeed;
  }
  cout << "SEED: " << seed << endl;
  srand(seed);

  Circuit::Evolve(argv[1]);
}
