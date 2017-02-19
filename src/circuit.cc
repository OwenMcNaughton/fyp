#include "circuit.hh"

#include <algorithm>
#include <cmath>
#include <functional>
#include <future>
#include <iostream>
#include <set>
#include <stdexcept>
#include "threadpool.hh"
#include "util.hh"

using namespace std;

map<vector<int>, map<string, int>> Circuit::kTruthTable = {};
vector<int> Circuit::kTruthDecimal = {};

vector<Circuit*> bests;

Circuit::Circuit() {

}

Circuit::Circuit(const string& contents) {
  Load(contents);
}

void Circuit::Evolve() {
  Circuit* circ = new Circuit();
  circ->Load(ReadFile("../circs/2bitmulstarter.circ"));
  SaveDotGraph(circ, "../graphs/", 0);

  vector<Circuit*> historical;
  historical.reserve(Util::kGens);
  historical[0] = circ;
  set<long> hashes = {};
  int stag_count = 0;
  ThreadPool pool(Util::kThreads);
  bests.reserve(Util::kThreads);

  for (int i = 1; i != Util::kGens; i++) {
    vector<future<set<long>>> futures;
    cout << "GEN: " << i << ", Dupes: ";
    vector<set<long>> new_hashes;
    for (int j = 0; j < Util::kThreads; j++) {
      Circuit* circ2 = circ->Copy();
      auto fut = pool.enqueue([circ2, i, j, hashes, &new_hashes]() {
        vector<Circuit*> children;
        set<long> new_hash = Circuit::MakeChildren(circ2, children, i, hashes);
        CircuitSort(children);
        bests[j] = Circuit::GetBestChild(children);
        return new_hash;
      });
      futures.push_back(move(fut));
    }

    int best_count = 0;
    int f = 0;
    for (int j = 0; j < Util::kThreads; j++) {
      auto new_hash = futures[j].get();
      hashes.insert(new_hash.begin(), new_hash.end());
      if (bests[j] && bests[j]->total_count_ >= best_count) {
        circ = bests[j];
        best_count = bests[j]->total_count_;
      }
    }

    circ->BinTruthToDec();

    cout << "\n\tBestTotal: " << circ->total_count_ << " BestExact: " <<
      circ->correct_count_ << " DecimalDiff: " << circ->decimal_diff_ << endl;

    // DetectStagnation(historical, &i, best_count, &stag_count, circ);

    SaveDotGraph(circ, "../graphs/", i + 1);

    if (circ->correct_count_ == pow(2, circ->inputs_.size())) {
      exit(0);
    }
  }
}

set<long> Circuit::MakeChildren(
    Circuit* parent, vector<Circuit*>& children, int gen, const set<long>& hashes) {
  int dupes = 0;
  children.push_back(parent->Copy());
  set<long> new_hashes = {};
  for (int j = 0; j != Util::kChildren; j++) {
    Circuit* child = parent->Copy();
    int m = (rand() % Util::kMutations) + 1;
    for (int k = 0; k != m; k++) {
      child->Mutate();
    }
    long hash = child->Hash();
    if (hashes.count(hash) == 0) {
      children.push_back(child);
      new_hashes.insert(hash);
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
  cout << dupes << " ";
  return new_hashes;
}

void Circuit::DetectStagnation(
    vector<Circuit*>& historical, int* gen, int best_count,
    int* stag_count, Circuit* circ) {
  historical[*gen] = circ;
  if (*gen - Util::kMaxGenStagnation - 1 >= 0) {
    if (historical[*gen - Util::kMaxGenStagnation]->total_count_ >= best_count) {
      if (*stag_count > 3) {
        circ = historical[0];
        *gen = 0;
        *stag_count = 0;
        cout << "STAGNATION, reset to: 0" << endl;
      } else {
        circ = historical[*gen - Util::kMaxGenStagnation - 1];
        cout << "STAGNATION, go to: " << (*gen - Util::kMaxGenStagnation - 1) << endl;
        *gen -= Util::kMaxGenStagnation - 1;
        *stag_count++;
      }
    }
  }
}

struct CircuitTruthSort {
  inline bool operator() (Circuit* circ1, Circuit* circ2) {
    return circ1->total_count_ > circ2->total_count_;
  }
};

struct CircuitDecimalDiffSort {
  inline bool operator() (Circuit* circ1, Circuit* circ2) {
    return circ1->decimal_diff_ < circ2->decimal_diff_;
  }
};

void Circuit::CircuitSort(vector<Circuit*>& children) {
  for (int j = 0; j != children.size(); j++) {
    children[j]->TestAll();
  }

  sort(children.begin(), children.end(), CircuitTruthSort());
}

Circuit* Circuit::GetBestChild(vector<Circuit*>& children) {
  for (int i = 1; i != children.size(); i++) {
    delete children[i];
  }
  children.erase(children.begin() + 1, children.end());
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

Circuit::~Circuit() {
  for (auto& v : gates_) {
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
