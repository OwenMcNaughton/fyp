#include "util.hh"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

int Util::kGens = 0;
int Util::kEvaluations = 0;
int Util::kChildren = 0;
int Util::kMutations = 0;
int Util::kMutatePercent = 0;
int Util::kMutationMode = 0;
  int Util::kMutationModeFixed = 0;
  int Util::kMutationModePercent = 1;
  int Util::kMutationModeRandom = 2;
int Util::kThreshold = 0;
int Util::kMessUp = 0;
int Util::kMaxGenStagnation = 0;
int Util::kThreads = 0;
int Util::kSeed = 0;
int Util::kSaveDotGraphs = 0;
int Util::kPruneOrphans = 0;
int Util::kPrintOrphans = 0;
int Util::kLog = 0;
int Util::kLogIter = 0;
map<string, int> Util::split_map_ = {};
string Util::kLogFolder = "";
vector<int> Util::kLegalGateTypes = {};
int Util::kBreedType = 0;
  int Util::kBreedTypeDisable = 0;
  int Util::kBreedTypeAbsPoly = 1;  // mix all of the top kBreedSample circs
  int Util::kBreedTypePerPoly = 2;  // mix all of the top kBreedSample% circs
  int Util::kBreedTypeAbsMono = 3;  // pair up each of the top kBreedSample
  int Util::kBreedTypePerMono = 4;  // pair up each of the top kBreedSample%
int Util::kBreedSample = 0;
int Util::kBreedEdges = 0;
int Util::kBreedGates = 0;
int Util::kBasicLog = 0;
int Util::kTruthWeight = 0;

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

void SaveDotGraph(Circuit* circ, string folder, int id, EvolutionLog elog) {
  circ->TestAll();
  float actual = circ->total_count_ / float(elog.goal_total_count_);
  circ->percent_ = actual;
  char s[50];
  sprintf(s, "%05d.gv", id);
  mkdir(folder.c_str(), ACCESSPERMS);
  string filename = folder + s;
  WriteFile(filename, circ->DotGraph());
  // system(("dot " + filename).c_str());
}

void SaveDotGraph(Circuit* circ, string folder, string unique, EvolutionLog elog) {
  circ->TestAll();
  float actual = circ->total_count_ / float(elog.goal_total_count_);
  circ->percent_ = actual;
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
    if (Strip(parts[0], ' ') == "kLegalGateTypes") {
      vector<string> legal_gates = Split(Strip(parts[1], ' '), ",");
      if (legal_gates.size() > 1) {
        for (auto& lg : legal_gates) {
          kLegalGateTypes.push_back(atoi(lg.c_str()));
        }
      } else {
        kLegalGateTypes = Gate::kGates;
      }
    }
    Util::split_map_[Strip(parts[0], ' ')] = atoi(Strip(parts[1], ' ').c_str());
  }


  if (argc > 1) {
    split_map_["kLogIter"] = atoi(argv[2]);
  }
  if (argc > 2) {
    split_map_["kThreshold"] = atoi(argv[3]);
  }
  if (argc > 3) {
    mkdir("../logs", ACCESSPERMS);
    Util::kLogFolder = argv[4];
    char s[50];
    sprintf(s, "../logs/%s", Util::kLogFolder.c_str());
    mkdir(s, ACCESSPERMS);
  }

  Util::kGens = split_map_["kGens"];
  Util::kEvaluations = split_map_["kEvaluations"];
  Util::kChildren = split_map_["kChildren"];
  Util::kMutations = split_map_["kMutations"];
  Util::kMutatePercent = split_map_["kMutatePercent"];
  Util::kMutationMode = split_map_["kMutationMode"];
  Util::kThreshold = split_map_["kThreshold"];
  Util::kMessUp = split_map_["kMessUp"];
  Util::kMaxGenStagnation = split_map_["kMaxGenStagnation"];
  Util::kThreads = split_map_["kThreads"];
  Util::kSeed = split_map_["kSeed"];
  Util::kSaveDotGraphs = split_map_["kSaveDotGraphs"];
  Util::kPruneOrphans = split_map_["kPruneOrphans"];
  Util::kPrintOrphans = split_map_["kPrintOrphans"];
  Util::kLog = split_map_["kLog"];
  Util::kLogIter = split_map_["kLogIter"];
  Util::kBreedType = split_map_["kBreedType"];
  Util::kBreedSample = split_map_["kBreedSample"];
  Util::kBreedEdges = split_map_["kBreedEdges"];
  Util::kBreedGates = split_map_["kBreedGates"];
  Util::kBasicLog = split_map_["kBasicLog"];
  Util::kTruthWeight = split_map_["kTruthWeight"];
  int threads = sysconf(_SC_NPROCESSORS_ONLN);
  if (threads > 50) {
    Util::kThreads = 50;
  }
  Util::kChildren = Util::kChildren / Util::kThreads;
}

