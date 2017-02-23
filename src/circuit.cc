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

Circuit::Circuit() {

}

Circuit::Circuit(const string& contents) {
  Load(contents);
  gate_count_ = 0;
  for (const auto& l : gates_) {
    gate_count_ += l.size();
  }
}

void Circuit::Evolve(const string& target) {
  Circuit* circ = new Circuit();
  circ->Load(ReadFile("../circs/" + target + ".circ"));

  EvolutionLog elog(circ);

  if (Util::kSaveDotGraphs) {
    SaveDotGraph(circ, "../graphs/", "original");
  }
  if (target.find("starter") == string::npos) {
    // circ->MessUp(Util::kMessUp);
  } else {
    // circ->FillEdges();
  }
  if (Util::kSaveDotGraphs) {
    SaveDotGraph(circ, "../graphs/", 0);
  }

  vector<Circuit*> historical;
  historical.reserve(Util::kGens);
  historical[0] = circ;
  int stag_count = 0;
  ThreadPool pool(Util::kThreads);

  for (int i = 1; i != Util::kGens; i++) {
    vector<future<GenerationLog>> futures;
    cout << "GEN: " << i << ", Dupes: ";
    for (int j = 0; j < Util::kThreads; j++) {
      Circuit* circ2 = circ->Copy();
      auto fut = pool.enqueue([circ2, i, j, elog]() {
        vector<Circuit*> children;
        auto glog = Circuit::MakeChildren(circ2, children, i, elog);
        CircuitSort(children);
        if (Util::kLog) {
          for (Circuit* c : children) {
            glog.correct_counts_.push_back(c->correct_count_);
            glog.total_counts_.push_back(c->total_count_);
          }
        }
        glog.best_ = Circuit::GetBestChild(children);
        return glog;
      });
      futures.push_back(move(fut));
    }

    int best_count = circ->total_count_;
    int f = 0;
    vector<GenerationLog> glogs;
    for (int j = 0; j < Util::kThreads; j++) {
      auto glog = futures[j].get();
      elog.hashes_.insert(glog.hashes_.begin(), glog.hashes_.end());
      if (glog.best_ && glog.best_->total_count_ >= best_count) {
        circ = glog.best_;
        best_count = glog.best_->total_count_;
      }
      glogs.push_back(glog);
    }
    if (Util::kLog){
      GenerationLog merged_glog(glogs, circ);
      elog.generations_.push_back(merged_glog);
      elog.SaveLog();
    }

    circ->BinTruthToDec();

    float actual = circ->total_count_ / float(elog.goal_total_count_);
    cout << "\n\tBestTotal: " << circ->total_count_ << " BestExact: " <<
      circ->correct_count_ << " DecimalDiff: " << circ->decimal_diff_ <<
      "\tTOTAL_PERCENT: " << actual << endl;

    // DetectStagnation(historical, &i, best_count, &stag_count, circ);

    if (Util::kSaveDotGraphs) {
      SaveDotGraph(circ, "../graphs/", i + 1);
    }

    float thresh = Util::kThreshold / 1000.0f;
    if (actual >= thresh) {
      exit(0);
    }
  }
}

GenerationLog Circuit::MakeChildren(
    Circuit* parent, vector<Circuit*>& children,
    int gen, const EvolutionLog& elog) {
  children.push_back(parent->Copy());
  GenerationLog glog;
  for (int j = 0; j != Util::kChildren; j++) {
    Circuit* child = parent->Copy();
    int mutation_count = 0;
    if (Util::kMutationCountFixed) {
      mutation_count = Util::kMutations;
    } else {
      mutation_count = (rand() % Util::kMutations) + 1;
    }
    for (int k = 0; k != mutation_count; k++) {
      child->Mutate();
    }
    long hash = child->Hash();
    if (elog.hashes_.count(hash) == 0) {
      children.push_back(child);
      glog.hashes_.insert(hash);
    } else {
      glog.dupes_++;
      if (glog.dupes_ > 100000) {
        // exit(0);
        break;
      }
      delete child;
      j--;
    }
  }
  cout << glog.dupes_ << " ";
  return glog;
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
