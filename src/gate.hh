#ifndef GATE_HH
#define GATE_HH

#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Gate {
 public:
  Gate(int type, string name, int layer);
  ~Gate();
  Gate* Copy(map<string, Gate*>& table);

  void AddInput(Gate* in);
  void ForgetGate(Gate* g);

  int Compute();
  void Mutate();

  bool FindLoops(set<Gate*>& seen);

  void PrintLayout(int i);

  static int kNot, kAnd, kOrr, kXor, kNnd, kOnn, kOff, kBuf;
  static int kLineOn, kLineOff, kLineUnknown;
  static vector<int> kGates;
  static map<int, string> kDotGraphNodes;

  vector<Gate*> inputs_;
  Gate* output_;
  int type_;
  string name_;
  int layer_;
};

#endif
