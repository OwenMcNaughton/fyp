#include "circuit.hh"
#include <string.h>
#include "test.cc"
#include "util.hh"

using namespace std;

int main(int argc, char** argv) {
  Util::InitParams();

  int seed = 0;
  if (Util::kSeed == 0) {
    seed = time(NULL);
  } else {
    seed = Util::kSeed;
  }
  cout << "SEED: " << seed << endl;
  srand(seed);

  if (argc > 1) {
    if (strcmp(argv[1], "t") == 0 || strcmp(argv[1], "test") == 0) {
      Test();
    } else {
      Circuit::Evolve(argv[1]);
    }
  } else {
    Circuit::Evolve(argv[1]);
  }

}
