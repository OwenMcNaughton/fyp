#include "circuit.hh"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <set>
#include "util.hh"

using namespace std;

map<vector<int>, map<string, int>> Circuit::kTruthTable = {};
set<long> Circuit::hashes = {};

Circuit::Circuit() {

}

Circuit::Circuit(const string& contents) {
  Load(contents);
}

Circuit* Circuit::Copy() {
  Circuit* c = new Circuit();
  c->Load(Serialize());
  return c;
}

void Circuit::Mutate() {
  vector<function<void()>> legal_mutation_types;

  if (edges_.size() > 1) {
    legal_mutation_types.push_back(bind(&Circuit::MutateRemoveEdge, this));
  }
  legal_mutation_types.push_back(bind(&Circuit::MutateNewEdge, this));
  legal_mutation_types.push_back(bind(&Circuit::MutateExistingGate, this));

  legal_mutation_types[rand() % legal_mutation_types.size()]();
}

void Circuit::MutationSeries() {
  int new_gate_count = rand() % 5 + 1;
  int del_edge_count = rand() % 9 + 1;
  int new_edge_count = rand() % 9 + 1;
  for (int i = 0; i != new_gate_count; i++) {
    MutateNewGate();
  }
  for (int i = 0; i != del_edge_count; i++) {
    MutateRemoveEdge();
  }
  for (int i = 0; i != new_edge_count; i++) {
    MutateNewEdge();
  }
}

void Circuit::MutateNewLayer() {
  AddLayer();
}

void Circuit::MutateNewGate() {
  if (gates_.empty()) {
    MutateNewLayer();
  }
  int layer = rand() % gates_.size();
  Gate* g = new Gate(Gate::kGates[rand() % 4], to_string(rand()), layer);
  AddGate(g);
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
  RemoveGate(gates_[layer][idx]->name_);
}

void Circuit::MutateNewEdge() {
  if (gates_.empty()) {
    return;
  }

  auto edge = MakeRandomEdge();

  if (edge.first && edge.second) {
    for (Gate* g : edge.second->inputs_) {
      if (g == edge.first) {
        return;
      }
    }
    AddEdge(edge.first, edge.second);
  }
}

void Circuit::MutateRemoveEdge() {
  if (edges_.empty()) {
    return;
  }
  Edge* e = edges_[rand() % edges_.size()];
  for (int j = 0; j < e->dst_->inputs_.size(); j++) {
    if (e->dst_->inputs_[j] == e->src_) {
      e->dst_->inputs_.erase(e->dst_->inputs_.begin() + j);
    }
  }
  for (int i = 0; i < edges_.size(); i++) {
    if (edges_[i] == e) {
      edges_.erase(edges_.begin() + i--);
    }
  }
}

void Circuit::RemoveEdge(const string& name) {
  for (int i = 0; i < edges_.size(); i++) {
    if (edges_[i]->src_->name_ == name || edges_[i]->dst_->name_ == name) {
      for (int j = 0; j < edges_[i]->dst_->inputs_.size(); j++) {
        if (edges_[i]->dst_->inputs_[j]->name_ == name) {
          edges_[i]->dst_->inputs_.erase(edges_[i]->dst_->inputs_.begin() + j);
        }
      }
      edges_.erase(edges_.begin() + i--);
    }
  }
}

