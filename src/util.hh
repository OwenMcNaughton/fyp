#ifndef UTIL_HH
#define UTIL_HH

#include "circuit.hh"
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

class Circuit;

class Util {
 public:
  static void InitParams(const string& file = "../src/params");

  static int kGens;
  static int kChildren;
  static int kMutations;
  static int kMaxGenStagnation;
  static int kThreads;
};

vector<string> Split(const string& s, string delimiter);

string ReadFile(const string& filename);
void WriteFile(const string& filename, const string& contents);

string Strip(const string& src, char strip);

map<vector<int>, map<string, int>> FormatTruthTable(
  vector<vector<string>>& truth_table, int input_count);

void SaveDotGraph(Circuit* circ, string folder, int id);


#endif
