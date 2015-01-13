#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
using namespace std;

#include "lcp.h"

/*
 * Enumerates suffix types, L, S, and S*
 */
enum SuffixType {
	L = 'L',
	S = 'S',
	S_star = 'A'
};


/*
 * Element of a bucket, contains suffix index, its type and lcp.
 */
struct BucketElement {
	int suffixIndex;
	SuffixType type;
	int lcp;
	
	BucketElement() {
		suffixIndex = -1;
		type = L;
		lcp = -1;
	}
	
	BucketElement(int _suffixIndex, SuffixType _type) {
		suffixIndex = _suffixIndex;
		type = _type;
		lcp = -1;
	}
	
	BucketElement(int _suffixIndex, SuffixType _type, int _lcp) {
		suffixIndex = _suffixIndex;
		type = _type;
		lcp = _lcp;
	}
};

/*
 * Container for bucket elements.
 */
struct Bucket {
	char letter;
	vector<BucketElement> elements;
	int head, tail;
	
	vector<int> suffixIndexToElementIndex;
	
	Bucket(char _letter, int size, int inputSize) {
		letter = _letter;
		elements.resize(size);
		suffixIndexToElementIndex.resize(inputSize);
		head = 0;
		tail = size-1;
	}
	
	void putBack(BucketElement element) {
		if (tail < 0) {
			throw string("putBack: bucket is full");
		}
		elements.at(tail) = element;
		suffixIndexToElementIndex.at(element.suffixIndex) = tail;
		tail--;
	}
	
	void putFront(BucketElement element) {
		if (head >= (int)elements.size()) {
			throw string("putFront: bucket is full");
		}
		elements.at(head) = element;
		suffixIndexToElementIndex.at(element.suffixIndex) = head;
		head++;
	}
	
	void resetTailPointer() {
		tail = elements.size() - 1;
	}
	
	int find(int suffixIndex) {
		return suffixIndexToElementIndex.at(suffixIndex);
	}
	
	void print() {
		printf("bucket: %c\n", letter);
		for (vector<BucketElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
			printf("%d %c\n", it->suffixIndex, it->type);
		}
	}
	
	void printSeq() {
		for (vector<BucketElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
			if (it->lcp < 0) {
				printf("x ");
			} else {
				printf("%d ", it->lcp);
			}
		}
	}
};

/*
 * Characteristic name of the suffix.
 * Stores lcp as well.
 */
struct Name {
	int index;
	string name;
	
	int lcp;
	
	Name(int _index, string _name) {
		index = _index;
		name = _name;
		lcp = -1;
	}
	
	string getSuffix(string& input) {
		return input.substr(index);
	}
	
	int suffixLCP(Name& n, string& input) {
		int len = min(name.length(), n.name.length());
		for (int i = 0; i < len; i++) {
			if (name.at(i) != n.name.at(i)) {
				return i;
			}
		}
		return len;
	}
};

/*
 * Calculates lcp of two strings, a and b.
 */
int lcp(const string& a, const string& b) {
	int len = min(a.length(), b.length());
	for (int i = 0; i < len; i++) {
		if (a.at(i) != b.at(i)) {
			return i;
		}
	}
	return len;
}

/*
 * Comparator object for the Name class.
 */
struct NameComparator {
	string& input;
	
	NameComparator(string& _input) : input(_input) {
	}
	
	bool operator()(const Name& a, const Name& b) {
		int i = a.index;
		int j = b.index;
		int n = input.length();
		while (i < n && j < n) {
			if (input.at(i) < input.at(j)) {
				return true;
			} else if (input.at(i) > input.at(j)) {
				return false;
			}
			i++;
			j++;
		}
		return true;
	}
};

/*
 * Calculates array of suffix types from the given input string.
 */
vector<SuffixType> createSuffixTypeArray(string input) {
	const int n = input.length();
	vector<SuffixType> types(n);
	types.at(n-1) = S;
	
	for (int i = n-2; i >= 0; i--) {
		if (input.at(i) < input.at(i+1)) {
			types.at(i) = S;
		} else if (input.at(i) > input.at(i+1)) {
			types.at(i) = L;
			if (types.at(i+1) == S) {
				types.at(i+1) = S_star;
			}
		} else {
			types.at(i) = types.at(i+1);
		}
	}
	
	return types;
}

/*
 * returns distinct letters from the given string, sorted in
 * alphabetical order.
 */
