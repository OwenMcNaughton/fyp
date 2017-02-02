#include "circuit.hh"
#include <string.h>
#include "test.cc"
#include "util.hh"

using namespace std;

int main(int argc, char** argv) {
  if (argc > 1) {
    srand(atoi(argv[1]));
  } else {
    int seed = time(NULL);
    cout << "SEED: " << seed << endl;
    srand(seed);
  }

  Util::InitParams();

  if (argc > 1) {
    if (strcmp(argv[1], "t") == 0 || strcmp(argv[1], "test") == 0) {
      Test();
    } else {
      Circuit::Evolve();
    }
  } else {
    Circuit::Evolve();
  }

}
