#ifndef LCP_H
#define LCP_H

#include <string>
#include <vector>
using std::string;
using std::vector;

void Test();

vector<int> CalculateLCP(string&);

int Lcp(int a, int b, string& input);

void Print(vector<int>& v);

#endif
