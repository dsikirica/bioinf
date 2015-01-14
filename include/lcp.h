#ifndef LCP_H
#define LCP_H

#include <string>
#include <vector>
using std::string;
using std::vector;

/*
 * Runs tests with random strings.
 */
void BatchTest();

/*
 * Calculates the LCP array for the given input string.
 */
vector<int> CalculateLCP(string&);

/*
 * Calculates the LCP value between suffixes with indicies a and b.
 */ 
int Lcp(int a, int b, string& input);

/*
 * Prints out the given vector.
 */
void Print(vector<int>& v);

#endif
