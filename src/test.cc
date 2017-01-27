#include "circuit.hh"
#include <iostream>

using namespace std;

void CircuitMiscTest();
void CircuitTest();
void CircuitMutateTest();
void UnpinnedCircuitTest();
void GateTest();

void TruthTest(
    vector<vector<string>>& expected, Circuit& circ, const string& msg, int inputs) {
  map<vector<int>, map<string, int>> format = FormatTruthTable(expected, inputs);
  Circuit::kTruthTable = format;
  circ.TestAll();
  map<vector<int>, map<string, int>> actual = circ.ephemeral_truth_;

  bool outputs_equal = true;
  for (auto& in : format) {
    for (auto& out : in.second) {
      if (out.second != actual[in.first][out.first]) {
        outputs_equal = false;
      }
    }
  }
  if (!outputs_equal) {
    cout << "\t FF: " << msg << " truth is wrongo!\ne a\n";
    for (auto& in : format) {
      for (auto& out : in.second) {
        cout << out.second << " ";
        cout << actual[in.first][out.first] << " ";
      }
      cout << endl;
    }
  }
}


void Test() {
  // GateTest();
  // CircuitMiscTest();
  // CircuitTest();
  // CircuitMutateTest();
  UnpinnedCircuitTest();
}

void GateTest() {
  cout << "### Gate" << endl;
}

void CircuitMiscTest() {
  cout << "### CircuitMisc" << endl;
  srand(0);

  string basic = ""
    "in1,in2,in3\n"
    "out1,out2,out3\n"
    "l1a 1,l1o 2,l1x 3\n"
    "l2x 3,l2n 0,l2a 1\n"
    "l3x 1,l3a 1,l3n 0\n"
    "l4x 2,l4a 0,l4n 2\n"
    "l5x 3,l5a 1,l5n 1\n"
    "l6x 1,l6a 2,l6n 3\n"
    "~\n"
    "in1 -> l1a\n"
    "in2 -> l1a\n"
    "l1a -> out1\n"
    "l1a -> out2\n"
    "in3 -> out3\n";

  Circuit circ(basic);

  for (int i = 0; i != 1000; i++) {
    auto e = circ.MakeRandomEdge();
    if (!e.first || !e.second) {
      continue;
    }
    if (e.first->layer_ >= e.second->layer_) {
      cout << "\t !!: " << e.first->layer_ << " " << e.second->layer_ << endl;
    }
  }

  vector<vector<string>> truth_table = {
    {"in1", "in2", "in3", "out1", "out2", "out3"},
    {"0","0","0","1","1","0"},
    {"0","0","1","1","1","0"},
    {"0","1","0","1","1","0"},
    {"0","1","1","1","1","0"},
    {"1","0","0","1","1","0"},
    {"1","0","1","1","1","0"},
    {"1","1","0","1","1","0"},
    {"1","1","1","1","1","0"}
  };
  Circuit::kTruthTable = FormatTruthTable(truth_table, 3);

  circ.TestAll();

  string expected_truth = ""
    "in1,in2,in3,out1,out2,out3,\n"
    "0 0 0 0 0 0 -\n"
    "0 0 1 0 0 1 -\n"
    "0 1 0 0 0 0 -\n"
    "0 1 1 0 0 1 -\n"
    "1 0 0 0 0 0 -\n"
    "1 0 1 0 0 1 -\n"
    "1 1 0 1 1 0 +\n"
    "1 1 1 1 1 1 -\n";

  if (circ.PrintTruth() != expected_truth) {
    cout << "\t !!: Wrong truth: " << circ.PrintTruth() << "\n" <<
      expected_truth << endl;
  }
}