string distinctLetters(string& str) {
	vector<char> chars(str.begin(), str.end());
	sort(chars.begin(), chars.end());
	unsigned int i = 0;
	while (i < chars.size() - 1) {
		if (chars.at(i) == chars.at(i+1)) {
			chars.erase(chars.begin() + i);
		} else {
			i++;
		}
	}
	string ret(chars.begin(), chars.end());
	return ret;
}

/*
 * Counts the occurrences of 'letter' in string 'in'.
 */
int count(char letter, string& in) {
	int ret = 0;
	for (string::iterator it = in.begin(); it != in.end(); ++it) {
		if (*it == letter) {
			++ret;
		}
	}
	return ret;
}

/*
 * Creates the initial empty buckets.
 */
vector<Bucket> createBuckets(string& input) {
	vector<Bucket> buckets;
	string distinct = distinctLetters(input);
	
	for (string::iterator it = distinct.begin(); it != distinct.end(); ++it) {
		char c = *it;
		Bucket b(c, count(c, input), input.length());
		buckets.push_back(b);
	}
	
	return buckets;
}

/*
 * Gets the bucket with the letter 'letter'.
 */
Bucket& getBucket(vector<Bucket>& buckets, char letter) {
	for (vector<Bucket>::iterator it = buckets.begin(); it != buckets.end(); ++it) {
		if (it->letter == letter) {
			return *it;
		}
	}
	string msg = "Could not find bucket with letter ";
	msg += letter;
	throw msg;
}


/* 2.1) */
void addSStarSuffix(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
	for (unsigned int i = 0; i < input.length(); i++) {
		if (types.at(i) == S_star) {
			Bucket& bucket = getBucket(buckets, input.at(i));
			BucketElement element(i, types.at(i));
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
				if (types.at(index) == L) {
					Bucket& into = getBucket(buckets, input.at(index));
					BucketElement newElement(index, types.at(index));
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
				if (types.at(index) == S || types.at(index) == S_star) {
					Bucket& into = getBucket(buckets, input.at(index));
					BucketElement newElement(index, types.at(index));
					into.putBack(newElement);
				}
			}
		}
	}
}

string getName(int index, string& in, vector<SuffixType> types) {
	string ret;
	ret += in.at(index);
	for (unsigned int i = index; i < in.length()-1; i++) {
		ret += in.at(i+1);
		if (types.at(i+1) == S_star) {
			break;
		}
	}
	return ret;
}

/* 3. */
vector<Name> getNames(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
	vector<Name> names;
	
	for (unsigned int i = 0; i < buckets.size(); i++) {
		Bucket& bucket = buckets.at(i);
		vector<BucketElement>& elements = bucket.elements;
		
		for (unsigned int j = 0; j < elements.size(); j++) {
			if (elements.at(j).type == S_star) {
				string name = getName(elements.at(j).suffixIndex, input, types);
				Name chName(elements.at(j).suffixIndex, name);
				names.push_back(chName);
			}
		}
	}
	
	return names;
}

typedef vector<Name> Names;

/* 3.1) */
vector<Names> getCategories(vector<Name>& names) {
	vector<Names> categories;
	vector<Name> first;
	first.push_back(names.at(0));
	categories.push_back(first);
	
	int category = 0;
	for (int i = 1; i < (int)names.size(); i++) {
		if (names.at(i).name != names.at(i-1).name) {
			vector<Name> newCategory;
			categories.push_back(newCategory);
			category++;
		}
		categories.at(category).push_back(names.at(i));
	}
	
	return categories;
}

vector<Name> flatten(vector<Names>& categories, string& input) {
	NameComparator nameComparator(input);
	vector<Name> names;
	
	for (vector<Names>::iterator it = categories.begin(); it != categories.end(); ++it) {
		if (it->size() > 1) {
			sort(it->begin(), it->end(), nameComparator);
		}
		for (vector<Name>::iterator name = it->begin(); name != it->end(); ++name) {
			names.push_back(*name);
		}
	}
	
	return names;
}

/* 3.2) */
void lcpInitial(vector<Name>& names, string& input) {
	names.at(0).lcp = 0;
	for (int i = 1; i < (int)names.size(); i++) {
		names.at(i).lcp = names.at(i).suffixLCP(names.at(i-1), input);
	}
}

/* 4.1) Inserts S* suffixes into buckets. */
void lastStepSStar(vector<Bucket>& buckets, vector<Name>& names, vector<SuffixType>& types, string& input) {
	for (int j = (int)names.size()-1; j >= 0; j--) {
		Name& name = names.at(j);
		int i = name.index;
		if (types.at(i) == S_star) {
			Bucket& bucket = getBucket(buckets, input.at(i));
			BucketElement element(i, types.at(i), name.lcp);
			bucket.putBack(element);
		}
	}
}

