#include "circuit.hh"

#include <cmath>
#include <iostream>
#include <set>
#include "util.hh"

using namespace std;

map<vector<int>, vector<pair<string, int>>> Circuit::kTruthTable = {};

Circuit::Circuit() {

}

Circuit* Circuit::Copy() {
  Circuit* c = new Circuit();
  c->Load(Serialize());
  return c;
}

void Circuit::Mutate() {
  int mutation_type = rand() % 7;
  switch (mutation_type) {
    case 0: MutateExistingGate(); break;
    case 1: MutateEdgeSource(); break;
    case 2: MutateEdgeDestination(); break;
    case 3: MutateOutputSource(); break;
    case 4: MutateNewGate(); break;
    case 5: MutateNewEdge(); break;
    case 6: MutateRemoveGate(); break;
  }
  return;
}

void Circuit::MutateExistingGate() {
  if (gates_.empty()) {
    return;
  }

  Gate* mutated = gates_[rand() % gates_.size()];
  mutated->Mutate();
}

void Circuit::MutateEdgeSource() {
  if (edges_.empty() || gates_.empty()) {
    return;
  }
  auto p = edges_[rand() % edges_.size()];
  Gate* new_src = gates_[rand() % gates_.size()];

  for (int i = 0; i < p.second->inputs_.size(); i++) {
    if (p.second->inputs_[i] == p.first) {
      p.second->inputs_.erase(p.second->inputs_.begin() + i);
    }
  }
  p.second->inputs_.push_back(new_src);

  p.first = new_src;
}

void Circuit::MutateEdgeDestination() {
  if (edges_.empty() || gates_.empty()) {
    return;
  }
  auto p = edges_[rand() % edges_.size()];
  Gate* new_dst = gates_[rand() % gates_.size()];

  for (int i = 0; i < p.first->inputs_.size(); i++) {
    if (p.first->inputs_[i] == p.second) {
      p.first->inputs_.erase(p.first->inputs_.begin() + i);
    }
  }
  p.first->inputs_.push_back(new_dst);

  p.second = new_dst;
}

void Circuit::MutateOutputSource() {
  if (edges_.empty() || gates_.empty()) {
    return;
  }
  Gate* out = outputs_[rand() % outputs_.size()];
  Gate* new_src = gates_[rand() % gates_.size()];

  for (int i = 0; i != edges_.size(); i++) {
    if (edges_[i].second == out) {
      edges_[i].first = new_src;
    }
  }

  out->inputs_.clear();
  out->inputs_.push_back(new_src);
}

void Circuit::MutateNewGate() {
  Gate* g = new Gate(Gate::kGates[rand() % 4], to_string(rand()));
  gates_.push_back(g);
}

void Circuit::MutateNewEdge() {
  if (gates_.empty()) {
    return;
  }

  Gate* src = gates_[rand() % gates_.size()];
  Gate* dst = gates_[rand() % gates_.size()];
  auto pair = make_pair(src, dst);
  dst->inputs_.push_back(src);
}

void Circuit::MutateRemoveGate() {
  if (gates_.empty() || edges_.empty()) {
    return;
  }

  int idx = rand() % gates_.size();
  Gate* to_remove = gates_[idx];
  for (int i = 0; i < edges_.size(); i++) {
    if (edges_[i].first == to_remove || edges_[i].second == to_remove) {
      for (int j = 0; j < edges_[i].second->inputs_.size(); j++) {
        if (edges_[i].second->inputs_[j] == to_remove) {
          edges_[i].second->inputs_.erase(edges_[i].second->inputs_.begin() + j);
        }
      }
      edges_.erase(edges_.begin() + i--);
    }
  }

  for (Gate* g : gates_) {
    g->ForgetGate(gates_[idx]);
  }
  for (Gate* g : inputs_) {
    g->ForgetGate(gates_[idx]);
  }
  for (Gate* g : outputs_) {
    g->ForgetGate(gates_[idx]);
  }

  delete gates_[idx];
  gates_.erase(gates_.begin() + idx);
}

