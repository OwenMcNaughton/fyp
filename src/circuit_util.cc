#include "circuit.hh"

#include <algorithm>
#include <cmath>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <set>

using namespace std;

Circuit* Circuit::Copy() {
  Circuit* c = new Circuit();

  CopyGates(this, c);

  for (auto& edge : edges_) {
    c->AddEdge(edge->src_->name_, edge->dst_->name_);
  }

  c->gate_count_ = gate_count_;
  c->genome_size_ = genome_size_;

  return c;
}

void Circuit::CopyGates(Circuit* src, Circuit* dst) {
  for (Gate* g : src->inputs_) {
    dst->AddInput(new Gate(g->type_, g->name_, -1));
  }
  for (auto& layer : src->gates_) {
    dst->AddLayer();
    for (Gate* g : layer) {
      dst->AddGate(new Gate(g->type_, g->name_, g->layer_));
    }
  }
  for (Gate* g : src->outputs_) {
    dst->AddOutput(new Gate(g->type_, g->name_, src->gates_.size()));
  }
}

vector<Circuit*> Circuit::MultiBreed(vector<Circuit*>& parents, Circuit* best) {
  vector<Circuit*> multi_breds = {best};
  for (int i = 0; i != Util::kThreads - 1; i++) {
    int idx1 = rand() % parents.size();
    int idx2 = rand() % parents.size();
    while (idx2 == idx1) {
      idx2 = rand() % parents.size();
    }
    vector<Circuit*> pairing = {parents[idx1], parents[idx2]};
    multi_breds.push_back(Breed(pairing));
  }
  return multi_breds;
}

Circuit* Circuit::Breed(vector<Circuit*>& parents) {
  Circuit* c = new Circuit();

  CopyGates(parents[0], c);

  int layer_idx = 0;
  int parent_idx = Util::kBreedGates ? rand() % parents.size() : 0;
  for (auto& layer : c->gates_) {
    int row_idx = 0;
    for (Gate* g : layer) {
      g->inputs_.clear();
      if (Util::kBreedGates) {
        g->type_ = parents[parent_idx]->gates_[layer_idx][row_idx]->type_;
      }
      parent_idx++;
      if (parent_idx == parents.size()) {
        parent_idx = 0;
      }
      row_idx++;
    }
    layer_idx++;
  }

  for (auto& edge : parents[0]->edges_) {
    c->AddEdge(edge->src_->name_, edge->dst_->name_);
  }

  layer_idx = 0;
  parent_idx = rand() % parents.size();
  for (auto& layer : c->gates_) {
    int row_idx = 0;
    for (Gate* g : layer) {
      c->FixMutatedGate(g);
      if (Util::kBreedEdges) {
        Gate* src = parents[parent_idx]->gates_[layer_idx][row_idx];
        for (int idx = 0; idx != src->inputs_.size(); idx++) {
          Gate* new_input = src->inputs_[idx];
          for (int i = 0; i < c->edges_.size(); i++) {
            if (c->edges_[i]->src_ == g->inputs_[idx] && c->edges_[i]->dst_ == g) {
              c->edges_.erase(c->edges_.begin() + i--);
            }
          }
          c->edges_.push_back(new Edge(new_input, g, new_input->layer_, g->layer_));
          g->inputs_[idx] = new_input;
        }
        parent_idx++;
        if (parent_idx == parents.size()) {
          parent_idx = 0;
        }
        row_idx++;
      }
    }
    layer_idx++;
  }

  c->gate_count_ = parents[0]->gate_count_;
  c->genome_size_ = parents[0]->genome_size_;

  return c;
}


void Circuit::Mutate() {
  // vector<function<void()>> legal_mutation_types;
  //
  // if (edges_.size() > 1) {
  //   legal_mutation_types.push_back(bind(&Circuit::MutateRemoveEdge, this));
  // }
  // legal_mutation_types.push_back(bind(&Circuit::MutateNewEdge, this));
  // legal_mutation_types.push_back(bind(&Circuit::MutateExistingGate, this));

  if (rand() % 2 == 0) {
    MutateExistingGate();
  } else {
    // MutateRemoveEdge();
    // MutateNewEdge();
    MutateAnInput();
  }

  // legal_mutation_types[rand() % legal_mutation_types.size()]();
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
  FixMutatedGate(mutated);
}