EvolutionLog::EvolutionLog(Circuit* skeleton) {
  columns_ = skeleton->gates_.size();
  for (const auto& l : skeleton->gates_) {
    rows_.push_back(l.size());
  }

  goal_correct_count_ = pow(2, skeleton->inputs_.size());
  goal_total_count_ = goal_correct_count_ * skeleton->outputs_.size();
  goal_total_weighted_count_ = goal_correct_count_ * pow(2, skeleton->outputs_.size());
}

void EvolutionLog::SaveLog() {
  char s[500];
  sprintf(s, "../logs/%s/elog%05d", Util::kLogFolder.c_str(), Util::kLogIter);

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
  contents += "goal_total_weighted_count: " + to_string(goal_total_weighted_count_) + "\n";
  contents += "~\n";

  generations_.back().best_->DetectSuperfluous();
  int gates_used = generations_.back().best_->gate_count_ -
    generations_.back().best_->superfluous_count_;
  contents += "gates_used: " + to_string(gates_used) + "\n";
  contents += "total_count: " +
    to_string(generations_.back().best_->total_count_) + "\n";
  contents += "total_weighted_count: " +
    to_string(generations_.back().best_->total_weighted_count_) + "\n";
  contents += "percent: " + to_string(generations_.back().best_->total_count_ /
    float(goal_total_count_)) + "\n";
  contents += "weighted_percent: " + to_string(generations_.back().best_->total_weighted_count_ /
    float(goal_total_weighted_count_)) + "\n";

  // for (const auto glog : generations_) {
  //   for (int i : glog.correct_counts_) {
  //     contents += to_string(i) + ",";
  //   }
  //   contents += ";";
  //   for (int i : glog.total_counts_) {
  //     contents += to_string(i) + ",";
  //   }
  //   contents += "\n";
  // }

  WriteFile(s, contents);
}

void EvolutionLog::SaveBasicLog(int evaluations) {
  char s[500];
  sprintf(s, "../logs/%s/elog%05d", Util::kLogFolder.c_str(), Util::kLogIter);

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
  contents += "goal_total_weighted_count: " + to_string(goal_total_weighted_count_) + "\n";
  contents += "~\n";

  generations_.back().best_->DetectSuperfluous();
  int gates_used = generations_.back().best_->gate_count_ -
    generations_.back().best_->superfluous_count_;
  contents += "gates_used: " + to_string(gates_used) + "\n";
  contents += "total_count: " +
    to_string(generations_.back().best_->total_count_) + "\n";
  contents += "total_weighted_count: " +
    to_string(generations_.back().best_->total_weighted_count_) + "\n";
  contents += "percent: " + to_string(generations_.back().best_->total_count_ /
    float(goal_total_count_)) + "\n";
  contents += "weighted_percent: " + to_string(generations_.back().best_->total_weighted_count_ /
    float(goal_total_weighted_count_)) + "\n";
  contents += "evaluations: " + to_string(evaluations) + "\n";

  // for (const auto glog : generations_) {
  //   for (int i : glog.correct_counts_) {
  //     contents += to_string(i) + ",";
  //   }
  //   contents += ";";
  //   for (int i : glog.total_counts_) {
  //     contents += to_string(i) + ",";
  //   }
  //   contents += "\n";
  // }

  WriteFile(s, contents);
}

