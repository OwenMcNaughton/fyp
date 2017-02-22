#include "util.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/stat.h>

using namespace std;

int Util::kGens = 0;
int Util::kChildren = 0;
int Util::kMutations = 0;
int Util::kMutationCountFixed = 0;
int Util::kMaxGenStagnation = 0;
int Util::kThreads = 0;
int Util::kSeed = 0;

vector<string> Split(const string& s, string delimiter) {
  vector<string> v;

  size_t match = s.find(delimiter);
  if (match == string::npos) {
    v.push_back(s);
    return v;
  }

  size_t last = 0;
  while (match != string::npos) {
    v.push_back(s.substr(last, match - last));
    last = match + delimiter.size();
    match = s.find(delimiter, match + 1);
  }
  v.push_back(s.substr(last));
  return v;
}

string ReadFile(const string& filename) {
  ifstream file(filename.c_str());
  if(file.fail()) {
    cout << "error loading file called " << filename << endl;
    exit(1);
  }

  stringstream stream;
  stream << file.rdbuf();
  file.close();

  return stream.str();
}

string Strip(const string& src, char strip) {
  string res = "";
  for (char c : src) {
    if (c != strip) {
      res += c;
    }
  }
  return res;
}

void WriteFile(const string& filename, const string& contents) {
  ofstream out(filename);
  out << contents;
  out.close();
}

map<vector<int>, map<string, int>> FormatTruthTable(
    vector<vector<string>>& truth_table, int input_count) {
  map<vector<int>, map<string, int>> truth_table_ret;
  for (int i = 1; i != truth_table.size(); i++) {
    vector<int> ins;
    for (int j = 0; j != input_count; j++) {
      ins.push_back(atoi(truth_table[i][j].c_str()));
    }
    map<string, int> expecteds;
    for (int j = input_count; j != truth_table[0].size(); j++) {
      expecteds[truth_table[0][j]] = atoi(truth_table[i][j].c_str());
    }
    truth_table_ret[ins] = expecteds;
  }
  return truth_table_ret;
}

vector<int> FormatTruthDecimal(
    map<vector<int>, map<string, int>> truth_table, int input_count,
    vector<Gate*> outputs) {
  vector<int> decimal_truth;
  for (auto& pair : truth_table) {
    int output_val = 0;
    for (Gate* g : outputs) {
      output_val = output_val << 1;
      output_val += pair.second[g->name_];
    }
    decimal_truth.push_back(output_val);
  }
  return decimal_truth;
}

void SaveDotGraph(Circuit* circ, string folder, int id) {
  circ->TestAll();
  char s[50];
  sprintf(s, "%05d.gv", id);
  mkdir(folder.c_str(), ACCESSPERMS);
  string filename = folder + s;
  WriteFile(filename, circ->DotGraph());
  // system(("dot " + filename).c_str());
}

void SaveDotGraph(Circuit* circ, string folder, string unique) {
  circ->TestAll();
  mkdir(folder.c_str(), ACCESSPERMS);
  string filename = folder + unique + ".gv";
  WriteFile(filename, circ->DotGraph());
  // system(("dot " + filename).c_str());
}

void Util::InitParams(const string& file) {
  string p = ReadFile(file);
  vector<string> split = Split(p, "\n");
  map<string, int> split_map;
  for (string& s : split) {
    if (s == "") {
      continue;
    }
    vector<string> parts = Split(s, ":");
    split_map[Strip(parts[0], ' ')] = atoi(Strip(parts[1], ' ').c_str());
  }
  Util::kGens = split_map["kGens"];
  Util::kChildren = split_map["kChildren"];
  Util::kMutations = split_map["kMutations"];
  Util::kMutationCountFixed = split_map["kMutationCountFixed"]
  Util::kMaxGenStagnation = split_map["kMaxGenStagnation"];
  Util::kThreads = split_map["kThreads"];
  Util::kSeed = split_map["kSeed"];
}
