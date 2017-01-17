#include "gate.hh"

#include <iostream>


string Gate::kNot = "NOT", Gate::kAnd = "AND",
  Gate::kOrr = "ORR", Gate::kNnd = "NND", Gate::kXor = "XOR",
  Gate::kOnn = "ONN", Gate::kOff = "OFF", Gate::kBuf = "BUF";

vector<string> Gate::kGates = {kNot, kAnd, kOrr, kXor, kNnd, kOnn, kOff, kBuf};

int Gate::kLineOn = 1, Gate::kLineOff = -1, Gate::kLineUnknown = 0;

map<string, string> Gate::kDotGraphNodes = {
  {kAnd, "[shape=invhouse,color=forestgreen,penwidth=2]"},
  {kOrr, "[shape=invtriangle,color=darkorchid,penwidth=2]"},
  {kXor, "[shape=invtriangle,peripheries=2,color=red,penwidth=1]"},
  {kNot, "[shape=invtriangle,color=gold,penwidth=2,width=.3]"},
  {kOnn, "[shape=circle,color=black,penwidth=2]"},
  {kOff, "[shape=circle,color=black,penwidth=2]"},
  {kBuf, "[shape=circle,color=black,penwidth=2]"}
};

Gate::Gate(const string& type, string name)
    : type_(type), name_(name) {

}

Gate* Gate::Copy(map<string, Gate*>& table) {
  Gate* g = new Gate(type_, name_);
  for (Gate* in : inputs_) {
    if (table.count(in->name_)) {
      g->AddInput(table[in->name_]);
    } else {
      Gate* in_copy = in->Copy(table);
      g->AddInput(in_copy);
    }
  }
  return g;
}

void Gate::Mutate() {
  type_ = kGates[rand() % 4];
}

void Gate::AddInput(Gate* in) {
  inputs_.push_back(in);
}

bool Gate::FindLoops(set<Gate*>& seen) {
  if (seen.count(this) > 0) {
    return true;
  } else {
    seen.insert(this);
    for (Gate* g : inputs_) {
      if (g->FindLoops(seen)) {
        return true;
      }
    }
    return false;
  }
}

int Gate::Compute() {
  // cout << "\t\t\tCOMPUTE " << name_ << " " << type_ << " " << inputs_.size() << endl;
  if (type_ == kOnn) {
    return kLineOn;
  }
  if (type_ == kOff) {
    return kLineOff;
  }
  if (type_ == kNot) {
    if (inputs_.size() != 1) {
      return kLineUnknown;
    } else {
      return -inputs_[0]->Compute();
    }
  }
  if (type_ == kAnd) {
    if (inputs_.empty()) {
      return kLineUnknown;
    } else {
      for (Gate* in : inputs_) {
        if (in->Compute() == kLineOff) {
          return kLineOff;
        } else if (in->Compute() == kLineUnknown) {
          return kLineUnknown;
        }
      }
      return kLineOn;
    }
  }
  if (type_ == kOrr) {
    if (inputs_.empty()) {
      return kLineUnknown;
    } else {
      for (Gate* in : inputs_) {
        if (in->Compute() == kLineOn) {
          return kLineOn;
        } else if (in->Compute() == kLineUnknown) {
          return kLineUnknown;
        }
      }
      return kLineOff;
    }
  }
  if (type_ == kXor) {
    if (inputs_.empty()) {
      return kLineUnknown;
    } else {
      bool one_on = false;
      for (Gate* in : inputs_) {
        if (in->Compute() == kLineOn) {
          if (one_on) {
            return kLineOff;
          } else {
            one_on = true;
          }
        } else if (in->Compute() == kLineUnknown) {
          return kLineUnknown;
        }
      }
      return one_on ? kLineOn : kLineOff;
    }
  }
  if (type_ == kBuf) {
    if (inputs_.size() == 1) {
      return inputs_[0]->Compute();
    } else {
      return kLineUnknown;
    }
  }
}

void Gate::PrintLayout(int depth) {
  string tab = "";
  for (int i = 0; i != depth; i++) {
    tab += "  ";
  }
  cout << tab << name_ << " " << type_ << " " << inputs_.size() << endl;
  for (Gate* in : inputs_) {
    in->PrintLayout(depth + 1);
  }
}
