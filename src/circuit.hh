#ifndef CIRCUIT_HH
#define CIRCUIT_HH

#include "gate.hh"
#include <map>
#include <set>
#include <string>
#include "util.hh"
#include <vector>

using namespace std;

class GenerationLog;
class EvolutionLog;

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
  Gate* PickRandomSrc(int end_before);
  Gate* PickRandomEdgeDst(int start_at);
  pair<Gate*, Gate*> MakeRandomEdge();

  vector<int> VectorizeInputs();
  void TestAll();
  void TestAllIter(int idx);
  void TestOne();

  void DetectOrphans();
  void DetectSuperfluous();

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
  void MutateAnInput();
  void FixMutatedGate(Gate* g);

  void MessUp(int factor);
  void FillEdges();
  void BinTruthToDec();

  static void CopyGates(Circuit* src, Circuit* dst);
  static Circuit* Breed(vector<Circuit*>& parents);

  static int GetDanglingCount(Circuit* circ);
  long Hash();

  bool operator<(Circuit& other) {
    TestAll();
    other.TestAll();
    return other.correct_count_ < correct_count_;
  }

  // { NON UTIL
  static void Evolve(const string& target);
  static GenerationLog MakeChildren(
    Circuit* parent, vector<Circuit*>& children,
    int gen, const EvolutionLog& elog);
  static set<long> MakeBredChildren(
    vector<Circuit*>& parents, vector<Circuit*>& children,
    int gen, const set<long>& hashes);
  static void CircuitSort(vector<Circuit*>& children);
  static Circuit* GetBestChild(vector<Circuit*>& children);
  static vector<Circuit*> GetBestChildren(vector<Circuit*>& children);
  // NON UTIL }

  vector<Gate*> inputs_;
  vector<Gate*> outputs_;
  vector<vector<Gate*>> gates_;
  vector<Edge*> edges_;

  static map<vector<int>, map<string, int>> kTruthTable;
  static vector<int> kTruthDecimal;
  vector<int> truth_decimal_;
  map<vector<int>, map<string, int>> ephemeral_truth_;
  map<vector<int>, map<string, set<Gate*>>> ephemeral_outputs_;
  map<string, Gate*> best_pinnings_;

  int correct_count_;
  int total_count_;
  int total_weighted_count_;
  int superfluous_score_;
  int decimal_diff_;
  bool bad_ = false;
  int gate_count_;
  int orphan_count_;
  int superfluous_count_;
  int genome_size_;
  float percent_;
  float weighted_percent_;
};

#endif
