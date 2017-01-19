#ifndef CIRCUIT_HH
#define CIRCUIT_HH

#include "gate.hh"
#include <map>
#include <string>
#include "util.hh"
#include <vector>

using namespace std;

class Circuit {
 public:
  Circuit();
  ~Circuit();
  Circuit* Copy();

  void SetTruthTable(vector<vector<string>>& truth_table);

  void AddInput(Gate* g);
  void AddOutput(Gate* g);
  void AddGate(Gate* g);
  void AddWire(const string& src, const string& dst);

  void TestOne();
  void TestAll();
  void TestAllIter(int idx);

  void Load(const string& filename);
  string Serialize();
  string DotGraph();

  void Mutate();

  void MutateExistingGate();
  void MutateEdgeSource();
  void MutateEdgeDestination();
  void MutateOutputSource();
  void MutateNewEdge();
  void MutateNewGate();
  void MutateRemoveGate();

  static void Evolve();
  
  bool FindLoops();

  bool operator<(Circuit& other) {
    TestAll();
    other.TestAll();
    return other.correct_count_ < correct_count_;
  }

  vector<Gate*> inputs_;
  vector<Gate*> outputs_;
  vector<Gate*> gates_;
  vector<pair<Gate*, Gate*>> edges_;

  static map<vector<int>, vector<pair<string, int>>> kTruthTable;

  vector<vector<string>> truth_table_str_;
  int correct_count_;
  bool bad_ = false;
};

#endif