void Circuit::FixMutatedGate(Gate* mutated) {
  if (mutated->ExpectedInputCount() > mutated->inputs_.size()) {
    Gate* i1 = PickRandomSrc(mutated->layer_);
    AddEdge(i1, mutated);
  }
  if (mutated->ExpectedInputCount() < mutated->inputs_.size()) {
    Gate* to_delete = mutated->inputs_.back();
    for (int i = 0; i < edges_.size(); i++) {
      if (edges_[i]->src_ == to_delete && edges_[i]->dst_ == mutated) {
        edges_.erase(edges_.begin() + i--);
        break;
      }
    }
    mutated->inputs_.pop_back();
  }
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

void Circuit::MutateAnInput() {
  auto layer = gates_[rand() % gates_.size()];
  Gate* g = layer[rand() % layer.size()];

  Gate* new_input = PickRandomSrc(g->layer_);
  int idx = rand() % g->inputs_.size();
  for (int i = 0; i < edges_.size(); i++) {
    if (edges_[i]->src_ == g->inputs_[idx] && edges_[i]->dst_ == g) {
      edges_.erase(edges_.begin() + i--);
    }
  }
  edges_.push_back(new Edge(new_input, g, new_input->layer_, g->layer_));
  g->inputs_[idx] = new_input;
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
  for (auto& v : circ->gates_) {
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
  for (auto& layer : gates_) {
    for (Gate* g : layer) {
      hash = hash * 37 + g->type_;
    }
  }
  for (auto& edge : edges_) {
    hash = hash * 37 + edge->src_->type_;
    hash = hash * 37 + edge->dst_->type_;
  }
  return hash;
}

void Circuit::RemoveGate(const string& name) {
  Gate* to_remove = NULL;
  int layer = 0;
  int idx = 0;
  bool breaker = false;
  for (auto& v : gates_) {
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
  for (Gate* g : inputs_) {
    g->computed_ = false;
    int res = g->Compute() == 1 ? 1 : 0;
    key.push_back(res);
  }
  return key;
}

void Circuit::TestOne() {
  vector<int> key = VectorizeInputs();

  ephemeral_truth_[key] = map<string, int>();
  map<string, int> val = kTruthTable[key];

  for (auto& layer : gates_) {
    for (Gate* g : layer) {
      g->computed_ = false;
    }
  }

  int count = 0;
  for (auto& expected : val) {
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
  total_count_ += count;

  if (count == val.size()) {
    correct_count_++;
  }
}

void Circuit::DetectOrphans() {
  orphan_count_ = 0;
  for (auto& layer : gates_) {
    for (Gate* g : layer) {
      g->orphan_ = false;
      g->IsConnectedToInput();
      if (g->orphan_) {
        orphan_count_++;
      }
    }
  }
}

void Circuit::DetectSuperfluous() {
  DetectOrphans();
  for (auto& l : gates_) {
    for (Gate* g : l) {
      g->childfree_ = true;
    }
  }
  for (Gate* g : outputs_) {
    g->IsConnectedToOutput();
  }
  superfluous_count_ = 0;
  for (auto& l : gates_) {
    for (Gate* g : l) {
      superfluous_count_ += g->childfree_ || g->orphan_ ? 1 : 0;
    }
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

  for (auto& layer : gates_) {
    for (Gate* g : layer) {
      g->computed_ = false;
    }
  }

  for (auto& layer : gates_) {
    for (Gate* g : layer) {
      if (g->orphan_) {
        continue;
      }
      int res = g->Compute();
      if (res == -1) {
        res = 0;
      } else if (res == 0) {
        bad_ = true;
      }
      outputs.push_back(make_pair(g, res));
    }
  }
  // shuffle(outputs.begin(), outputs.end(), default_random_engine{});

  ephemeral_outputs_[key].clear();

  for (auto& expected : val) {
    ephemeral_outputs_[key][expected.first] = {};
    for (auto& output : outputs) {
      if (expected.second == output.second) {
        ephemeral_outputs_[key][expected.first].insert(output.first);
      }
    }
  }
}

void Circuit::AssignBestPinnings() {
  map<string, map<Gate*, int>> best_pinnings;
  for (auto& eo : ephemeral_outputs_) {
    for (pair<string, set<Gate*>> outputs : eo.second) {
      for (auto& correct_gate : outputs.second) {
        if (best_pinnings[outputs.first].count(correct_gate)) {
          best_pinnings[outputs.first][correct_gate]++;
        } else {
          best_pinnings[outputs.first][correct_gate] = 1;
        }
      }
    }
  }
  for (auto& bp : best_pinnings) {
    Gate* best = nullptr;
    int most_right = 0;
    for (auto& pair : bp.second) {
      if (pair.second >= most_right) {
        most_right = pair.second;
        best = pair.first;
      }
    }
    if (best) {
      best_pinnings_[bp.first] = best;
      for (Gate* g : outputs_) {
        if (g->name_ == bp.first) {
          RemoveEdge(g->name_);
          AddEdge(best, g);
        }
      }
    }
  }
}

void Circuit::TestAll() {
  correct_count_ = 0;
  total_count_ = 0;
  total_weighted_count_ = 0;
  if (Util::kPruneOrphans) {
    DetectOrphans();
  }

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
  for (auto& v : gates_) {
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
  if (dst_gate->CanTakeInput()) {
    AddEdge(src_gate, dst_gate);
  }
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
  vector<string> truth = Split(split[split.size() == 4 ? 3 : 2], "\n");
  if (split.size() == 4) {
    vector<string> legal_gate_types = Split(split[2], ",");
    Util::kLegalGateTypes.clear();
    for (auto& lgt : legal_gate_types) {
      Util::kLegalGateTypes.push_back(atoi(lgt.c_str()));
    }
  }

  for (auto& s : inputs) {
    Gate* g = new Gate(Gate::kOnn, s, -1);
    AddInput(g);
  }
  if (nodes.size() == 4) {
    if (nodes[2].find("$") != string::npos) {
      // Specifying rows and column counts
      vector<string> cr = Split(nodes[2], "$");
      for (int i = 0; i != atoi(cr[0].c_str()); i++) {
        vector<Gate*> layer;
        gates_.push_back(layer);
        for (int j = 0; j != atoi(cr[1].c_str()); j++) {
          Gate* g = new Gate(
            Util::kLegalGateTypes[rand() % Util::kLegalGateTypes.size()],
            to_string(rand()), i);
          AddGate(g);
        }
      }
    } else {
      // Specifying each row size
      if (nodes[2].find("#") != string::npos) {
        vector<string> cr = Split(nodes[2], "#");
        vector<string> rc = Split(cr[1], ",");
        int i = 0;
        for (auto& c : rc) {
          vector<Gate*> layer;
          gates_.push_back(layer);
          vector<string> wat = Split(c, ";");
          int type = rand() % 4;
          bool mutateable = true;
          if (wat.size() > 1) {
            type = atoi(wat[1].c_str());
            mutateable = false;
          }
          for (int j = 0; j != atoi(wat[0].c_str()); j++) {
            Gate* g = new Gate(
              Util::kLegalGateTypes[rand() % Util::kLegalGateTypes.size()],
              to_string(rand()), i, mutateable);
            AddGate(g);
          }
          i++;
        }
      }
    }
  } else {
    // Specifying gates precisely
    for (int i = 2; i != nodes.size(); i++) {
      vector<string> gates = Split(Strip(nodes[i], '\n'), ",");
      if (gates[0].size() < 1) {
        continue;
      }
      vector<Gate*> layer;
      gates_.push_back(layer);
      for (auto& s : gates) {
        vector<string> node = Split(s, " ");
        if (node.size() < 2) {
          continue;
        }
        Gate* g = new Gate(atoi(node[1].c_str()), node[0], i - 2);
        AddGate(g);
      }
    }
  }
  for (auto& s : outputs) {
    Gate* g = new Gate(Gate::kBuf, Strip(s, '\n'), nodes.size() - 2);
    AddOutput(g);
  }

  for (auto& s : edges) {
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

  int input_size = atoi(truth[1].c_str());
  vector<vector<string>> truth_table;
  for (int i = 2; i < truth.size() - 1; i++) {
    if (truth[i].size() > 2) {
      vector<string> row = Split(truth[i], ",");
      truth_table.push_back(row);
    }
  }
  Circuit::kTruthTable = FormatTruthTable(truth_table, input_size);
  Circuit::kTruthDecimal = FormatTruthDecimal(
    Circuit::kTruthTable, input_size, outputs_);
}

string Circuit::DotGraph() {
  DetectSuperfluous();
  string dotgraph = "digraph {\n";
  int out_of = pow(2, inputs_.size());
  int total_out_of = out_of * outputs_.size();
  dotgraph += "rankdir=LR\n\tlabelloc=\"t\"\nlabel=\"" + PrintTruth() +
    to_string(correct_count_) + " / " + to_string(out_of) + "\n" +
    to_string(total_count_) + " / " + to_string(total_out_of) +
    "\nGates used: " + to_string(gate_count_) + "\"\n";
  // dotgraph += "labelloc=\"\t\"\nlabel=\"Percent: " + to_string(percent_) +
  //   ", used " + to_string(gate_count_ - superfluous_count_) + " out of " +
  //   to_string(gate_count_) + "\"\n";
  for (auto& v : gates_) {
    for (Gate* g : v) {
      string node_type = g->orphan_ || g->childfree_
        ? Gate::kDotGraphOrphanNode
        : Gate::kDotGraphNodes[g->type_];
      // string node_type = Gate::kDotGraphNodes[g->type_];
      if (g->orphan_ || g->childfree_) {
        if (Util::kPrintOrphans) {
          dotgraph += "\t" + g->name_ + " " + node_type + "\n";
        }
      } else {
        dotgraph += "\t" + g->name_ + " " + node_type + "\n";
      }
    }
  }
  for (Gate* g : inputs_) {
    dotgraph += "\t" + g->name_ + " " + Gate::kDotGraphNodes[g->type_] + "\n";
  }
  for (Gate* g : outputs_) {
    dotgraph += "\t" + g->name_ + " " + Gate::kDotGraphNodes[g->type_] + "\n";
  }

  dotgraph += "\n";

  for (auto& edge : edges_) {
    if (edge->src_->orphan_ || edge->src_->childfree_ ||
        edge->dst_->orphan_ || edge->dst_->childfree_) {
      if (Util::kPrintOrphans) {
        dotgraph += "\t" + edge->src_->name_ + " -> " + edge->dst_->name_ + "\n";
      }
    } else {
      dotgraph += "\t" + edge->src_->name_ + " -> " + edge->dst_->name_ + "\n";
    }
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
  for (auto& p : ephemeral_truth_) {
    for (int i : p.first) {
      truth += to_string(i) + " ";
    }
    for (Gate* g : outputs_) {
      truth += to_string(p.second[g->name_]) + " ";
    }
    bool all_correct = true;
    for (auto& result : kTruthTable[p.first]) {
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

void Circuit::MessUp(int factor) {
  for (int i = 0; i != factor; i++) {
    Mutate();
  }
}

// TODO fix for >2 input gates
void Circuit::FillEdges() {
  for (auto& i : inputs_) {

  }

  for (auto& l : gates_) {
    for (Gate* g : l) {
      int count = g->ExpectedInputCount();
      for (int i = 0; i != count; i++) {
        if (!g->CanTakeInput()) {
          break;
        }
        Gate* i1 = PickRandomSrc(g->layer_);
        AddEdge(i1, g);
      }
    }
  }
}

Gate* Circuit::PickRandomSrc(int end_before) {
  if (rand() % gates_.size() == 0 || end_before == 0) {
    return inputs_[rand() % inputs_.size()];
  } else {
    auto layer = gates_[rand() % end_before];
    return layer[rand() % layer.size()];
  }
}

void Circuit::BinTruthToDec() {
  truth_decimal_.clear();
  int i = 0;
  total_weighted_count_ = 0;
  for (auto& pair : ephemeral_truth_) {
    int output_val = 0;
    int significance = 1;
    for (Gate* g : outputs_) {
      if (pair.second[g->name_] == kTruthTable[pair.first][g->name_]) {
        total_weighted_count_ += significance;
      }
      significance *= 2;
      output_val = output_val << 1;
      output_val += pair.second[g->name_];
    }
    total_weighted_count_++;
    truth_decimal_.push_back(output_val);
    i++;
  }

  decimal_diff_ = 0;
  for (int i = 0; i != kTruthDecimal.size(); i++) {
    decimal_diff_ += abs(kTruthDecimal[i] - truth_decimal_[i]);
  }
}
