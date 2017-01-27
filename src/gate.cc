#include "gate.hh"

#include <iostream>


const int Gate::kNot = 0, Gate::kAnd = 1,
  Gate::kOrr = 2, Gate::kXor = 3, Gate::kNnd = 4,
  Gate::kOnn = 5, Gate::kOff = 6, Gate::kBuf = 7;

vector<int> Gate::kGates = {kNot, kAnd, kOrr, kXor, kNnd, kOnn, kOff, kBuf};

int Gate::kLineOn = 1, Gate::kLineOff = -1, Gate::kLineUnknown = 0;

map<int, string> Gate::kDotGraphNodes = {
  {kAnd, "[shape=invhouse,color=forestgreen,penwidth=2]"},
  {kOrr, "[shape=invtriangle,color=darkorchid,penwidth=2]"},
  {kXor, "[shape=invtriangle,peripheries=2,color=red,penwidth=1]"},
  {kNnd, "[shape=invhouse,peripheries=2,color=lawngreen,penwidth=1]"},
  {kNot, "[shape=invtriangle,color=gold,penwidth=2]"},
  {kOnn, "[shape=circle,color=black,penwidth=2]"},
  {kOff, "[shape=circle,color=black,penwidth=2]"},
  {kBuf, "[shape=circle,color=black,penwidth=2]"}
};

Gate::Gate(int type, string name, int layer)
    : type_(type), name_(name), layer_(layer), computed_(false) {

}

Gate* Gate::Copy(map<string, Gate*>& table) {
  Gate* g = new Gate(type_, name_, layer_);
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

void Gate::ForgetGate(Gate* g) {
  for (int i = 0; i != inputs_.size(); i++) {
    if (inputs_[i] == g) {
      inputs_.erase(inputs_.begin() + i--);
    }
  }
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
  if (computed_) {
    return stored_answer_;
  }
  computed_ = true;

  if (type_ == kOnn) {
    stored_answer_ = kLineOn;
    return stored_answer_;
  }
  if (type_ == kOff) {
    stored_answer_ = kLineOff;
    return stored_answer_;
  }
  if (type_ == kNot) {
    if (inputs_.size() != 1) {
      stored_answer_ = kLineUnknown;
      return stored_answer_;
    } else {
      stored_answer_ = -inputs_[0]->Compute();
      return stored_answer_;
    }
  }
  if (type_ == kAnd) {
    if (inputs_.empty()) {
      stored_answer_ = kLineUnknown;
      return stored_answer_;
    } else {
      for (Gate* in : inputs_) {
        int res = in->Compute();
        if (res == kLineOff) {
          stored_answer_ = kLineOff;
          return stored_answer_;
        } else if (res == kLineUnknown) {
          stored_answer_ = kLineUnknown;
          return stored_answer_;
        }
      }
      stored_answer_ = kLineOn;
      return stored_answer_;
    }
  }
  if (type_ == kNnd) {
    if (inputs_.empty()) {
      stored_answer_ = kLineUnknown;
      return stored_answer_;
    } else {
      for (Gate* in : inputs_) {
        int res = in->Compute();
        if (res == kLineOff) {
          stored_answer_ = kLineOn;
          return stored_answer_;
        } else if (res == kLineUnknown) {
          stored_answer_ = kLineUnknown;
          return stored_answer_;
        }
      }
      stored_answer_ = kLineOff;
    }
  }
  if (type_ == kOrr) {
    if (inputs_.empty()) {
      stored_answer_ = kLineUnknown;
    } else {
      for (Gate* in : inputs_) {
        int res = in->Compute();
        if (res == kLineOn) {
          stored_answer_ = kLineOn;
          return stored_answer_;
        } else if (res == kLineUnknown) {
          stored_answer_ = kLineUnknown;
          return stored_answer_;
        }
      }
      stored_answer_ = kLineOff;
    }
  }
  if (type_ == kXor) {
    if (inputs_.empty()) {
      stored_answer_ = kLineUnknown;
      return stored_answer_;
    } else {
      bool one_on = false;
      for (Gate* in : inputs_) {
        int res = in->Compute();
        if (res == kLineOn) {
          if (one_on) {
            stored_answer_ = kLineOff;
            return stored_answer_;
          } else {
            one_on = true;
          }
        } else if (res == kLineUnknown) {
          stored_answer_ = kLineUnknown;
          return stored_answer_;
        }
      }
      stored_answer_ = one_on ? kLineOn : kLineOff;
      return stored_answer_;
    }
  }
  if (type_ == kBuf) {
    if (inputs_.size() == 1) {
      stored_answer_ = inputs_[0]->Compute();
      return stored_answer_;
    } else {
      stored_answer_ = kLineUnknown;
      return stored_answer_;
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

bool Gate::CanTakeInput() {
  switch (type_) {
    case kNot:
      return inputs_.empty();
    case kAnd:
      return inputs_.size() < 2;
    case kXor:
      return inputs_.size() < 2;
    case kOrr:
      return inputs_.size() < 2;
    case kNnd:
      return inputs_.size() < 2;
  }
}

Gate::~Gate() {

}
