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
  cout << mutation_type << endl;
  switch (mutation_type) {
    // case 0: MutateNewGate(); break;
    // case 1: MutateExistingGate(); break;
    // case 2: MutateRemoveGate(); break;
    // case 3: MutateNewEdge(); break;
    // case 4: MutateExistingEdge(); break;
    // case 5: MutateRemoveEdge(); break;
  }
}

void Circuit::MutateNewGate() {
  Gate* g = new Gate(Gate::kGates[rand() % 4], to_string(rand()));
  int layer = rand() % gates_.size();
  AddGate(g, layer);
}

void Circuit::MutateExistingGate() {
  if (gates_.empty()) {
    return;
  }

  int layer = rand() % gates_.size();
  if (gates_[layer].empty()) {
    return;
  }
  Gate* mutated = gates_[layer][rand() % gates_[layer].size()];
  mutated->Mutate();
}

void Circuit::MutateRemoveGate() {
  if (gates_.empty() || edges_.empty()) {
    return;
  }

  int layer = rand() % gates_.size();
  if (gates_[layer].empty()) {
    return;
  }
  int idx = rand() % gates_[layer].size();
  Gate* to_remove = gates_[layer][idx];
  for (int i = 0; i < edges_.size(); i++) {
    if (edges_[i]->src_ == to_remove || edges_[i]->dst_ == to_remove) {
      for (int j = 0; j < edges_[i]->dst_->inputs_.size(); j++) {
        if (edges_[i]->dst_->inputs_[j] == to_remove) {
          edges_[i]->dst_->inputs_.erase(edges_[i]->dst_->inputs_.begin() + j);
        }
      }
      edges_.erase(edges_.begin() + i--);
    }
  }

  for (auto v : gates_) {
    for (Gate* g : v) {
      g->ForgetGate(gates_[layer][idx]);
    }
  }
  for (Gate* g : inputs_) {
    g->ForgetGate(gates_[layer][idx]);
  }
  for (Gate* g : outputs_) {
    g->ForgetGate(gates_[layer][idx]);
  }

  delete gates_[layer][idx];
  gates_[layer].erase(gates_[layer].begin() + idx);
}

void Circuit::MutateNewEdge() {
  if (gates_.empty()) {
    return;
  }

  int src_layer = rand() % gates_.size() == 0 ? -1 : rand() % gates_.size();
  Gate* src = nullptr;
  if (src_layer == -1) {
    src = inputs_[rand() % inputs_.size()];
  } else {
    if (gates_[src_layer].empty()) {
      return;
    }
    src = gates_[src_layer][rand() % gates_[src_layer].size()];
  }

  Gate* dst = nullptr;
  int dst_layer = -2;
  if (src_layer == -1) {
    if (rand() % gates_.size() == 0) {
      dst = outputs_[rand() % outputs_.size()];
    } else {
      dst_layer = rand() % gates_.size();
      if (gates_[dst_layer].empty()) {
        return;
      }
      dst = gates_[dst_layer][rand() % gates_[dst_layer].size()];
    }
  } else if (src_layer == gates_.size() - 1) {
    dst = outputs_[rand() % outputs_.size()];
  } else {
    if (rand() % gates_.size() == 0) {
      dst = outputs_[rand() % outputs_.size()];
    } else {
      dst_layer = rand() % (gates_.size() - src_layer - 1) + src_layer + 1;
      if (gates_[dst_layer].empty()) {
        return;
      }
      dst = gates_[dst_layer][rand() % gates_[dst_layer].size()];
    }
  }
  dst->inputs_.push_back(src);

  Edge* edge = new Edge(src, dst, src_layer, dst_layer);
  edges_.push_back(edge);
}

void Circuit::MutateExistingEdge() {
  if (edges_.empty() || gates_.empty()) {
    return;
  }

  Edge* edge = edges_[rand() % edges_.size()];

  if (rand() % 2 == 0) {
    Gate* src = nullptr;
    int src_layer = -1;
    if (rand() % gates_.size() == 0 || edge->dst_layer_ == 0) {
      // source = input
      if (inputs_.empty()) {
        return;
      }
      src = inputs_[rand() % inputs_.size()];
    } else {
      // source = before dst layer
      src_layer = rand() % edge->dst_layer_;
      if (gates_[src_layer].empty()) {
        return;
      }
      src = gates_[src_layer][rand() % gates_[src_layer].size()];
    }

    for (int i = 0; i < edge->dst_->inputs_.size(); i++) {
      if (edge->dst_->inputs_[i] == edge->src_) {
        edge->dst_->inputs_.erase(edge->dst_->inputs_.begin() + i--);
      }
    }
    edge->src_ = src;
    edge->src_layer_ = src_layer;
    edge->dst_->inputs_.push_back(src);

  } else {
    Gate* dst = nullptr;
    int dst_layer = -2;
    if (rand() % gates_.size() == 0 || edge->src_layer_ == gates_.size() - 1) {
      // dst = output
      if (outputs_.empty()) {
        return;
      }
      dst = outputs_[rand() % outputs_.size()];
    } else {
      // dst = after src layer
      dst_layer = rand() % (gates_.size() - edge->src_layer_ - 1) + edge->src_layer_ + 1;
      if (gates_[dst_layer].empty()) {
        return;
      }
      dst = gates_[dst_layer][rand() % gates_[dst_layer].size()];
    }

    for (int i = 0; i < edge->dst_->inputs_.size(); i++) {
      if (edge->dst_->inputs_[i] == edge->src_) {
        edge->dst_->inputs_.erase(edge->dst_->inputs_.begin() + i);
      }
    }
    edge->dst_ = dst;
    edge->dst_layer_ = dst_layer;
    edge->dst_->inputs_.push_back(edge->src_);

  }
}

