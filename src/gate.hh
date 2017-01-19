#ifndef GATE_HH
#define GATE_HH

#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Gate {
 public:
  Gate(const string& type, string name);
  ~Gate();
  Gate* Copy(map<string, Gate*>& table);

  void AddInput(Gate* in);
  void ForgetGate(Gate* g);

  int Compute();
  void Mutate();

  bool FindLoops(set<Gate*>& seen);

  static string kNot, kAnd, kOrr, kXor, kNnd, kOnn, kOff, kBuf;
  static int kLineOn, kLineOff, kLineUnknown;
  static vector<string> kGates;
  static map<string, string> kDotGraphNodes;

  vector<Gate*> inputs_;
  Gate* output_;
  string type_;
  string name_;
};

#endif
