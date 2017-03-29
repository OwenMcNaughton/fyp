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
  genome_size_ = gate_count_ + (gate_count_ + outputs_.size()) * 2;
}

void Circuit::Evolve(const string& target) {
  Circuit* circ = new Circuit(ReadFile("../circs/" + target + ".circ"));
  vector<Circuit*> circs;

  EvolutionLog elog(circ);

  if (Util::kSaveDotGraphs) {
    SaveDotGraph(circ, "../graphs/", "original", elog);
  }
  circ->MessUp(Util::kMessUp);
  circ->FillEdges();
  if (Util::kSaveDotGraphs) {
    SaveDotGraph(circ, "../graphs/", 0, elog);
  }

  int evaluations = 0;
  ThreadPool pool(Util::kThreads);

  for (int i = 1;; i++) {
    vector<future<GenerationLog>> futures;
    cout << "GEN: " << i << ", Dupes: ";
    if (i != 1 && Util::kBreedType != Util::kBreedTypeDisable) {
      if (Util::kBreedType == Util::kBreedTypeAbsPoly ||
          Util::kBreedType == Util::kBreedTypePerPoly) {
        circ = Breed(circs);
      }
    }
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
        if (Util::kBreedType == Util::kBreedTypeDisable) {
          glog.best_ = Circuit::GetBestChild(children);
        } else {
          glog.bests_ = Circuit::GetBestChildren(children);
          glog.best_ = glog.bests_[0];
        }
        return glog;
      });
      futures.push_back(move(fut));
    }

    int best_count = circ->total_count_;
    vector<GenerationLog> glogs;
    for (int j = 0; j < Util::kThreads; j++) {
      auto glog = futures[j].get();
      elog.hashes_.insert(glog.hashes_.begin(), glog.hashes_.end());
      if (glog.best_ && glog.best_->total_count_ >= best_count ||
          Util::kBreedType != 0) {
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
    Circuit* newcirc = elog.DetectStagnation();
    circ = newcirc;
    if (Util::kBreedType != Util::kBreedTypeDisable) {
      circs = elog.generations_.back().bests_;
    }

    circ->BinTruthToDec();

    float actual = circ->total_count_ / float(elog.goal_total_count_);
    cout << "\n\tBestTotal: " << circ->total_count_ << " BestExact: " <<
      circ->correct_count_ << " DecimalDiff: " << circ->decimal_diff_ <<
      "\tTOTAL_PERCENT: " << actual << endl;
    circ->percent_ = actual;

    if (Util::kSaveDotGraphs) {
      SaveDotGraph(circ, "../graphs/", i + 1, elog);
    }

    float thresh = Util::kThreshold / 1000.0f;
    if (actual >= thresh) {
      exit(0);
    }
    evaluations += Util::kThreads * Util::kChildren;
    if (evaluations > Util::kEvaluations) {
      exit(0);
    }
  }
}

GenerationLog Circuit::MakeChildren(
    Circuit* parent, vector<Circuit*>& children,
    int gen, const EvolutionLog& elog) {
  children.push_back(parent->Copy());
  GenerationLog glog;
  int mutation_count = 0;
  if (Util::kMutationMode == Util::kMutationModeFixed) {
    mutation_count = Util::kMutations;
  } else if (Util::kMutationMode == Util::kMutationModePercent) {
    mutation_count = int((Util::kMutatePercent / 100.0) * parent->genome_size_);
  }
  for (int j = 0; j != Util::kChildren; j++) {
    Circuit* child = parent->Copy();
    if (Util::kMutationMode == Util::kMutationModeRandom){
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

vector<Circuit*> Circuit::GetBestChildren(vector<Circuit*>& children) {
  int bt = Util::kBreedType;
  int n = bt == Util::kBreedTypeAbsMono || bt == Util::kBreedTypeAbsPoly
    ? Util::kBreedSample
    : children.size() * float(Util::kBreedSample / float(children.size()));
  n = n % 2 == 0 ? n : n - 1;
  n = n == 0 ? 1 : n;
  for (int i = n; i != children.size(); i++) {
    delete children[i];
  }
  children.erase(children.begin() + 1, children.end());
  return children;
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
