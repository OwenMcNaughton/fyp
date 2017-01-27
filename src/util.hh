#ifndef UTIL_HH
#define UTIL_HH

#include "circuit.hh"
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

class Circuit;

const int kGens = 100;
const int kChildren = 10000;
const int kMutations = 10;
const int kPrintRate = 10;

vector<string> Split(const string& s, string delimiter);

string ReadFile(const string& filename);
void WriteFile(const string& filename, const string& contents);

string Strip(const string& src, char strip);

map<vector<int>, map<string, int>> FormatTruthTable(
  vector<vector<string>>& truth_table, int input_count);

void SaveDotGraph(Circuit* circ, string folder, int id);

#endif