void Circuit::RemoveEdge(Gate* node) {
  for (int i = 0; i < edges_.size(); i++) {
    if (edges_[i]->src_ == node || edges_[i]->dst_ == node) {
      for (int j = 0; j < edges_[i]->dst_->inputs_.size(); j++) {
        if (edges_[i]->dst_->inputs_[j] == node) {
          edges_[i]->dst_->inputs_.erase(edges_[i]->dst_->inputs_.begin() + j);
        }
      }
      edges_.erase(edges_.begin() + i--);
    }
  }
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

long Circuit::Hash() {
  long hash = 37;
  for (auto layer : gates_) {
    for (Gate* g : layer) {
      hash = hash * 37 + g->type_;
    }
  }
  for (auto edge : edges_) {
    hash = hash * 37 + edge->src_->type_;
    hash = hash * 37 + edge->dst_->type_;
  }
  return hash;
}

void Circuit::Evolve() {
  Circuit* circ = new Circuit();
  circ->Load(ReadFile("../circs/full_adder_starter.circ"));

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
  SaveDotGraph(circ, "../graphs/", 0);

  hashes = {};

  for (int i = 0; i != kGens; i++) {
    cout << "GEN: " << i << endl;
    vector<Circuit*> children;
    MakeChildren(circ, children, i);

    Circuit* best = GetBestChild(children);

    circ = best->Copy();
    circ->TestAll();
    SaveDotGraph(circ, "../graphs/", i + 1);

    if (circ->correct_count_ == 8) {
      exit(0);
    }

    for (int j = 0; j < children.size(); j++) {
      delete children[j];
    }
  }
}

void Circuit::MakeChildren(Circuit* parent, vector<Circuit*>& children, int gen) {
  int dupes = 0;
  children.push_back(parent->Copy());
  for (int j = 0; j != kChildren; j++) {
    Circuit* child = parent->Copy();
    int m = rand() % kMutations;
    for (int k = 0; k != m; k++) {
      child->Mutate();
    }
    long hash = child->Hash();
    if (hashes.count(hash) == 0) {
      children.push_back(child);
      hashes.insert(hash);
    } else {
      dupes++;
      if (dupes > 100000) {
        // exit(0);
        break;
      }
      delete child;
      j--;
    }
  }
  cout << dupes << endl;
}

struct CircuitTruthSort {
  inline bool operator() (Circuit* circ1, Circuit* circ2) {
    return circ1->correct_count_ > circ2->correct_count_;
  }
};

Circuit* Circuit::GetBestChild(vector<Circuit*>& children) {
  int correct_max = -1;
  for (int j = 0; j != children.size(); j++) {
    children[j]->TestAll();
  }

  sort(children.begin(), children.end(), CircuitTruthSort());

  return children[0];
}

void Circuit::AddInput(Gate* g) {
  inputs_.push_back(g);
}

void Circuit::AddOutput(Gate* g) {
  outputs_.push_back(g);
}

void Circuit::AddGate(Gate* g) {
  gates_[g->layer_].push_back(g);
}

void Circuit::RemoveGate(const string& name) {
  Gate* to_remove = NULL;
  int layer = 0;
  int idx = 0;
  bool breaker = false;
  for (auto v : gates_) {
    idx = 0;
    for (Gate* g : v) {
      if (g->name_ == name) {
        to_remove = g;
        breaker = true;
        break;
      }
      idx++;
    }
    if (breaker) {
      break;
    }
    layer++;
  }

  RemoveEdge(to_remove);

  delete to_remove;
  gates_[layer].erase(gates_[layer].begin() + idx);
  if (gates_[layer].empty()) {
    // gates_.erase(gates_.begin() + layer);
  }
}

void Circuit::AddLayer() {
  vector<Gate*> layer;
  gates_.push_back(layer);
}

vector<int> Circuit::VectorizeInputs() {
  vector<int> key;
  string kk = "";
  for (Gate* g : inputs_) {
    int res = g->Compute() == 1 ? 1 : 0;
    kk += to_string(res) + " ";
    key.push_back(res);
  }
  return key;
}

void Circuit::TestOne() {
  vector<int> key = VectorizeInputs();

  ephemeral_truth_[key] = map<string, int>();
  map<string, int> val = kTruthTable[key];

  int count = 0;
  for (auto expected : val) {
    for (pair<string, Gate*> pair : best_pinnings_) {
      if (pair.first == expected.first) {
        int res = pair.second->Compute();
        if (res == -1) {
          res = 0;
        } else if (res == 0) {
          bad_ = true;
        }
        if (res == expected.second) {
          count++;
        }
        ephemeral_truth_[key][pair.first] = res;
      }
    }
  }

  if (count == val.size()) {
    correct_count_++;
  }
}

void Circuit::FindBestPinningsIter(int idx) {
  if (idx == inputs_.size() - 1) {
    inputs_[idx]->type_ = Gate::kOff;
    FindBestPinningsOne();
    inputs_[idx]->type_ = Gate::kOnn;
    FindBestPinningsOne();
  } else {
    inputs_[idx]->type_ = Gate::kOff;
    FindBestPinningsIter(idx + 1);
    inputs_[idx]->type_ = Gate::kOnn;
    FindBestPinningsIter(idx + 1);
  }
}

void Circuit::FindBestPinningsOne() {
  vector<int> key = VectorizeInputs();

  map<string, int> val = kTruthTable[key];
  vector<pair<Gate*, int>> outputs;

  for (auto layer : gates_) {
    for (Gate* g : layer) {
      int res = g->Compute();
      if (res == -1) {
        res = 0;
      } else if (res == 0) {
        bad_ = true;
      }
      outputs.push_back(make_pair(g, res));
    }
  }
  shuffle(outputs.begin(), outputs.end(), default_random_engine{});

  ephemeral_outputs_[key].clear();

  for (auto expected : val) {
    ephemeral_outputs_[key][expected.first] = {};
    for (auto output : outputs) {
      if (expected.second == output.second) {
        ephemeral_outputs_[key][expected.first].insert(output.first);
      }
    }
  }
}

void Circuit::AssignBestPinnings() {
  map<string, map<Gate*, int>> best_pinnings;
  for (auto eo : ephemeral_outputs_) {
    for (pair<string, set<Gate*>> outputs : eo.second) {
      for (auto correct_gate : outputs.second) {
        if (best_pinnings[outputs.first].count(correct_gate)) {
          best_pinnings[outputs.first][correct_gate]++;
        } else {
          best_pinnings[outputs.first][correct_gate] = 1;
        }
      }
    }
  }
  for (auto bp : best_pinnings) {
    Gate* best = nullptr;
    int most_right = 0;
    for (auto pair : bp.second) {
      if (pair.second >= most_right) {
        most_right = pair.second;
        best = pair.first;
      }
    }
    if (best) {
      best_pinnings_[bp.first] = best;
      for (Gate* g : outputs_) {
        if (g->name_ == bp.first) {
          // g->inputs_.clear();
          RemoveEdge(g->name_);
          AddEdge(best, g);
        }
      }
    }
  }
}

void Circuit::TestAll() {
  correct_count_ = 0;
  FindBestPinningsIter(0);
  AssignBestPinnings();

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
  int dst_layer = gates_.size();
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
    cout << "\t!! [" << src << "] -> " << dst << " not found" << endl;
    exit(1);
  }
  if (dst_gate == NULL) {
    cout << "\t!! " << src << " -> [" << dst << "] not found" << endl;
    exit(1);
  }
  AddEdge(src_gate, dst_gate);
}

