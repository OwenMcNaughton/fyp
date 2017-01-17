#ifndef UTIL_HH
#define UTIL_HH

#include <map>
#include <string>
#include <vector>

using namespace std;

vector<string> Split(const string& s, string delimiter);

string ReadFile(const string& filename);
void WriteFile(const string& filename, const string& contents);

string Strip(const string& src, char strip);

map<vector<int>, vector<pair<string, int>>> FormatTruthTable(
  vector<vector<string>>& truth_table, int input_count);

#endif
