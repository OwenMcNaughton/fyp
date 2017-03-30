#include "gate.hh"

#include "util.hh"
#include <iostream>

const int Gate::kNot = 0, Gate::kAnd = 1,
  Gate::kOrr = 2, Gate::kXor = 3, Gate::kNnd = 4,
  Gate::kOnn = 5, Gate::kOff = 6, Gate::kBuf = 7,
  Gate::kFullSum = 8, Gate::kFullCarry = 9,
  Gate::kHalfSum = 10, Gate::kHalfCarry = 11;

vector<int> Gate::kGates = {
  kNot, kAnd, kOrr, kXor, kNnd, kOnn, kOff, kBuf,
  kFullSum, kFullCarry, kHalfSum, kHalfCarry
};

vector<vector<vector<int>>> kFullSumTruth = {
  {  // a=0
    {  // b=0
      {Gate::kLineOff, Gate::kLineOn}
    },
    {  // b=1
      {Gate::kLineOn, Gate::kLineOff}
    }
  },
  {  // a=1
    {  // b=0
      {Gate::kLineOn, Gate::kLineOff}
    },
    {  // b=1
      {Gate::kLineOff, Gate::kLineOn}
    }
  }
};

vector<vector<vector<int>>> kFullCarryTruth = {
  {  // a=0
    {  // b=0
      {Gate::kLineOff, Gate::kLineOff}
    },
    {  // b=1
      {Gate::kLineOff, Gate::kLineOn}
    }
  },
  {  // a=1
    {  // b=0
      {Gate::kLineOff, Gate::kLineOn}
    },
    {  // b=1
      {Gate::kLineOn, Gate::kLineOn}
    }
  }
};

vector<vector<int>> kHalfSumTruth = {
  {  // a=0
    Gate::kLineOff, Gate::kLineOn
  },
  {  // a=1
    Gate::kLineOn, Gate::kLineOff
  }
};

vector<vector<int>> kHalfCarryTruth = {
  {  // a=0
    Gate::kLineOff, Gate::kLineOff
  },
  {  // a=1
    Gate::kLineOff, Gate::kLineOn
  }
};

int Gate::kLineOn = 1, Gate::kLineOff = -1, Gate::kLineUnknown = 0;

map<int, string> Gate::kDotGraphNodes = {
  {kAnd, "[shape=invhouse,color=forestgreen,penwidth=2,label=\"AND\"]"},
  {kOrr, "[shape=invtriangle,color=darkorchid,penwidth=2,label=\"ORR\"]"},
  {kXor, "[shape=invtriangle,peripheries=2,color=red,penwidth=1,label=\"XOR\"]"},
  {kNnd, "[shape=invhouse,peripheries=2,color=lawngreen,penwidth=1]"},
  {kNot, "[shape=invtriangle,color=gold,penwidth=2]"},
  {kOnn, "[shape=circle,color=black,penwidth=2]"},
  {kOff, "[shape=circle,color=black,penwidth=2]"},
  {kBuf, "[shape=circle,color=black,penwidth=2]"},
  {kFullSum, "[shape=square,color=black,penwidth=2,label=\"FAs\"]"},
  {kFullCarry, "[shape=square,color=black,penwidth=2,label=\"FAc\"]"},
  {kHalfSum, "[shape=square,color=grey,penwidth=2,label=\"HAs\"]"},
  {kHalfCarry, "[shape=square,color=grey,penwidth=2,label=\"HAc\"]"}
};

string Gate::kDotGraphOrphanNode = "[shape=circle,color=black,style=dashed,label=\"\"]";

Gate::Gate(int type, string name, int layer, bool mutateable)
    : type_(type), name_(name), layer_(layer), mutateable_(mutateable),
      computed_(false), orphan_(false), childfree_(false) {

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
  if (mutateable_) {
    type_ = Util::kLegalGateTypes[rand() % Util::kLegalGateTypes.size()];
  }
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
      return stored_answer_;
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
      return stored_answer_;
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
  if (type_ == kFullSum) {
    if (inputs_.size() == 3) {
      int a = inputs_[0]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      int b = inputs_[1]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      int cin = inputs_[2]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      if (a == -1 || b == -1 || cin == -1) {
        return kLineUnknown;
      }
      return kFullSumTruth[a][b][cin];
    } else {
      return kLineUnknown;
    }
  }
  if (type_ == kFullCarry) {
    if (inputs_.size() == 3) {
      int a = inputs_[0]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      int b = inputs_[1]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      int cin = inputs_[2]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      if (a == -1 || b == -1 || cin == -1) {
        return kLineUnknown;
      }
      return kFullCarryTruth[a][b][cin];
    } else {
      return kLineUnknown;
    }
  }
  if (type_ == kHalfSum) {
    if (inputs_.size() == 2) {
      int a = inputs_[0]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      int b = inputs_[1]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      if (a == -1 || b == -1) {
        return kLineUnknown;
      }
      return kHalfSumTruth[a][b];
    } else {
      return kLineUnknown;
    }
  }
  if (type_ == kHalfCarry) {
    if (inputs_.size() == 2) {
      int a = inputs_[0]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      int b = inputs_[1]->Compute() == kLineOff ? 0 : kLineOn ? 1 : -1;
      if (a == -1 || b == -1 ) {
        return kLineUnknown;
      }
      return kHalfCarryTruth[a][b];
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
    case kFullSum:
      return inputs_.size() < 3;
    case kFullCarry:
      return inputs_.size() < 3;
    case kHalfSum:
      return inputs_.size() < 2;
    case kHalfCarry:
      return inputs_.size() < 2;
  }
}

int Gate::ExpectedInputCount() {
  switch (type_) {
    case kNot:
      return 1;
    case kAnd:
    case kXor:
    case kOrr:
    case kNnd:
    case kHalfSum:
    case kHalfCarry:
      return 2;
    case kFullSum:
    case kFullCarry:
      return 3;
  }
}

void Gate::IsConnectedToInput() {
  for (Gate* g : inputs_) {
    if (g->orphan_) {
      continue;
    }
    if (g->type_ == kOnn || g->type_ == kOff) {
      orphan_ = false;
      return;
    }
    g->IsConnectedToInput();
    if (!g->orphan_) {
      orphan_ = false;
      return;
    }
  }
  orphan_ = true;
}

void Gate::IsConnectedToOutput() {
  childfree_ = false;
  for (Gate* g : inputs_) {
    g->IsConnectedToOutput();
  }
}

Gate::~Gate() {

}
