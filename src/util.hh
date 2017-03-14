#ifndef UTIL_HH
#define UTIL_HH

#include "circuit.hh"
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Circuit;

class Util {
 public:
  static void InitParams(
    int argc, char** argv, const string& file = "../src/params");

  static int kGens;
  static int kChildren;
  static int kMutations;
  static int kMutatePercent;
  // 0 for exact kMutations, 1 for rand() % kMutations, 2 for kMutatePercent.
  static int kMutationMode;
  static int kThreshold;
  static int kMessUp;
  static int kMaxGenStagnation;
  static int kThreads;
  static int kSeed;
  static int kSaveDotGraphs;
  static int kPruneOrphans;
  static int kLog;
  static int kLogIter;
  static string kLogFolder;
  static vector<int> kLegalGateTypes;

  static map<string, int> split_map_;
};

vector<string> Split(const string& s, string delimiter);

string ReadFile(const string& filename);
void WriteFile(const string& filename, const string& contents);

string Strip(const string& src, char strip);

map<vector<int>, map<string, int>> FormatTruthTable(
  vector<vector<string>>& truth_table, int input_count);

vector<int> FormatTruthDecimal(
  map<vector<int>, map<string, int>> truth_table, int input_count,
  vector<Gate*> outputs);

void SaveDotGraph(Circuit* circ, string folder, int id);
void SaveDotGraph(Circuit* circ, string folder, string unique);

class GenerationLog {
 public:
  GenerationLog();
  GenerationLog(const vector<GenerationLog>& logs, Circuit* best);

  vector<int> total_counts_;
  vector<int> correct_counts_;
  int dupes_ = 0;
  Circuit* best_;
  set<long> hashes_;
};

class EvolutionLog {
 public:
  EvolutionLog(Circuit* skeleton);

  void SaveLog();

  int columns_;
  vector<int> rows_;

  set<long> hashes_;

  int goal_correct_count_;
  int goal_total_count_;
  vector<GenerationLog> generations_;
};


#endif