bool Circuit::FindLoops() {
  for (Gate* in : outputs_) {
    set<Gate*> seen;
    if (in->FindLoops(seen)) {
      return true;
    }
  }
  return false;
}

void Circuit::Evolve() {
  Circuit* circ = new Circuit();
  circ->Load(ReadFile("circs/full_adder_starter.circ"));

  vector<vector<string>> truth_table = {
    {"a", "b", "cin", "s", "cout"},
    {"0","0","0","0","0"},
    {"0","0","1","1","0"},
    {"0","1","0","1","0"},
    {"0","1","1","0","1"},
    {"1","0","0","1","0"},
    {"1","0","1","0","1"},
    {"1","1","0","0","1"},
    {"1","1","1","1","1"}
  };

  Circuit::kTruthTable = FormatTruthTable(truth_table, 3);

  SaveDotGraph(circ, 0);

  for (int i = 0; i != kGens; i++) {
    cout << "GEN: " << i << endl;
    vector<Circuit*> children;
    for (int j = 0; j != kChildren; j++) {
      Circuit* child = circ->Copy();
      int m = rand() % kMutations;
      for (int k = 0; k != m; k++) {
        child->Mutate();
      }
      if (!child->FindLoops()) {
        children.push_back(child);
      }
    }

    int correct_max = -1;
    for (int j = 0; j != children.size(); j++) {
      children[j]->TestAll();
      if (!children[j]->bad_) {
        if (children[j]->correct_count_ > correct_max) {
          correct_max = children[j]->correct_count_;
        }
      }
    }

    int gate_min = 9999999999;
    int maxidx = -1;
    vector<Circuit*> good_circs;
    for (int j = 0; j != children.size(); j++) {
      if (children[j]->correct_count_ == correct_max) {
        int dangling_count = 0;
        for (Gate* g : children[j]->gates_) {
          if (g->inputs_.empty()) {
            dangling_count++;
          }
        }
        if (dangling_count < 4) {
          good_circs.push_back(children[j]);
        }
      }
    }

    Circuit* best = good_circs[rand() % good_circs.size()];

    if (circ->correct_count_ <= best->correct_count_) {
      circ = best->Copy();
      circ->TestAll();
      SaveDotGraph(best, i + 1);
    }

    for (int j = 0; j < children.size(); j++) {
      delete children[j];
    }
  }
}

void Circuit::AddInput(Gate* g) {
  inputs_.push_back(g);
}

void Circuit::AddOutput(Gate* g) {
  outputs_.push_back(g);
}

void Circuit::AddGate(Gate* g) {
  gates_.push_back(g);
}

void Circuit::TestOne() {
  vector<int> key;
  string kk = "";
  for (Gate* g : inputs_) {
    int res = g->Compute() == 1 ? 1 : 0;
    kk += to_string(res) + " ";
    key.push_back(res);
  }

  vector<pair<string, int>> val = kTruthTable[key];
  int count = 0;
  for (auto expected : val) {
    for (Gate* g : outputs_) {
      if (g->name_ == expected.first) {
        int res = g->Compute();
        if (res == -1) {
          res = 0;
        } else if (res == 0) {
          bad_ = true;
        }
        if (res == expected.second) {
          count++;
        }
      }
    }
  }

  if (count == val.size()) {
    correct_count_++;
  }
}

void Circuit::TestAll() {
  correct_count_ = 0;
  TestAllIter(0);
}

void Circuit::TestAllIter(int idx) {
  if (idx == inputs_.size() - 1) {
    inputs_[idx]->type_ = Gate::kOff;
    TestOne();
    inputs_[idx]->type_ = Gate::kOnn;
    TestOne();
  } else {
    inputs_[idx]->type_ = Gate::kOff;
    TestAllIter(idx + 1);
    inputs_[idx]->type_ = Gate::kOnn;
    TestAllIter(idx + 1);
  }
}

