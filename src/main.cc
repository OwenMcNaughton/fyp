#include <algorithm>
#include <cmath>
#include "gate.hh"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>

#include "circuit.hh"
#include "gate.hh"
#include "util.hh"

using namespace std;

int main(int argc, char** argv) {
  srand(time(NULL));
  srand(1);

  Circuit::Evolve();
}
