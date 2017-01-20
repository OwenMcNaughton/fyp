#ifndef CIRCUIT_HH
#define CIRCUIT_HH

#include "gate.hh"
#include <map>
#include <string>
#include "util.hh"
#include <vector>

using namespace std;

struct Edge {
  Gate* src_;
  Gate* dst_;
  int src_layer_;
  int dst_layer_;

  Edge(Gate* src, Gate* dst, int src_layer, int dst_layer)
      : src_(src), dst_(dst), src_layer_(src_layer), dst_layer_(dst_layer) {

  }
};

class Circuit {
 public:
  Circuit();
  ~Circuit();
  Circuit* Copy();

  void SetTruthTable(vector<vector<string>>& truth_table);

  void AddInput(Gate* g);
  void AddOutput(Gate* g);
  void AddGate(Gate* g, int layer);
  void AddLayer();
  void RemoveLayer(int idx);
  void AddEdge(const string& src, const string& dst);

  void TestOne();
  void TestAll();
  void TestAllIter(int idx);

  void Load(const string& filename);
  string Serialize();
  string DotGraph();

  void Mutate();
  void MutateNewGate();
  void MutateExistingGate();
  void MutateRemoveGate();
  void MutateNewEdge();
  void MutateExistingEdge();
  void MutateRemoveEdge();

  static void Evolve();

  bool FindLoops();
  static int GetDanglingCount(Circuit* circ);

  bool operator<(Circuit& other) {
    TestAll();
    other.TestAll();
    return other.correct_count_ < correct_count_;
  }

  vector<Gate*> inputs_;
  vector<Gate*> outputs_;
  vector<vector<Gate*>> gates_;
  vector<Edge*> edges_;

  static map<vector<int>, vector<pair<string, int>>> kTruthTable;

  vector<vector<string>> truth_table_str_;
  int correct_count_;
  bool bad_ = false;
};

#endif
