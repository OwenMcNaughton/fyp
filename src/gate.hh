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

  void AddInput(Gate* in);
  int Compute();
  Gate* Copy(map<string, Gate*>& table);
  void Mutate();
  void PrintLayout(int depth);

  bool FindLoops(set<Gate*>& seen);

  static string kNot, kAnd, kOrr, kXor, kNnd, kOnn, kOff, kBuf;
  static int kLineOn, kLineOff, kLineUnknown;
  static vector<string> kGates;
  static map<string, string> kDotGraphNodes;

 // private:
  vector<Gate*> inputs_;
  Gate* output_;
  string type_;
  string name_;
};

#endif
