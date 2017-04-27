#include "circuit.hh"

#include <algorithm>
#include <cmath>
#include <functional>
#include <future>
#include <iostream>
#include <set>
#include <stdexcept>
#include "threadpool.hh"
#include <thread>
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
  vector<Circuit*> multibred_circs;

  EvolutionLog elog(circ);

  if (Util::kSaveDotGraphs) {
    SaveDotGraph(circ, "../graphs/", "original", elog);
  }
  circ->MessUp(Util::kMessUp);
  circ->FillEdges();
  if (Util::kSaveDotGraphs) {
    SaveDotGraph(circ, "../graphs/", 0, elog);
  }

  circ->TestAll();

  int evaluations = 0;
  ThreadPool pool(Util::kThreads);

  for (int i = 1;; i++) {
    vector<future<GenerationLog>> futures;
    if (Util::kBasicLog) {
      cout << "GEN: " << i << ", Dupes: ";
    }
    if (i != 1 && Util::kBreedType != Util::kBreedTypeDisable) {
      if (Util::kBreedType == Util::kBreedTypeAbsPoly ||
          Util::kBreedType == Util::kBreedTypePerPoly) {
        vector<Circuit*> c = {circ};
        Circuit* bred = Breed(circs);
        c.push_back(bred);
        sort(c.begin(), c.end());
      } else {
        multibred_circs = MultiBreed(circs, circ);
      }
    }
    for (int j = 0; j < Util::kThreads; j++) {
      Circuit* circ2 = nullptr;
      if (i == 1) {
        circ2 = circ->Copy();
      } else {
        if (Util::kBreedType == Util::kBreedTypeAbsMono ||
            Util::kBreedType == Util::kBreedTypePerMono) {
          circ2 = multibred_circs[j];
        } else {
          circ2 = circ->Copy();
        }
      }
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
          glog.bests_ = {glog.best_};
        } else {
          glog.bests_ = Circuit::GetBestChildren(children);
          glog.best_ = glog.bests_[0];
        }
        glog.best_->TestAll();
        if (Util::kTruthWeight) {
          glog.best_->BinTruthToDec();
        }
        return glog;
      });
      futures.push_back(move(fut));
    }

    if (Util::kTruthWeight) {
      circ->BinTruthToDec();
    }
    int best_count = 0;
    if (Util::kTruthWeight) {
      best_count = circ->total_weighted_count_;
    } else {
      best_count = circ->total_count_;
    }
    vector<GenerationLog> glogs;
    for (int j = 0; j < Util::kThreads; j++) {
      auto glog = futures[j].get();
      elog.hashes_.insert(glog.hashes_.begin(), glog.hashes_.end());
      if (Util::kTruthWeight) {
        if (glog.best_ && glog.best_->total_weighted_count_ >= best_count) {
          circ = glog.best_;
          best_count = glog.best_->total_weighted_count_;
        }
      } else {
        if (glog.best_ && glog.best_->total_count_ >= best_count) {
          circ = glog.best_;
          best_count = glog.best_->total_count_;
        }
      }
      glogs.push_back(glog);
    }
    float actual = circ->total_count_ / float(elog.goal_total_count_);
    float weighted_actual = circ->total_weighted_count_ / float(elog.goal_total_weighted_count_);
    cout << circ->total_weighted_count_ << " / " << elog.goal_total_weighted_count_ << "  =  " << weighted_actual << endl; 
    circ->percent_ = actual;
    circ->weighted_percent_ = weighted_actual;
    evaluations += Util::kChildren * Util::kThreads;
    if (Util::kLog){
      GenerationLog merged_glog(glogs, circ);
      elog.generations_.push_back(merged_glog);
      elog.SaveFullLog(evaluations, "final");
    }
    Circuit* newcirc = elog.DetectStagnation();
    circ = newcirc;
    if (Util::kBreedType != Util::kBreedTypeDisable) {
      circs = elog.generations_.back().bests_;
      cout << "\nLEN: " << circs.size() << endl;
    }
    elog.total_history_.push_back(circ->total_count_);
    elog.weighted_total_history_.push_back(circ->total_weighted_count_);
    elog.percent_history_.push_back(circ->percent_);
    elog.weighted_percent_history_.push_back(circ->weighted_percent_);

    if (Util::kBasicLog) {
      cout << "\n\tBestTotal: " << circ->total_count_ << " BestExact: " <<
        circ->correct_count_ << " WeightedCount: " << circ->total_weighted_count_ <<
        "\tTOTAL_PERCENT: " << actual << "  WEIGHTED_PERCENT: " << weighted_actual << endl;
    }
    cout << "EVALS: " << evaluations << endl;

    if (Util::kSaveDotGraphs) {
      SaveDotGraph(circ, "../graphs/", i + 1, elog);
    }

    if (elog.generations_.size() > 10) {
      elog.generations_.erase(elog.generations_.begin(), elog.generations_.begin() + 8);
    }

    float thresh = Util::kThreshold / 1000.0f;
    if (actual >= thresh) {
      cout << "Got it in gen " << i << " after " << evaluations << " evals" << endl;
      elog.SaveFullLog(evaluations, "final");
      return;
    }
    if (evaluations > Util::kEvaluations) {
      cout << "Failed after " << i << " gens" << endl;
      elog.SaveFullLog(evaluations, "final");
      return;
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
  if (Util::kBasicLog) {
    cout << glog.dupes_ << " ";
  }
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

struct CircuitWeightedTruthSort {
  inline bool operator() (Circuit* circ1, Circuit* circ2) {
    return circ1->total_weighted_count_ > circ2->total_weighted_count_;
  }
};

void Circuit::CircuitSort(vector<Circuit*>& children) {
  for (int j = 0; j != children.size(); j++) {
    children[j]->TestAll();
    if (Util::kTruthWeight) {
      children[j]->BinTruthToDec();
    }
  }
  if (Util::kTruthWeight) {
    sort(children.begin(), children.end(), CircuitWeightedTruthSort());
  } else {
    sort(children.begin(), children.end(), CircuitTruthSort());
  }
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
  for (int i = 1; i != children.size(); i++) {
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
