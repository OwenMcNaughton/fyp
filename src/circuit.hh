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
  void SetTruthTable(vector<vector<string>>& truth_table);

  void AddInput(Gate* g);
  void AddOutput(Gate* g);
  void AddGate(Gate* g);

  void PrintOne(bool print_legend);
  void PrintLegend();
  void PrintAll();

  void TestOne();
  void TestAll();
  void PrintCorrectness();

  void Load(const string& filename);

  void AddWire(const string& src, const string& dst);
  string Serialize();
  string DotGraph();

  Circuit* Copy();
  void Mutate();
  void Evolve();
  void PrintLayout();

  void MutateExistingGate();
  void MutateEdgeSource();
  void MutateEdgeDestination();

  bool FindLoops();

  bool operator<(Circuit& other) {
    TestAll();
    other.TestAll();
    return other.correct_count_ < correct_count_;
  }

 // private:
  void PrintAllIter(int idx);
  void TestAllIter(int idx);

  vector<Gate*> inputs_;
  vector<Gate*> outputs_;
  vector<Gate*> gates_;
  vector<pair<Gate*, Gate*>> edges_;

  static map<vector<int>, vector<pair<string, int>>> kTruthTable;

  vector<vector<string>> truth_table_str_;
  int correct_count_;
};

#endif
