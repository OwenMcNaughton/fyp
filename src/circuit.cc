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
  int mutation_type = rand() % 3;
  mutation_type = 1;
  switch (mutation_type) {
    case 0: MutateExistingGate(); break;
    case 1: MutateEdgeSource(); break;
    case 2: MutateEdgeDestination(); break;
  }
  return;

  if (mutation_type == 0) {
    MutateExistingGate();
  } else if (mutation_type == 1) {
    Gate* g = new Gate(Gate::kGates[rand() % 4], to_string(rand()));
    gates_.push_back(g);
  } else if (mutation_type == 44 && !gates_.empty()) {
    int idx = rand() % gates_.size();
    Gate* g = gates_[idx];


    for (int i = 0; i != edges_.size(); i++) {
      if (edges_[i].first == g || edges_[i].second == g) {
        edges_.erase(edges_.begin() + i--);
      }
    }
    for (int i = 0; i != edges_.size(); i++) {
      if (edges_[i].first == g || edges_[i].second == g) {
        edges_.erase(edges_.begin() + i--);
      }
    }

    for (Gate* gg : gates_) {
      for (int i = 0; i != gg->inputs_.size(); i++) {
        if (gg->inputs_[i] == g) {
          gg->inputs_.erase(gg->inputs_.begin() + i--);
        }
      }
    }
    for (Gate* gg : gates_) {
      for (int i = 0; i != gg->inputs_.size(); i++) {
        if (gg->inputs_[i] == g) {
          gg->inputs_.erase(gg->inputs_.begin() + i--);
        }
      }
    }

    gates_.erase(gates_.begin() + idx);
  } else if (mutation_type == 3) {
    auto pair = make_pair(gates_[rand() % gates_.size()], gates_[rand() % gates_.size()]);
    edges_.push_back(pair);
  } else if (mutation_type == 4) {
    edges_[rand() % edges_.size()].second = gates_[rand() % gates_.size()];
  } else if (mutation_type == 5) {
    edges_[rand() % edges_.size()].first = gates_[rand() % gates_.size()];
  } else if (mutation_type == 6) {
    edges_.erase(edges_.begin() + (rand() % edges_.size()));
  }

  int tt = rand() % 5;
  if (tt == 0) {
    int idd = rand() % outputs_.size();
    Gate* gg = outputs_[idd];
    gg->inputs_.clear();
    gg->inputs_.push_back(gates_[rand() % gates_.size()]);
    for (int i = 0; i != edges_.size(); i++) {
      if (edges_[i].second == gg) {
        edges_.erase(edges_.begin() + i--);
      }
    }
    edges_.push_back(make_pair(gg->inputs_[0], gg));
  }
}

void Circuit::MutateExistingGate() {
  Gate* mutated = gates_[rand() % gates_.size()];
  mutated->Mutate();
}

void Circuit::MutateEdgeSource() {
  if (edges_.empty()) {
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
  if (edges_.empty()) {
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
  vector<Circuit*> children;
  for (int i = 0; i != 100; i++) {
    children.push_back(Copy());
    // children.push_back(Mutate());
  }

  // PrintLayout();
  TestAll();
  PrintCorrectness();

  Circuit* cc = Copy();
  // cc->PrintLayout();
  cc->TestAll();
  cc->PrintCorrectness();
  // cc->TestAll();
  // cc->PrintCorrectness();
  // PrintAll();
  // cc->PrintAll();

  // TestAll();
  // PrintCorrectness();

  // sort(children.begin(), children.end());
  for (Circuit* c : children) {
    // c->TestAll();
    // int out_of = pow(2, c->inputs_.size()) * c->outputs_.size();
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

void Circuit::PrintOne(bool print_legend) {
  if (print_legend) {
    PrintLegend();
  }

  string row = "";
  for (Gate* g : inputs_) {
    row += g->Compute() == 1 ? "1" : "0";
    row += "       ";
  }
  for (Gate* g : outputs_) {
    row += g->Compute() == 1 ? "1" : "0";
    row += "       ";
  }
  cout << row << endl;
}

void Circuit::TestOne() {
  vector<int> key;
  string kk = "";
  for (Gate* g : inputs_) {
    int res = g->Compute() == 1 ? 1 : 0;
    kk += to_string(res) + " ";
    key.push_back(res);
  }

  cout << "######" << endl;
  cout << kk << endl;
  vector<pair<string, int>> val = kTruthTable[key];
  for (auto expected : val) {
    for (Gate* g : outputs_) {
      if (g->name_ == expected.first) {
        int res = g->Compute() == 1 ? 1 : 0;
        cout << g->name_ << ": " << res << " / " << expected.second << endl;
        if (res == expected.second) {
          correct_count_++;
        }
      }
    }
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

void Circuit::PrintLegend() {
  string top_row = "";
  for (Gate* g : inputs_) {
    top_row += g->name_;
    int pad = 8 - g->name_.length();
    for (int i = 0; i != pad; i++) {
      top_row += " ";
    }
  }
  for (Gate* g : outputs_) {
    top_row += g->name_;
    int pad = 8 - g->name_.length();
    for (int i = 0; i != pad; i++) {
      top_row += " ";
    }
  }
  cout << top_row << endl;
}

void Circuit::PrintAll() {
  PrintLegend();
  PrintAllIter(0);
}

void Circuit::PrintAllIter(int idx) {
  if (idx == inputs_.size() - 1) {
    inputs_[idx]->type_ = Gate::kOff;
    PrintOne(false);
    inputs_[idx]->type_ = Gate::kOnn;
    PrintOne(false);
  } else {
    inputs_[idx]->type_ = Gate::kOff;
    PrintAllIter(idx + 1);
    inputs_[idx]->type_ = Gate::kOnn;
    PrintAllIter(idx + 1);
  }
}

void Circuit::PrintLayout() {
  for (Gate* in : outputs_) {
    cout << "OUTPUT: " << in->name_ << endl;
    in->PrintLayout(1);
  }
}

void Circuit::PrintCorrectness() {
  int out_of = pow(2, inputs_.size()) * outputs_.size();
  cout << correct_count_ << " / " << out_of << endl;
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
  dotgraph += "labelloc=\"t\"\n";
  dotgraph += "label=\"" + to_string(correct_count_) + " / " + to_string(out_of) + "\"\n";
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