void Circuit::AddEdge(Gate* src, Gate* dst) {
  edges_.push_back(new Edge(src, dst, src->layer_, dst->layer_));
  dst->AddInput(src);
}

void Circuit::Load(const string& contents) {
  vector<string> split = Split(contents, "~");
  vector<string> nodes = Split(split[0], "\n");
  vector<string> inputs = Split(nodes[0], ",");
  vector<string> outputs = Split(nodes[1], ",");
  vector<string> edges = Split(split[1], "\n");

  for (auto s : inputs) {
    Gate* g = new Gate(Gate::kOnn, s, -1);
    AddInput(g);
  }
  for (int i = 2; i != nodes.size(); i++) {
    vector<string> gates = Split(Strip(nodes[i], '\n'), ",");
    if (gates[0].size() < 1) {
      continue;
    }
    vector<Gate*> layer;
    gates_.push_back(layer);
    for (auto s : gates) {
      vector<string> node = Split(s, " ");
      if (node.size() < 2) {
        continue;
      }
      Gate* g = new Gate(atoi(node[1].c_str()), node[0], i - 2);
      AddGate(g);
    }
  }
  for (auto s : outputs) {
    Gate* g = new Gate(Gate::kBuf, Strip(s, '\n'), nodes.size() - 2);
    AddOutput(g);
  }

  for (auto s : edges) {
        vector<string> edge = Split(s, "->");
    if (edge.size() < 2) {
      continue;
    }
    string src = Strip(edge[0], ' ');
    vector<string> dsts = Split(edge[1], ",");
    for (string dst : dsts) {
      AddEdge(src, Strip(dst, ' '));
    }
  }
}