void CircuitTest() {
  cout << "### Circuit" << endl;

  string basic = ""
    "in1,in2\n"
    "out\n"
    "l1a 1,l1o 2\n"
    "l2x 3,l2n 0\n"
    "l3x 3,\n"
    "~\n"
    "in1 -> l1a\n"
    "in2 -> l1a\n"
    "in1 -> l1o\n"
    "in2 -> l1o\n"
    "in1 -> l2x\n"
    "in2 -> l2x\n"
    "l1a -> l2x\n"
    "l1o -> l2n\n"
    "l2n -> l3x\n"
    "l2x -> l3x\n"
    "l3x -> out\n";

  Circuit circ(basic);

  vector<vector<string>> truth_table = {
    {"in1", "in2", "out"},
    {"0","0","1"},
    {"0","1","1"},
    {"1","0","1"},
    {"1","1","0"}
  };

  TruthTest(truth_table, circ, "First", 2);

  circ.AddGate(new Gate(Gate::kOrr, "l2o", 1));
  circ.AddEdge("in1", "l2o");
  circ.AddEdge("l1o", "l2o");
  circ.AddEdge("l2o", "l3x");

  truth_table = {
    {"in1", "in2", "out"},
    {"0","0","1"},
    {"0","1","0"},
    {"1","0","0"},
    {"1","1","1"}
  };
  TruthTest(truth_table, circ, "Second", 2);

  circ.RemoveGate("l2o");
  circ.RemoveGate("l2n");

  circ.AddGate(new Gate(Gate::kAnd, "l2a", 1));
  circ.AddGate(new Gate(Gate::kOrr, "l2o", 1));
  circ.AddEdge("l1a", "l2a");
  circ.AddEdge("l1o", "l2a");
  circ.AddEdge("in2", "l2o");
  circ.AddEdge("l1a", "l2o");
  circ.AddEdge("l2a", "l3x");
  circ.AddEdge("l2o", "l3x");

  truth_table = {
    {"in1", "in2", "out"},
    {"0","0","0"},
    {"0","1","0"},
    {"1","0","1"},
    {"1","1","0"}
  };
  TruthTest(truth_table, circ, "Third", 2);

  string full_adder = ""
    "a,b,cin\n"
    "s,cout\n"
    "xor1 3,and1 1\n"
    "and2 1,xor2 3\n"
    "orr 2,\n"
    "~\n"
    "a -> xor1\n"
    "b -> xor1\n"
    "a -> and1\n"
    "b -> and1\n"
    "cin -> and2\n"
    "cin -> xor2\n"
    "xor1 -> xor2\n"
    "xor1 -> and2\n"
    "and1 -> orr\n"
    "and2 -> orr\n"
    "orr -> cout\n"
    "xor2 -> s\n";

  Circuit c(full_adder);

  vector<vector<string>> truth_table3 = {
    {"a","b","cin","s","cout"},
    {"0","0","0","0","0"},
    {"0","0","1","1","0"},
    {"0","1","0","1","0"},
    {"0","1","1","0","1"},
    {"1","0","0","1","0"},
    {"1","0","1","0","1"},
    {"1","1","0","0","1"},
    {"1","1","1","1","1"},
  };
  TruthTest(truth_table3, c, "Fourth", 3);
}

void CircuitMutateTest() {
  cout << "### CircuitMutateTest" << endl;

  srand(time(NULL));

  {
    cout << "##### MutateNewGate" << endl;
    string basic = ""
      "in1,in2,in3\n"
      "out1,out2,out3\n"
      "l1a 1,l1o 2,l1x 3\n"
      "l2x 3,l2n 0,l2a 1\n"
      "l3x 1,l3a 1,l3n 0\n"
      "l4x 2,l4a 0,l4n 2\n"
      "l5x 3,l5a 1,l5n 1\n"
      "l6x 1,l6a 2,l6n 3\n"
      "~\n"
      "in1 -> l1a\n";
    Circuit circ(basic);
    for (int i = 0; i != 1000; i++) {
      circ.MutateNewGate();
      circ.TestAll();
    }
  }
  {
    cout << "##### MutateExistingGate" << endl;
    string basic = ""
      "in1,in2,in3\n"
      "out1,out2,out3\n"
      "l1a 1,l1o 2,l1x 3\n"
      "l2x 3,l2n 0,l2a 1\n"
      "l3x 1,l3a 1,l3n 0\n"
      "l4x 2,l4a 0,l4n 2\n"
      "l5x 3,l5a 1,l5n 1\n"
      "l6x 1,l6a 2,l6n 3\n"
      "~\n"
      "in1 -> l1a\n";
    Circuit circ(basic);
    for (int i = 0; i != 1000; i++) {
      circ.MutateExistingGate();
      circ.TestAll();
    }
  }
  {
    cout << "##### MutateRemoveGate" << endl;
    string basic = ""
      "in1,in2,in3\n"
      "out1,out2,out3\n"
      "l1a 1,l1o 2,l1x 3\n"
      "l2x 3,l2n 0,l2a 1\n"
      "l3x 1,l3a 1,l3n 0\n"
      "l4x 2,l4a 0,l4n 2\n"
      "l5x 3,l5a 1,l5n 1\n"
      "l6x 1,l6a 2,l6n 3\n"
      "~\n"
      "in1 -> l1a\n";
    Circuit circ(basic);
    for (int i = 0; i != 100; i++) {
      circ.MutateNewEdge();
    }
    for (int i = 0; i != 100; i++) {
      circ.MutateRemoveGate();
      circ.TestAll();
    }
  }
}

void UnpinnedCircuitTest() {
  string full_adder = ""
    "a,b,cin\n"
    "s,cout\n"
    "xor1 3,and1 1\n"
    "and2 1,xor2 3\n"
    "orr 2,\n"
    "~\n"
    "a -> xor1\n"
    "b -> xor1\n"
    "a -> and1\n"
    "b -> and1\n"
    "cin -> and2\n"
    "cin -> xor2\n"
    "xor1 -> xor2\n"
    "xor1 -> and2\n"
    "and1 -> orr\n"
    "and2 -> orr\n"
    "orr -> cout\n"
    "xor2 -> s\n";

  Circuit c(full_adder);

  vector<vector<string>> truth_table3 = {
    {"a","b","cin","s","cout"},
    {"0","0","0","0","0"},
    {"0","0","1","1","0"},
    {"0","1","0","1","0"},
    {"0","1","1","0","1"},
    {"1","0","0","1","0"},
    {"1","0","1","0","1"},
    {"1","1","0","0","1"},
    {"1","1","1","1","1"},
  };
  TruthTest(truth_table3, c, "Fourth", 3);
  cout << c.correct_count_ << endl;
}