void Circuit::AddWire(const string& src, const string& dst) {
  Gate* src_gate = NULL;
  Gate* dst_gate = NULL;

  for (Gate* g : gates_) {
    if (g->name_ == src) {
      src_gate = g;
    } else if (g->name_ == dst) {
      dst_gate = g;
    }
  }
  for (Gate* g : inputs_) {
    if (g->name_ == src) {
      src_gate = g;
    } else if (g->name_ == dst) {
      dst_gate = g;
    }
  }
  for (Gate* g : outputs_) {
    if (g->name_ == src) {
      src_gate = g;
    } else if (g->name_ == dst) {
      dst_gate = g;
    }
  }

  if (src_gate == NULL) {
    cout << "src_gate: " << src << " not found" << endl;
    return;
    exit(1);
  }
  if (dst_gate == NULL) {
    cout << "dst_gate: " << dst << " not found" << endl;
    return;
    exit(1);
  }
  edges_.push_back(make_pair(src_gate, dst_gate));
  dst_gate->AddInput(src_gate);
}

void Circuit::Load(const string& contents) {
  vector<string> split = Split(contents, "~");
  vector<string> nodes = Split(split[0], "\n");
  vector<string> edges = Split(split[1], "\n");

  for (auto s : nodes) {
    vector<string> node = Split(s, " ");
    if (node.size() < 2) {
      continue;
    }
    if (node[0] == "in") {
      Gate* g = new Gate(Gate::kOnn, node[1]);
      AddInput(g);
    } else if (node[0] == "out") {
      Gate* g = new Gate(Gate::kBuf, node[1]);
      AddOutput(g);
    } else {
      Gate* g = new Gate(node[1], node[0]);
      AddGate(g);
    }
  }
  for (auto s : edges) {
    vector<string> edge = Split(s, "->");
    if (edge.size() < 2) {
      continue;
    }
    string src = Strip(edge[0], ' ');
    vector<string> dsts = Split(edge[1], ",");
    for (string dst : dsts) {
      AddWire(src, Strip(dst, ' '));
    }
  }
}

string Circuit::Serialize() {
  string s = "";
  for (Gate* g : gates_) {
    s += g->name_ + " " + g->type_ + "\n";
  }
  for (Gate* g : inputs_) {
    s += "in " + g->name_ + "\n";
  }
  for (Gate* g : outputs_) {
    s += "out " + g->name_ + "\n";
  }

  s += "~\n";

  for (Gate* g : gates_) {
    for (Gate* in : g->inputs_) {
      s += in->name_ + " -> " + g->name_ + "\n";
    }
  }
  for (Gate* g : outputs_) {
    for (Gate* in : g->inputs_) {
      s += in->name_ + " -> " + g->name_ + "\n";
    }
  }
  return s;
}

string Circuit::DotGraph() {
  string dotgraph = "digraph {\n";
  int out_of = pow(2, inputs_.size()) * outputs_.size();
  dotgraph += "labelloc=\"t\"\nlabel=\"" +
    to_string(correct_count_) + " / " + to_string(out_of) + "\"\n";
  for (Gate* g : gates_) {
    dotgraph += "\t" + g->name_ + " " + Gate::kDotGraphNodes[g->type_] + "\n";
  }
  for (Gate* g : inputs_) {
    dotgraph += "\t" + g->name_ + " " + Gate::kDotGraphNodes[g->type_] + "\n";
  }
  for (Gate* g : outputs_) {
    dotgraph += "\t" + g->name_ + " " + Gate::kDotGraphNodes[g->type_] + "\n";
  }

  dotgraph += "\n";

  for (auto edge : edges_) {
    dotgraph += "\t" + edge.first->name_ + " -> " + edge.second->name_ + "\n";
  }

  dotgraph += "\n}";
  return dotgraph;
}

Circuit::~Circuit() {
  for (Gate* g : gates_) {
    delete g;
  }
  for (Gate* g : inputs_) {
    delete g;
  }
  for (Gate* g : outputs_) {
    delete g;
  }
}