void insertNotFirstL(int index, vector<Bucket>& buckets, Bucket& bucket, vector<SuffixType>& types, string& input) {
	BucketElement elem(index, types.at(index), 0);
				
	BucketElement& prevL = bucket.elements.at(bucket.head-1);
	int suffixA = elem.suffixIndex + 1;
	int suffixB = prevL.suffixIndex + 1;
	
	if (input.at(suffixA) == input.at(suffixB)) {
		Bucket& bucketForSuffix = getBucket(buckets, input[suffixA]);
		int indexA = bucketForSuffix.find(suffixA);
		int indexB = bucketForSuffix.find(suffixB);
		int begin = 1 + min(indexA, indexB);
		int end = max(indexA, indexB);
		int minLcp = 1000000000;
		for (; begin <= end; begin++) {
			BucketElement& element = bucketForSuffix.elements.at(begin);
			if (element.lcp < minLcp) {
				minLcp = element.lcp;
			}
		}
		elem.lcp = minLcp + 1;
	} else {
		elem.lcp = 1;
	}
	
	bucket.putFront(elem);
}

void insertNotFirstS(int index, vector<Bucket>& buckets, Bucket& bucket, vector<SuffixType>& types, string& input) {
	BucketElement elem(index, types.at(index), 0);
				
	BucketElement& prev = bucket.elements.at(bucket.tail + 1);
	int suffixA = elem.suffixIndex + 1;
	int suffixB = prev.suffixIndex + 1;
	
	if (input.at(suffixA) == input.at(suffixB)) {
		Bucket& bucketForSuffix = getBucket(buckets, input[suffixA]);
		int indexA = bucketForSuffix.find(suffixA);
		int indexB = bucketForSuffix.find(suffixB);
		int begin = 1 + min(indexA, indexB);
		int end = max(indexA, indexB);
		int minLcp = 1000000000;
		for (; begin <= end; begin++) {
			BucketElement& element = bucketForSuffix.elements.at(begin);
			if (element.lcp < minLcp) {
				minLcp = element.lcp;
			}
		}
		elem.lcp = minLcp + 1;
	} else {
		elem.lcp = 1;
	}
	
	bucket.putBack(elem);
}

void updateLSBorder(Bucket& bucket, vector<SuffixType>& types, string& input) {
	if (bucket.head < (int)bucket.elements.size()) {
		BucketElement& elemA = bucket.elements.at(bucket.head - 1);
		BucketElement& elemB = bucket.elements.at(bucket.head);
		if (elemB.suffixIndex != -1 && types.at(elemB.suffixIndex) == S_star) {
			int lcpValue = lcp(elemA.suffixIndex, elemB.suffixIndex, input);
			elemB.lcp = lcpValue;
		}
	}
}

void updateBorder(int position, Bucket& bucket, vector<SuffixType>& types, string& input) {
	BucketElement& elemA = bucket.elements.at(position);
	if (position == 0) {
		elemA.lcp = 0;
		return;
	}
	
	BucketElement& elemB = bucket.elements.at(position - 1);
	if (elemB.suffixIndex != -1) {
		int lcpValue = lcp(elemA.suffixIndex, elemB.suffixIndex, input);
		elemA.lcp = lcpValue;
	}
}

/* 4.2) Inserts L suffixes into buckets and updates lcps. */
void lastStepL(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
	for (int i = 0; i < (int)buckets.size(); i++) {
		for (int j = 0; j < (int)buckets[i].elements.size(); j++) {
			int index = buckets[i].elements[j].suffixIndex - 1;
			if (index >= 0 && types.at(index) == L) {
				Bucket& bucket = getBucket(buckets, input.at(index));
				if (bucket.head == 0) {
					// there's no L's in this bucket yet
					BucketElement elem(index, L, 0);
					bucket.putFront(elem);
				} else {
					// there are L's in this bucket
					insertNotFirstL(index, buckets, bucket, types, input);
				}
				
				updateLSBorder(bucket, types, input);
			}
		}
	}
}

