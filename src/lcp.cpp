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
	
	void putBack(BucketElement element) {
		elements[tail] = element;
		tail--;
	}
	
	void putFront(BucketElement element) {
		elements[head] = element;
		head++;
	}
	
	void resetTailPointer() {
		tail = elements.size() - 1;
	}
	
	void print() {
		printf("bucket: %c\n", letter);
		for (vector<BucketElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
			printf("%d %c\n", it->suffixIndex, it->type);
		}
	}
};

struct Name {
	int index;
	string name;
	
	Name() {
		index = -1;
	}
	
	Name(int _index, string _name) {
		index = _index;
		name = _name;
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

Bucket& getBucket(vector<Bucket>& buckets, char letter) {
	for (vector<Bucket>::iterator it = buckets.begin(); it != buckets.end(); ++it) {
		if (it->letter == letter) {
			return *it;
		}
	}
	throw "Could not find bucket with letter " + letter;
}

/* 2.1) */
void addSStarSuffix(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
	for (unsigned int i = 0; i < input.length(); i++) {
		if (types[i] == S_star) {
			Bucket& bucket = getBucket(buckets, input[i]);
			BucketElement element(i, types[i]);
			bucket.putBack(element);
		} 
	}
}

/* 2.2) */
void addLSuffixes(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
	for (unsigned int i = 0; i < buckets.size(); i++) {
		vector<BucketElement>& elements = buckets.at(i).elements;
		
		for (unsigned int j = 0; j < elements.size(); j++) {
			BucketElement& element = elements.at(j);
			if (element.suffixIndex > 0) {
				int index = element.suffixIndex - 1;	
				if (types[index] == L) {
					Bucket& into = getBucket(buckets, input[index]);
					BucketElement newElement(index, types[index]);
					into.putFront(newElement);
				}
			}
		}
	}
}

/* 2.3) */
void addSSuffixes(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
	for (vector<Bucket>::iterator it = buckets.begin(); it != buckets.end(); ++it) {
		it->resetTailPointer();
	}
	
	for (int i = buckets.size()-1; i >= 0; i--) {
		vector<BucketElement>& elements = buckets.at(i).elements;
		
		for (int j = elements.size()-1; j >= 0; j--) {
			BucketElement& element = elements.at(j);
			if (element.suffixIndex > 0) {
				int index = element.suffixIndex - 1;
				if (types[index] == S || types[index] == S_star) {
					Bucket& into = getBucket(buckets, input[index]);
					BucketElement newElement(index, types[index]);
					into.putBack(newElement);
				}
			}
		}
	}
}

/* 3. */
string getName(int index, string& in, vector<SuffixType> types) {
	string ret;
	ret += in[index];
	for (unsigned int i = index; i < in.length()-1; i++) {
		ret += in[i+1];
		if (types[i+1] == S_star) {
			break;
		}
	}
	return ret;
}

vector<Name> getNames(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
	vector<Name> names;
	
	for (unsigned int i = 0; i < buckets.size(); i++) {
		Bucket& bucket = buckets.at(i);
		vector<BucketElement>& elements = bucket.elements;
		
		for (unsigned int j = 0; j < elements.size(); j++) {
			if (elements[j].type == S_star) {
				string name = getName(elements[j].suffixIndex, input, types);
				Name chName(elements[j].suffixIndex, name);
				names.push_back(chName);
			}
		}
	}
	
	return names;
}

void test1(string&);
void test2(string&);

void test() {
	string input = "otorinolaringologija$";
	test2(input);
}

void test2(string& input) {
	vector<Bucket> buckets = createBuckets(input);
	vector<SuffixType> types = createSuffixTypeArray(input);
	
	for (vector<SuffixType>::iterator it = types.begin(); it != types.end(); ++it) {
		printf("%c", *it);
	}
	printf("\n");
	
	addSStarSuffix(buckets, types, input);
	addLSuffixes(buckets, types, input);
	addSSuffixes(buckets, types, input);
	vector<Name> names = getNames(buckets, types, input);
	
	//cout << input << endl;
	//for (vector<Bucket>::iterator it = buckets.begin(); it != buckets.end(); ++it) {
	//	it->print();
	//}
	
	for (vector<Name>::iterator it = names.begin(); it != names.end(); ++it) {
		printf("%d %s\n", it->index, it->name.c_str());
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
