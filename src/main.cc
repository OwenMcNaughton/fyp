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

  Circuit circ;
  circ.Load(ReadFile("circs/full_adder.circ"));

  vector<vector<string>> truth_table = {
    {"a", "b", "cin", "s", "cout"},
    {"0", "0", "0", "0", "0"},
    {"0", "0", "1", "1", "0"},
    {"0", "1", "0", "1", "0"} ,
    {"0", "1", "1", "0", "1"},
    {"1", "0", "0", "1", "0"},
    {"1", "0", "1", "0", "1"},
    {"1", "1", "0", "0", "1"},
    {"1", "1", "1", "1", "1"}
  };

  Circuit::kTruthTable = FormatTruthTable(truth_table, 3);

  vector<Circuit> children;
  for (int i = 0; i != 1000; i++) {
    Circuit child = *circ.Copy();
    child.Mutate();
    // if (!child.FindLoops()) {
      children.push_back(child);
    // }
  }

  for (int i = 0; i != children.size(); i++) {
    char s[50];
    sprintf(s, "graphs/%05d.gv", i);
    string filename = s;

    // children[i].PrintLayout();

    // children[i].TestAll();
    WriteFile(filename, children[i].DotGraph());
  }
  cout << children.size() << endl;
}