/* 4.3) Inserts S suffixes into buckets and updates lcps. */
void lastStepS(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
	for (int i = 0; i < (int)buckets.size(); i++) {
		buckets[i].resetTailPointer();
	}
	
	for (int i = (int)buckets.size() - 1; i >= 0; i--) {
		for (int j = (int)buckets.at(i).elements.size() - 1; j >= 0; j--) {
			BucketElement element = buckets.at(i).elements.at(j);
			int index = element.suffixIndex - 1;
			if (index >= 0 && types.at(index) != L) {
				Bucket& bucket = getBucket(buckets, input.at(index));
				if (bucket.tail == (int)bucket.elements.size()-1) {
					// there's no S's in this bucket yet
					BucketElement elem(index, types.at(index), 0);
					bucket.putBack(elem);
					updateBorder(bucket.tail+1, bucket, types, input);
				} else {
					// there are S's in this bucket
					insertNotFirstS(index, buckets, bucket, types, input);
					updateBorder(bucket.tail+1, bucket, types, input);
					updateBorder(bucket.tail+2, bucket, types, input);
				}
			}
		}
	}
}

/* 4. */
vector<Bucket> calculateLCP(vector<Name>& names, vector<SuffixType>& types, string& input) {
	vector<Bucket> buckets = createBuckets(input);
	
	lastStepSStar(buckets, names, types, input);
	lastStepL(buckets, types, input);
	lastStepS(buckets, types, input);
	
	return buckets;
}

vector<int> bruteForce(string& input);
void test1(string&);
vector<int> test2(string&);

string randomString(int minSize, int maxSize) {
	int size = minSize + (rand() % (maxSize - minSize + 1));
	
	string ret = "";
	for (int i = 0; i < size; i++) {
		char c = 'a' + (rand() % 4);
		ret += c;
	}
	return ret;
}

bool areSame(vector<int>& a, vector<int>& b) {
	if (a.size() != b.size()) {
		return false;
	} else {
		for (int i = 0; i < (int)a.size(); i++) {
			if (a[i] != b[i]) {
				return false;
			}
		}
	}
	return true;
}

void batchTest() {
	const int t = 1000;
	const int size = 50;
	int correct = 0;
	srand(time(NULL));
	
	for (int i = 0; i < t; i++) {
		string input = randomString(size, size) + "$";
		vector<int> actual = test2(input);
		vector<int> expected = bruteForce(input);
		cout << input << endl;
		if (areSame(actual, expected)) {
			correct++;
		}
	}
	
	printf("%d/%d\n", correct, t);
}

void test() {
	batchTest();
}

vector<int> test2(string& input) {
	vector<Bucket> buckets = createBuckets(input);
	vector<SuffixType> types = createSuffixTypeArray(input);
	
	addSStarSuffix(buckets, types, input);
	addLSuffixes(buckets, types, input);
	addSSuffixes(buckets, types, input);
	vector<Name> unsortedNames = getNames(buckets, types, input);
	vector<Names> categories = getCategories(unsortedNames);
	vector<Name> names = flatten(categories, input);
	lcpInitial(names, input);
	buckets = calculateLCP(names, types, input);
	
	vector<int> result;
	for (vector<Bucket>::iterator it = buckets.begin(); it != buckets.end(); ++it) {
		for (int i = 0; i < (int)it->elements.size(); i++) {
			result.push_back(it->elements[i].lcp);
		}
	}
	return result;
}

void test1() {
	string input = "otorinolaringologija$";
	//string input = "aralralraralkapalkar$";
	
	try {
		printf("actual:   "); test2(input);
		printf("expected: "); bruteForce(input);
	} catch (string e) {
		printf("Exception: %s\n", e.c_str());
	}
}

struct SuffixComparator {
	string& input;
	
	SuffixComparator(string& in) : input(in) {}
	
	bool operator()(const int& a, const int& b) {
		int i = a;
		int j = b;
		int k = 0;
		while (i+k < (int)input.length() && j+k < (int)input.length()) {
			if (input.at(i+k) < input.at(j+k)) {
				return true;
			} else if (input.at(i+k) > input.at(j+k)) {
				return false;
			}
			k++;
		}
		return true;
	}
};

int lcp(int a, int b, string& input) {
	int i = a;
	int j = b;
	int k = 0;
	int lcp = 0;
	while (i+k < (int)input.length() && j+k < (int)input.length()) {
		if (input.at(i+k) == input.at(j+k)) {
			lcp++;
		} else {
			break;
		}
		k++;
	}
	return lcp;
}

/*
 * Brute-force solution, for testing purposes.
 */
vector<int> bruteForce(string& input) {
	SuffixComparator cmp(input);
	
	vector<int> v;
	for (int i = 0; i < (int)input.length(); i++) {
		v.push_back(i);
	}
	
	sort(v.begin(), v.end(), cmp);
	
	vector<int> result;
	result.push_back(0);
	for (int i = 1; i < (int)v.size(); i++) {
		int l = lcp(v.at(i-1), v.at(i), input);
		result.push_back(l);
	}
	return result;
}