void EvolutionLog::SaveFullLog(int evaluations, const string& name) {
  char s[500];
  sprintf(s, "../logs/%s/%s", Util::kLogFolder.c_str(), name.c_str());

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
  contents += "goal_total_weighted_count: " + to_string(goal_total_weighted_count_) + "\n";
  contents += "~\n";

  generations_.back().best_->DetectSuperfluous();
  int gates_used = generations_.back().best_->gate_count_ -
    generations_.back().best_->superfluous_count_;
  contents += "gates_used: " + to_string(gates_used) + "\n";
  contents += "total_count: " +
    to_string(generations_.back().best_->total_count_) + "\n";
  contents += "total_weighted_count: " +
    to_string(generations_.back().best_->total_weighted_count_) + "\n";
  contents += "percent: " + to_string(generations_.back().best_->total_count_ /
    float(goal_total_count_)) + "\n";
  contents += "weighted_percent: " + to_string(generations_.back().best_->total_weighted_count_ /
    float(goal_total_weighted_count_)) + "\n";
  contents += "evaluations: " + to_string(evaluations) + "\n";

  contents += "~\n";

  for (int i : total_history_) {
    contents += to_string(i) + ",";
  }
  contents += "\n";
  for (int i : weighted_total_history_) {
    contents += to_string(i) + ",";
  }
  contents += "\n";
  for (float i : percent_history_) {
    contents += to_string(i) + ",";
  }
  contents += "\n";
  for (float i : weighted_percent_history_) {
    contents += to_string(i) + ",";
  }
  contents += "\n";

  WriteFile(s, contents);
}

Circuit* EvolutionLog::DetectStagnation() {
  if (generations_.size() < Util::kMaxGenStagnation) {
    return generations_.back().best_;
  } else {
    int idx = generations_.size() - Util::kMaxGenStagnation;
    if (generations_[idx].best_->total_count_ >=
        generations_.back().best_->total_count_) {
      return generations_[0].best_;
    } else {
      return generations_.back().best_;
    }
  }
}

GenerationLog::GenerationLog() {

}

struct CircuitWeightedTruthSort {
  inline bool operator() (Circuit* circ1, Circuit* circ2) {
    return circ1->total_weighted_count_ > circ2->total_weighted_count_;
  }
};

GenerationLog::GenerationLog(const vector<GenerationLog>& logs, Circuit* best) {
  best_ = logs[0].best_;
  for (const auto& glog : logs) {
    correct_counts_.reserve(
      correct_counts_.size() + glog.correct_counts_.size());
      correct_counts_.insert(correct_counts_.end(),
      glog.correct_counts_.begin(), glog.correct_counts_.end());
    total_counts_.reserve(total_counts_.size() + glog.total_counts_.size());
    total_counts_.insert(total_counts_.end(),
      glog.total_counts_.begin(), glog.total_counts_.end());
    dupes_ += glog.dupes_;
    bests_.insert(bests_.end(), glog.bests_.begin(), glog.bests_.end());
  }
  sort(bests_.begin(), bests_.end(), CircuitWeightedTruthSort());
  if (bests_.size() > Util::kBreedSample) {
    for (int i = Util::kBreedSample; i != bests_.size(); i++) {
      delete bests_[i];
    }
    bests_.erase(bests_.begin() + Util::kBreedSample, bests_.end());
  }
  best_ = bests_[0];
  sort(total_counts_.begin(), total_counts_.end());
  sort(correct_counts_.begin(), correct_counts_.end());
}