string Circuit::Serialize() {
  string s = "";
  for (Gate* g : inputs_) {
    s += g->name_ ;
    s += g == inputs_.back() ? "\n" : ",";
  }

  for (Gate* g : outputs_) {
    s += g->name_;
    s += g == outputs_.back() ? "\n" : ",";
  }

  for (int layer = 0; layer < gates_.size(); layer++) {
    for (int gate = 0; gate < gates_[layer].size(); gate++) {
      s += gates_[layer][gate]->name_ + " " + to_string(gates_[layer][gate]->type_);
      if (gate == gates_[layer].size() - 1 && layer != gates_.size() - 1) {
        s += "\n";
      } else {
        s += ",";
      }
    }
    if (layer == gates_.size() - 1) {
      s += "\n";
    }
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
  int out_of = pow(2, inputs_.size());
  dotgraph += "labelloc=\"t\"\nlabel=\"" + PrintTruth() +
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

void Circuit::PrintLayout() {
  for (Gate* in : outputs_) {
    cout << "OUTPUT: " << in->name_ << endl;
    in->PrintLayout(1);
  }
}

string Circuit::PrintTruth() {
  string truth = "";
  for (Gate* g : inputs_) {
    truth += g->name_ + ",";
  }
  for (Gate* g : outputs_) {
    truth += g->name_ + ",";
  }
  truth += "\n";
  for (auto p : ephemeral_truth_) {
    for (int i : p.first) {
      truth += to_string(i) + " ";
    }
    for (Gate* g : outputs_) {
      truth += to_string(p.second[g->name_]) + " ";
    }
    bool all_correct = true;
    for (auto result : kTruthTable[p.first]) {
      if (ephemeral_truth_[p.first][result.first] != result.second) {
        all_correct = false;
      }
    }
    truth += all_correct ? "+" : "-";
    truth += "\n";
  }
  return truth;
}

Gate* Circuit::PickRandomFromInputs() {
  return inputs_[rand() % inputs_.size()];
}

Gate* Circuit::PickRandomFromOutputs() {
  return outputs_[rand() % outputs_.size()];
}

Gate* Circuit::PickRandomFromLayersStartingFrom(int layer) {
  auto picked_layer = gates_[rand() % (gates_.size() - layer) + layer];
  if (picked_layer.empty()) {
    return nullptr;
  }
  return picked_layer[rand() % picked_layer.size()];
}

Gate* Circuit::PickRandomFromLayersEndingBefore(int layer) {
  auto picked_layer = gates_[rand() % (gates_.size() - 1)];
  if (picked_layer.empty()) {
    return nullptr;
  }
  return picked_layer[rand() % picked_layer.size()];
}

Gate* Circuit::PickRandomEdgeSrc(int end_before) {
  if (rand() % gates_.size() == 0) {
    return PickRandomFromInputs();
  } else {
    return PickRandomFromLayersEndingBefore(end_before);
  }
}

Gate* Circuit::PickRandomEdgeDst(int start_at) {
  return PickRandomFromLayersStartingFrom(start_at == -1 ? 0 : start_at);
}

pair<Gate*, Gate*> Circuit::MakeRandomEdge() {
  Gate* src = PickRandomEdgeSrc(gates_.size());
  if (!src) {
    return make_pair(nullptr, nullptr);
  }
  Gate* dst = PickRandomEdgeDst(src->layer_ + 1);
  if (dst && dst->type_ == Gate::kBuf && !dst->inputs_.empty()) {
    dst = nullptr;
  }
  if (!dst->CanTakeInput()) {
    dst = nullptr;
  }
  return make_pair(src, dst);
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
  for (Edge* e : edges_) {
    delete e;
  }
}
