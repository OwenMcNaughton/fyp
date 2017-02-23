#include "util.hh"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/stat.h>

using namespace std;

int Util::kGens = 0;
int Util::kChildren = 0;
int Util::kMutations = 0;
int Util::kThreshold = 0;
int Util::kMessUp = 0;
int Util::kMutationCountFixed = 0;
int Util::kMaxGenStagnation = 0;
int Util::kThreads = 0;
int Util::kSeed = 0;
int Util::kSaveDotGraphs = 0;
int Util::kPruneOrphans = 0;
int Util::kLog = 0;
int Util::kLogIter = 0;
map<string, int> Util::split_map_ = {};

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

void Util::InitParams(int argc, char** argv, const string& file) {
  string p = ReadFile(file);
  vector<string> split = Split(p, "\n");
  for (string& s : split) {
    if (s == "") {
      continue;
    }
    vector<string> parts = Split(s, ":");
    Util::split_map_[Strip(parts[0], ' ')] = atoi(Strip(parts[1], ' ').c_str());
  }

  if (argc > 1) {
    split_map_["kLogIter"] = atoi(argv[2]);
  }
  if (argc > 2) {
    split_map_["kThreshold"] = atoi(argv[3]);
  }
  if (argc > 3) {
    if (atoi(argv[4]) != -1) {
      split_map_["kMessUp"] = atoi(argv[4]);
    }
  }

  Util::kGens = split_map_["kGens"];
  Util::kChildren = split_map_["kChildren"];
  Util::kMutations = split_map_["kMutations"];
  Util::kThreshold = split_map_["kThreshold"];
  Util::kMessUp = split_map_["kMessUp"];
  Util::kMutationCountFixed = split_map_["kMutationCountFixed"];
  Util::kMaxGenStagnation = split_map_["kMaxGenStagnation"];
  Util::kThreads = split_map_["kThreads"];
  Util::kSeed = split_map_["kSeed"];
  Util::kSaveDotGraphs = split_map_["kSaveDotGraphs"];
  Util::kPruneOrphans = split_map_["kPruneOrphans"];
  Util::kLog = split_map_["kLog"];
  Util::kLogIter = split_map_["kLogIter"];
}

EvolutionLog::EvolutionLog(Circuit* skeleton) {
  columns_ = skeleton->gates_.size();
  for (const auto& l : skeleton->gates_) {
    rows_.push_back(l.size());
  }
  goal_correct_count_ = pow(2, skeleton->inputs_.size());
  goal_total_count_ = goal_correct_count_ * skeleton->outputs_.size();
}

void EvolutionLog::SaveLog() {
  mkdir("../logs", ACCESSPERMS);
  char s[50];
  sprintf(s, "../logs/elog%05d", Util::kLogIter);

  string contents = "";
  for (const auto& param : Util::split_map_) {
    contents += param.first + ": " + to_string(param.second) + "\n";
  }
  contents += "columns: " + to_string(columns_) + "\n";
  contents += "rows: ";
  for (int i = 0; i < rows_.size(); i++) {
    contents += to_string(rows_[i]);
    contents += i == rows_.size() - 1 ? "\n" : ",";
  }
  contents += "goal_correct_count: " + to_string(goal_correct_count_) + "\n";
  contents += "goal_total_count: " + to_string(goal_total_count_) + "\n";
  contents += "~\n";

  generations_.back().best_->DetectSuperfluous();
  int gates_used = generations_.back().best_->gate_count_ -
    generations_.back().best_->superfluous_count_;
  contents += "gates_used: " + to_string(gates_used);
  contents += "total_count: " +
    to_string(generations_.back().best_->total_count_);
  contents += "percent: " + to_string(generations_.back().best_->total_count_ /
    float(goal_total_count_));

  for (const auto glog : generations_) {
    for (int i : glog.correct_counts_) {
      contents += to_string(i) + ",";
    }
    contents += ";";
    for (int i : glog.total_counts_) {
      contents += to_string(i) + ",";
    }
    contents += "\n";
  }

  WriteFile(s, contents);
}

GenerationLog::GenerationLog() {

}

GenerationLog::GenerationLog(const vector<GenerationLog>& logs, Circuit* best) {
  for (const auto& glog : logs) {
    correct_counts_.reserve(
      correct_counts_.size() + glog.correct_counts_.size());
      correct_counts_.insert(correct_counts_.end(),
      glog.correct_counts_.begin(), glog.correct_counts_.end());
    total_counts_.reserve(total_counts_.size() + glog.total_counts_.size());
    total_counts_.insert(total_counts_.end(),
      glog.total_counts_.begin(), glog.total_counts_.end());
    dupes_ += glog.dupes_;
  }
  sort(total_counts_.begin(), total_counts_.end());
  sort(correct_counts_.begin(), correct_counts_.end());
}
