#ifndef CIRCUIT_HH
#define CIRCUIT_HH

#include "gate.hh"
#include <map>
#include <set>
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
  Circuit(const string& contents);
  ~Circuit();
  Circuit* Copy();

  void AddInput(Gate* g);
  void AddOutput(Gate* g);
  void AddGate(Gate* g);
  void RemoveGate(const string& name);
  void AddLayer();
  void RemoveLayer(int idx);
  void AddEdge(const string& src, const string& dst);
  void AddEdge(Gate* src, Gate* dst);
  void RemoveEdge(const string& name);
  void RemoveEdge(Gate* node);

  Gate* PickRandomFromInputs();
  Gate* PickRandomFromOutputs();
  Gate* PickRandomFromLayersStartingFrom(int layer);
  Gate* PickRandomFromLayersEndingBefore(int layer);
  Gate* PickRandomEdgeSrc(int end_before);
  Gate* PickRandomEdgeDst(int start_at);
  pair<Gate*, Gate*> MakeRandomEdge();

  vector<int> VectorizeInputs();
  void TestAll();
  void TestAllIter(int idx);
  void TestOne();
  void FindBestPinningsIter(int idx);
  void FindBestPinningsOne();
  void AssignBestPinnings();

  void Load(const string& filename);
  string DotGraph();
  string PrintTruth();
  void PrintLayout();
  string Serialize();

  void Mutate();
  void MutationSeries();
  void MutateNewLayer();
  void MutateNewGate();
  void MutateExistingGate();
  void MutateRemoveGate();
  void MutateNewEdge();
  void MutateExistingEdge();
  void MutateRemoveEdge();

  static void Evolve();
  static set<long> MakeChildren(
    Circuit* parent, vector<Circuit*>& children, int gen, const set<long>& hashes);
  static set<long> MakeBredChildren(
    vector<Circuit*>& parents, vector<Circuit*>& children,
    int gen, const set<long>& hashes);
  static void CircuitSort(vector<Circuit*>& children);
  static Circuit* GetBestChild(vector<Circuit*>& children);
  static void FilterBestChildren(vector<Circuit*>& children, int thread);
  static void BreedBatch(vector<Circuit*>& children);
  static Circuit* Breed(Circuit* a, Circuit* b);
  static void DetectStagnation(
    vector<Circuit*>& historical, int* gen, int best_count,
    int* stag_count, Circuit* circ);

  static int GetDanglingCount(Circuit* circ);
  long Hash();

  bool operator<(Circuit& other) {
    TestAll();
    other.TestAll();
    return other.correct_count_ < correct_count_;
  }

  vector<Gate*> inputs_;
  vector<Gate*> outputs_;
  vector<vector<Gate*>> gates_;
  vector<Edge*> edges_;

  static map<vector<int>, map<string, int>> kTruthTable;
  map<vector<int>, map<string, int>> ephemeral_truth_;
  map<vector<int>, map<string, set<Gate*>>> ephemeral_outputs_;
  map<string, Gate*> best_pinnings_;

  int correct_count_;
  int total_count_;
  int superfluous_score_;
  bool bad_ = false;
};

#endif
