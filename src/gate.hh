#ifndef GATE_HH
#define GATE_HH

#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Gate {
 public:
  Gate(int type, string name, int layer, bool mutateable = true);
  ~Gate();
  Gate* Copy(map<string, Gate*>& table);

  void AddInput(Gate* in);
  void ForgetGate(Gate* g);

  int Compute();
  void Mutate();

  bool FindLoops(set<Gate*>& seen);

  void PrintLayout(int i);

  bool CanTakeInput();
  void IsConnectedToInput();
  void IsConnectedToOutput();
  int ExpectedInputCount();

  static const int kNot, kAnd, kOrr, kXor, kNnd, kOnn, kOff, kBuf,
    kFullSum, kFullCarry, kHalfSum, kHalfCarry, kNor, kXnr;
  static int kLineOn, kLineOff, kLineUnknown;
  static vector<int> kGates;
  static map<int, string> kDotGraphNodes;
  static string kDotGraphOrphanNode;

  vector<Gate*> inputs_;
  Gate* output_;
  int type_;
  string name_;
  int layer_;
  int idx_;
  bool computed_;
  int stored_answer_;
  bool orphan_;
  bool childfree_;
  bool mutateable_;
};

#endif
