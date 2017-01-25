#include "circuit.hh"
#include <string.h>
#include "test.cc"

using namespace std;

int main(int argc, char** argv) {
  srand(time(NULL));
  srand(2);

  if (argc > 1) {
    if (strcmp(argv[1], "t") == 0 || strcmp(argv[1], "test") == 0) {
      Test();
    }
  } else {
    Circuit::Evolve();
  }

}