void Circuit::MutateRemoveEdge() {

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

int Circuit::GetDanglingCount(Circuit* circ) {
  int dangling_count = 0;
  for (auto v : circ->gates_) {
    for (Gate* g : v) {
      if (g->inputs_.empty()) {
        dangling_count++;
      }
    }
  }
  return dangling_count;
}

void Circuit::Evolve() {
  Circuit* circ = new Circuit();
  circ->Load(ReadFile("circs/full_adder.circ"));

  return;

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
      cout << "\t\t\t\t" << child->edges_.size() << endl;
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
        int dangling_count = GetDanglingCount(children[j]);
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

void Circuit::AddGate(Gate* g, int layer) {
  gates_[layer].push_back(g);
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

void Circuit::AddEdge(const string& src, const string& dst) {
  Gate* src_gate = NULL;
  Gate* dst_gate = NULL;

  int layer = 0;
  int src_layer = -1;
  int dst_layer = -2;
  for (auto v : gates_) {
    for (Gate* g : v) {
      if (g->name_ == src) {
        src_gate = g;
        src_layer = layer;
      } else if (g->name_ == dst) {
        dst_gate = g;
        dst_layer = layer;
      }
    }
    layer++;
  }
  for (Gate* g : inputs_) {
    if (g->name_ == src) {
      src_gate = g;
    }
  }
  for (Gate* g : outputs_) {
    if (g->name_ == dst) {
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
  edges_.push_back(new Edge(src_gate, dst_gate, src_layer, dst_layer));
  dst_gate->AddInput(src_gate);
}

void Circuit::Load(const string& contents) {
  vector<string> split = Split(contents, "~");
  vector<string> nodes = Split(split[0], ";");
  vector<string> inputs = Split(nodes[0], ",");
  vector<string> layers = Split(nodes[1].substr(1), "\n");
  vector<string> outputs = Split(nodes[2], ",");
  vector<string> edges = Split(split[1], "\n");

  for (auto s : inputs) {
    Gate* g = new Gate(Gate::kOnn, s);
    AddInput(g);
  }
  for (auto s : outputs) {
    Gate* g = new Gate(Gate::kOnn, Strip(s, '\n'));
    AddOutput(g);
  }
  int i = 0;
  for (auto l : layers) {
    vector<Gate*> layer;
    gates_.push_back(layer);
    vector<string> gates = Split(Strip(l, '\n'), ",");
    for (auto s : gates) {
      vector<string> node = Split(s, " ");
      Gate* g = new Gate(node[1], node[0]);
      AddGate(g, i);
    }
    i++;
  }

  for (auto s : edges) {
    vector<string> edge = Split(s, "->");
    if (edge.size() < 2) {
      continue;
    }
    string src = Strip(edge[0], ' ');
    vector<string> dsts = Split(edge[1], ",");
    for (string dst : dsts) {
      cout << src << " -> " << Strip(dst, ' ' ) << endl;
      AddEdge(src, Strip(dst, ' '));
    }
  }
}

string Circuit::Serialize() {
  string s = "";
  for (Gate* g : inputs_) {
    s += g->name_ ;
    s += g == inputs_.back() ? ";\n" : ",";
  }

  for (auto v : gates_) {
    for (Gate* g : v) {
      s += g->name_ + " " + g->type_;
      s += g == gates_.back().back() ? ";" : "";
      s += g == v.back() ? "\n" : ",";
    }
  }

  for (Gate* g : outputs_) {
    s += g->name_;
    s += g == outputs_.back() ? ";\n" : ",";
  }

  s += "~\n";

  for (auto v : gates_) {
    for (Gate* g : v) {
      for (Gate* in : g->inputs_) {
        s += in->name_ + " -> " + g->name_ + "\n";
      }
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
  for (auto v : gates_) {
    for (Gate* g : v) {
      dotgraph += "\t" + g->name_ + " " + Gate::kDotGraphNodes[g->type_] + "\n";
    }
  }
  for (Gate* g : inputs_) {
    dotgraph += "\t" + g->name_ + " " + Gate::kDotGraphNodes[g->type_] + "\n";
  }
  for (Gate* g : outputs_) {
    dotgraph += "\t" + g->name_ + " " + Gate::kDotGraphNodes[g->type_] + "\n";
  }

  dotgraph += "\n";

  for (auto edge : edges_) {
    dotgraph += "\t" + edge->src_->name_ + " -> " + edge->dst_->name_ + "\n";
  }

  dotgraph += "\n}";
  return dotgraph;
}

Circuit::~Circuit() {
  for (auto v : gates_) {
    for (Gate* g : v) {
      delete g;
    }
  }
  for (Gate* g : inputs_) {
    delete g;
  }
  for (Gate* g : outputs_) {
    delete g;
  }
}
