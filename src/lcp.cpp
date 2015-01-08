#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
using namespace std;

#include "lcp.hpp"

enum SuffixType {
	L = 'L',
	S = 'S',
	S_star = 'A'
};

struct BucketElement {
	int suffixIndex;
	SuffixType type;
	
	BucketElement() {
		suffixIndex = -1;
		type = L;
	}
	
	BucketElement(int _suffixIndex, SuffixType _type) {
		suffixIndex = _suffixIndex;
		type = _type;
	}
};

struct Bucket {
	char letter;
	vector<BucketElement> elements;
	int head, tail;
	
	Bucket(char _letter, int size) {
		letter = _letter;
		elements.resize(size);
		head = 0;
		tail = size-1;
	}
};

vector<SuffixType> createSuffixTypeArray(string input) {
	const int n = input.length();
	vector<SuffixType> types(n);
	types[n-1] = S;
	
	for (int i = n-2; i >= 0; i--) {
		if (input[i] < input[i+1]) {
			types[i] = S;
		} else if (input[i] > input[i+1]) {
			types[i] = L;
			if (types[i+1] == S) {
				types[i+1] = S_star;
			}
		} else {
			types[i] = types[i+1];
		}
	}
	
	return types;
}

string distinctLetters(string& str) {
	vector<char> chars(str.begin(), str.end());
	sort(chars.begin(), chars.end());
	unsigned int i = 0;
	while (i < chars.size() - 1) {
		if (chars[i] == chars[i+1]) {
			chars.erase(chars.begin() + i);
		} else {
			i++;
		}
	}
	string ret(chars.begin(), chars.end());
	return ret;
}

int count(char letter, string& in) {
	int ret = 0;
	for (string::iterator it = in.begin(); it != in.end(); ++it) {
		if (*it == letter) {
			++ret;
		}
	}
	return ret;
}

vector<Bucket> createBuckets(string& input) {
	vector<Bucket> buckets;
	string distinct = distinctLetters(input);
	
	for (string::iterator it = distinct.begin(); it != distinct.end(); ++it) {
		char c = *it;
		Bucket b(c, count(c, input));
		buckets.push_back(b);
	}
	
	return buckets;
}

void test1(string&);
void test2(string&);

void test() {
	string input = "otorinolaringologija$";
	test2(input);
}

void test2(string& input) {
	string d = distinctLetters(input);
	vector<Bucket> buckets = createBuckets(input);
	
	for (vector<Bucket>::iterator it = buckets.begin(); it != buckets.end(); ++it) {
		Bucket b = *it;
		printf("%c %d\n", b.letter, b.tail - b.head + 1);
	}
}

void test1(string& input) {
	vector<SuffixType> types = createSuffixTypeArray(input);
	
	cout << input << endl;
	for (vector<SuffixType>::iterator it = types.begin(); it != types.end(); ++it) {
		printf("%c", *it);
	}
	printf("\n");
}
