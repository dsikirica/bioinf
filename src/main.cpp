#include <cstdio>
#include <iostream>
#include <fstream>
#include <ostream>
#include <ctime>
using namespace std;

#include "lcp.h"

void Run(const char *directory) {
	int i = 1;
	char filename[32];
	
	while (true) {
		sprintf(filename, "%s/input%d.txt", directory, i);
		printf("%s\n", filename);
		
		ifstream file (filename, ifstream::in);
		if (!file.good()) {
			break;
		}
		
		string line;
		file >> line;
		file.close();
		
		printf("File: %s\nInput length: %d\nCalculating lcp...\n", filename, (int)line.length());
		long timeNow = time(NULL);
		vector<int> output = CalculateLCP(line);
		printf("Time elapsed: %ld [sec]\n\n", time(NULL) - timeNow);
		
		string out;
		char num[16];
		for (int j = 0; j < (int)output.size(); j++) {
			sprintf(num, "%d", output[j]);
			out += num;
			out += " ";
		}
		
		sprintf(filename, "%s/output%d.txt", directory, i);
		ofstream outFile (filename, ofstream::out);
		outFile << out;
		outFile.close();
		i++;
	}
}

int main(int argc, char **argv) {
	//Test();
	Run("tests");
	return 0;
}
