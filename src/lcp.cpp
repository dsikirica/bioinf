#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cstdarg>
using namespace std;

#include "lcp.h"

#define foreach(Type, data) for (vector<Type>::iterator it = data.begin(); it != data.end(); ++it)

//#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
void log(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stdout, format, args);
  va_end(args);
}
#else
void log(const char *format, ...) {
  
}
#endif

/*
 * Enumerates suffix types, L, S , and S*
 */
enum SuffixType {
  kL = 'L',
  kS = 'S',
  kS_star = 'A'
};


/*
 * Element of a bucket, contains suffix index, its type and lcp.
 */
struct BucketElement {
  int suffix_index;
  SuffixType type;
  int lcp;
  
  BucketElement() {
    suffix_index = -1;
    type = kL;
    lcp = -1;
  }
  
  BucketElement(int suffix_index_, SuffixType type_) {
    suffix_index = suffix_index_;
    type = type_;
    lcp = -1;
  }
  
  BucketElement(int suffix_index_, SuffixType type_, int lcp_) {
    suffix_index = suffix_index_;
    type = type_;
    lcp = lcp_;
  }
};

/*
 * Container for bucket elements.
 */
struct Bucket {
  char letter;
  vector<BucketElement> elements;
  int head, tail;
  
  vector<int> suffix_index_to_element_index;
  
  Bucket(char letter_, int size, int input_size) {
    letter = letter_;
    elements.resize(size);
    suffix_index_to_element_index.resize(input_size, -1);
    head = 0;
    tail = size-1;
  }
  
  void PutBack(BucketElement element) {
    if (tail < 0) {
      throw string("PutBack: bucket is full");
    }
    elements.at(tail) = element;
    suffix_index_to_element_index.at(element.suffix_index) = tail;
    tail--;
  }
  
  void PutFront(BucketElement element) {
    if (head >= (int)elements.size()) {
      throw string("PutFront: bucket is full");
    }
    elements.at(head) = element;
    suffix_index_to_element_index.at(element.suffix_index) = head;
    head++;
  }
  
  void ResetTailPointer() {
    tail = elements.size() - 1;
  }
  
  int Find(int suffix_index) {
    return suffix_index_to_element_index.at(suffix_index);
  }
  
  void Print() {
    log("bucket: %c\n", letter);
    for (vector<BucketElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
      log("%d %c %d\n", it->suffix_index, it->type, it->lcp);
    }
  }
  
  void PrintSeq() {
    for (vector<BucketElement>::iterator it = elements.begin(); it != elements.end(); ++it) {
      if (it->lcp < 0) {
        log("x ");
      } else {
        log("%d ", it->lcp);
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
  
  Name(int index_, string name_) {
    index = index_;
    name = name_;
    lcp = -1;
  }
  
  string GetSuffix(string& input) {
    return input.substr(index);
  }
  
  int SuffixLCP(Name& n, string& input) {
    int len = min(name.length(), n.name.length());
    for (int i = 0; i < len; i++) {
      if (name.at(i) != n.name.at(i)) {
        return i;
      }
    }
    return len;
  }
};

void UpdateBorder(int position, Bucket& bucket, vector<SuffixType>& types, string& input);
void UpdateBorderToLeft(int position, Bucket& bucket, vector<SuffixType>& types, string& input);

/*
 * Calculates lcp of two strings, a and b.
 */
int Lcp(const string& a, const string& b) {
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
  
  NameComparator(string& input_) : input(input_) {
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
vector<SuffixType> CreateSuffixTypeArray(string input) {
  const int n = input.length();
  vector<SuffixType> types(n);
  types.at(n-1) = kS;
  
  for (int i = n-2; i >= 0; i--) {
    if (input.at(i) < input.at(i+1)) {
      types.at(i) = kS;
    } else if (input.at(i) > input.at(i+1)) {
      types.at(i) = kL;
      if (types.at(i+1) == kS) {
        types.at(i+1) = kS_star;
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
string DistinctLetters(string& str) {
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
int Count(char letter, string& in) {
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
vector<Bucket> CreateBuckets(string& input) {
  vector<Bucket> buckets;
  string distinct = DistinctLetters(input);
  
  for (string::iterator it = distinct.begin(); it != distinct.end(); ++it) {
    char c = *it;
    Bucket b(c, Count(c, input), input.length());
    buckets.push_back(b);
  }
  
  return buckets;
}

/*
 * Gets the bucket with the letter 'letter'.
 */
Bucket& GetBucket(vector<Bucket>& buckets, char letter) {
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
void AddSStarSuffix(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
  for (unsigned int i = 0; i < input.length(); i++) {
    if (types.at(i) == kS_star) {
      Bucket& bucket = GetBucket(buckets, input.at(i));
      BucketElement element(i, types.at(i));
      bucket.PutBack(element);
    } 
  }
}

/* 2.2) */
void AddLSuffixes(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
  for (unsigned int i = 0; i < buckets.size(); i++) {
    vector<BucketElement>& elements = buckets.at(i).elements;
    
    for (unsigned int j = 0; j < elements.size(); j++) {
      BucketElement& element = elements.at(j);
      if (element.suffix_index > 0) {
        int index = element.suffix_index - 1;  
        if (types.at(index) == kL) {
          Bucket& into = GetBucket(buckets, input.at(index));
          BucketElement newElement(index, types.at(index));
          into.PutFront(newElement);
        }
      }
    }
  }
}

/* 2.3) */
void AddSSuffixes(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
  for (vector<Bucket>::iterator it = buckets.begin(); it != buckets.end(); ++it) {
    it->ResetTailPointer();
  }
  
  for (int i = buckets.size()-1; i >= 0; i--) {
    vector<BucketElement>& elements = buckets.at(i).elements;
    
    for (int j = elements.size()-1; j >= 0; j--) {
      BucketElement& element = elements.at(j);
      if (element.suffix_index > 0) {
        int index = element.suffix_index - 1;
        if (types.at(index) == kS || types.at(index) == kS_star) {
          Bucket& into = GetBucket(buckets, input.at(index));
          BucketElement new_element(index, types.at(index));
          into.PutBack(new_element);
        }
      }
    }
  }
}

string GetName(int index, string& in, vector<SuffixType> types) {
  string ret;
  ret += in.at(index);
  for (unsigned int i = index; i < in.length()-1; i++) {
    ret += in.at(i+1);
    if (types.at(i+1) == kS_star) {
      break;
    }
  }
  return ret;
}

/* 3. */
vector<Name> GetNames(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
  vector<Name> names;
  
  for (unsigned int i = 0; i < buckets.size(); i++) {
    Bucket& bucket = buckets.at(i);
    vector<BucketElement>& elements = bucket.elements;
    
    for (unsigned int j = 0; j < elements.size(); j++) {
      if (elements.at(j).type == kS_star) {
        string name = GetName(elements.at(j).suffix_index, input, types);
        Name chName(elements.at(j).suffix_index, name);
        names.push_back(chName);
      }
    }
  }
  
  return names;
}

typedef vector<Name> Names;

/* 3.1) */
vector<Names> GetCategories(vector<Name>& names) {
  vector<Names> categories;
  vector<Name> first;
  first.push_back(names.at(0));
  categories.push_back(first);
  
  int category = 0;
  for (int i = 1; i < (int)names.size(); i++) {
    if (names.at(i).name != names.at(i-1).name) {
      vector<Name> new_category;
      categories.push_back(new_category);
      category++;
    }
    categories.at(category).push_back(names.at(i));
  }
  
  return categories;
}

vector<Name> Flatten(vector<Names>& categories, string& input) {
  NameComparator name_comparator(input);
  vector<Name> names;
  
  for (vector<Names>::iterator it = categories.begin(); it != categories.end(); ++it) {
    if (it->size() > 1) {
      sort(it->begin(), it->end(), name_comparator);
    }
    for (vector<Name>::iterator name = it->begin(); name != it->end(); ++name) {
      names.push_back(*name);
    }
  }
  
  return names;
}

/* 3.2) */
void LcpInitial(vector<Name>& names, string& input) {
  names.at(0).lcp = 0;
  for (int i = 1; i < (int)names.size(); i++) {
    names.at(i).lcp = names.at(i).SuffixLCP(names.at(i-1), input);
  }
}

/* 4.1) Inserts S* suffixes into buckets. */
void LastStepSStar(vector<Bucket>& buckets, vector<Name>& names, vector<SuffixType>& types, string& input) {
  for (int j = (int)names.size()-1; j >= 0; j--) {
    Name& name = names.at(j);
    int i = name.index;
    if (types.at(i) == kS_star) {
      Bucket& bucket = GetBucket(buckets, input.at(i));
      BucketElement element(i, types.at(i), name.lcp);
      bucket.PutBack(element);
      if (bucket.tail < (int)bucket.elements.size() - 2) {
        UpdateBorder(bucket.tail+2, bucket, types, input);
      }
    }
  }
}

void InsertNotFirstL(int index, vector<Bucket>& buckets, Bucket& bucket, vector<SuffixType>& types, string& input) {
  BucketElement elem(index, types.at(index), 0);
        
  BucketElement& prevL = bucket.elements.at(bucket.head - 1);
  int suffixA = elem.suffix_index + 1;
  int suffixB = prevL.suffix_index + 1;
  
  if (input.at(suffixA) == input.at(suffixB)) {
    Bucket& bucket_for_suffix = GetBucket(buckets, input[suffixA]);
    int indexA = bucket_for_suffix.Find(suffixA);
    int indexB = bucket_for_suffix.Find(suffixB);
    int begin = 1 + min(indexA, indexB);
    int end = max(indexA, indexB);
    int minLcp = 1000000000;
    for (; begin <= end; begin++) {
      BucketElement& element = bucket_for_suffix.elements.at(begin);
      if (element.lcp != -1 && element.lcp < minLcp) {
        minLcp = element.lcp;
      }
    }
    elem.lcp = minLcp + 1;
  } else {
    elem.lcp = 1;
  }
  
  bucket.PutFront(elem);
}

void InsertNotFirstS(int index, vector<Bucket>& buckets, Bucket& bucket, vector<SuffixType>& types, string& input) {
  BucketElement elem(index, types.at(index), 0);
        
  BucketElement& prev = bucket.elements.at(bucket.tail + 1);
  int suffixA = elem.suffix_index + 1;
  int suffixB = prev.suffix_index + 1;
  
  if (input.at(suffixA) == input.at(suffixB)) {
    Bucket& bucket_for_suffix = GetBucket(buckets, input[suffixA]);
    int indexA = bucket_for_suffix.Find(suffixA);
    int indexB = bucket_for_suffix.Find(suffixB);
    int begin = 1 + min(indexA, indexB);
    int end = max(indexA, indexB);
    int minLcp = 1000000000;
    for (; begin <= end; begin++) {
      BucketElement& element = bucket_for_suffix.elements.at(begin);
      if (element.lcp < minLcp) {
        minLcp = element.lcp;
      }
    }
    elem.lcp = minLcp + 1;
  } else {
    elem.lcp = 1;
  }
  
  bucket.PutBack(elem);
}

void UpdateLSBorder(Bucket& bucket, vector<SuffixType>& types, string& input) {
  if (bucket.head < (int)bucket.elements.size()) {
    BucketElement& elemA = bucket.elements.at(bucket.head - 1);
    BucketElement& elemB = bucket.elements.at(bucket.head);
    if (elemB.suffix_index != -1 && types.at(elemB.suffix_index) == kS_star) {
      int lcp_value = Lcp(elemA.suffix_index, elemB.suffix_index, input);
      elemB.lcp = lcp_value;
    }
  }
}

void UpdateBorder(int position, Bucket& bucket, vector<SuffixType>& types, string& input) {
  BucketElement& elemA = bucket.elements.at(position);
  if (position == 0) {
    elemA.lcp = 0;
    return;
  }
  
  BucketElement& elemB = bucket.elements.at(position - 1);
  if (elemB.suffix_index != -1) {
    int lcp_value = Lcp(elemA.suffix_index, elemB.suffix_index, input);
    elemA.lcp = lcp_value;
  }
}

void UpdateBorderToLeft(int position, Bucket& bucket, vector<SuffixType>& types, string& input) {
  BucketElement& elemA = bucket.elements.at(position);
  if (position == 0) {
    elemA.lcp = 0;
    return;
  }
  
  BucketElement& elemB = bucket.elements.at(position - 1);
  if (elemB.suffix_index != -1) {
    int lcp_value = Lcp(elemA.suffix_index, elemB.suffix_index, input);
    elemA.lcp = lcp_value;
  } else {
    int index = bucket.head - 1;
    if (index >= 0) {
      BucketElement& elemC = bucket.elements.at(index);
      int lcp_value = Lcp(elemA.suffix_index, elemC.suffix_index, input);
      elemA.lcp = lcp_value;
    }
  }
}

void UpdateBorderToRight(int position, Bucket& bucket, vector<SuffixType>& types, string& input) {
  BucketElement& elemA = bucket.elements.at(position);
  if (position == (int)bucket.elements.size() - 1) {
    elemA.lcp = 0;
    return;
  }
  
  int index = bucket.tail + 1;
  if (index < (int)bucket.elements.size()) {
    BucketElement& elemB = bucket.elements.at(index);
    int lcp_value = Lcp(elemA.suffix_index, elemB.suffix_index, input);
    elemA.lcp = lcp_value;
  }
}

/* 4.2) Inserts kL suffixes into buckets and updates lcps. */
void LastStepL(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
  for (int i = 0; i < (int)buckets.size(); i++) {
    for (int j = 0; j < (int)buckets[i].elements.size(); j++) {
      int index = buckets[i].elements[j].suffix_index - 1;
      if (index >= 0 && types.at(index) == kL) {
        Bucket& bucket = GetBucket(buckets, input.at(index));
        if (bucket.head == 0) {
          // there's no kL's in this bucket yet
          BucketElement elem(index, kL, 0);
          bucket.PutFront(elem);
        } else {
          // there are kL's in this bucket
          InsertNotFirstL(index, buckets, bucket, types, input);
        }
        
        //UpdateLSBorder(bucket, types, input);
        if (bucket.tail < (int)bucket.elements.size() - 1) {
          UpdateBorderToLeft(bucket.tail+1, bucket, types, input);
        }
      }
    }
  }
}

/* 4.3) Inserts kS suffixes into buckets and updates lcps. */
void LastStepS(vector<Bucket>& buckets, vector<SuffixType>& types, string& input) {
  for (int i = 0; i < (int)buckets.size(); i++) {
    buckets[i].ResetTailPointer();
  }
  
  for (int i = (int)buckets.size() - 1; i >= 0; i--) {
    for (int j = (int)buckets.at(i).elements.size() - 1; j >= 0; j--) {
      BucketElement element = buckets.at(i).elements.at(j);
      int index = element.suffix_index - 1;
      if (index >= 0 && types.at(index) != kL) {
        Bucket& bucket = GetBucket(buckets, input.at(index));
        if (bucket.tail == (int)bucket.elements.size()-1) {
          // there's no kS 's in this bucket yet
          BucketElement elem(index, types.at(index), 0);
          bucket.PutBack(elem);
          UpdateBorderToLeft(bucket.tail+1, bucket, types, input);
        } else {
          // there are kS 's in this bucket
          InsertNotFirstS(index, buckets, bucket, types, input);
          UpdateBorderToLeft(bucket.tail+1, bucket, types, input);
          UpdateBorderToLeft(bucket.tail+2, bucket, types, input);
        }
      }
    }
  }
}

/* 4. */
vector<Bucket> CalculateLCP(vector<Name>& names, vector<SuffixType>& types, string& input) {
  vector<Bucket> buckets = CreateBuckets(input);
  
  LastStepSStar(buckets, names, types, input);
  
  log("==== 4.1) ====\n");
  foreach(Bucket, buckets) {
    it->Print();
  }
  
  LastStepL(buckets, types, input);
  
  log("==== 4.2) ====\n");
  foreach(Bucket, buckets) {
    it->Print();
  }
  
  LastStepS(buckets, types, input);
  
  return buckets;
}

vector<int> BruteForce(string& input);
void Test1();
vector<int> Test2(string&);

bool AreSame(vector<int>& a, vector<int>& b) {
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

void Print(vector<int>& v) {
  for (int i = 0; i < (int)v.size(); i++) {
    printf("%d ", v[i]);
  }
  printf("\n");
}

string RandomString(int minSize, int maxSize) {
  int size = minSize + (rand() % (maxSize - minSize + 1));
  
  string ret = "";
  for (int i = 0; i < size; i++) {
    char c = 'a' + (rand() % 25);
    ret += c;
  }
  return ret;
}

void BatchTest() {
  const int t = 2000;
  const int size = 500;
  int correct = 0;
  int wrongs = 0;
  srand(time(NULL));
  
  for (int i = 0; i < t; i++) {
    string input = RandomString(size, size) + "$";
    vector<int> actual = Test2(input);
    vector<int> expected = BruteForce(input);
    if (AreSame(actual, expected)) {
      correct++;
    } else {
      wrongs++;
      cout << input << endl;
      printf("actual:   "); Print(actual);
      printf("expected: "); Print(expected);
      if (wrongs > 5) {
        break;
      }
    }
  }
  
  printf("%d/%d\n", correct, t);
}

void test() {
  BatchTest();
  //Test1();
}

vector<int> Test2(string& input) {
  vector<Bucket> buckets = CreateBuckets(input);
  vector<SuffixType> types = CreateSuffixTypeArray(input);
  
  AddSStarSuffix(buckets, types, input);
  
  log("==== 1. ====\n");
  foreach(Bucket, buckets) {
    it->Print();
  }
  
  AddLSuffixes(buckets, types, input);
  
  log("==== 2. ====\n");
  foreach(Bucket, buckets) {
    it->Print();
  }
  
  AddSSuffixes(buckets, types, input);
  
  log("==== 3. ====\n");
  foreach(Bucket, buckets) {
    it->Print();
  }
  
  vector<Name> unsorted_names = GetNames(buckets, types, input);
  vector<Names> categories = GetCategories(unsorted_names);
  vector<Name> names = Flatten(categories, input);
  LcpInitial(names, input);
  buckets = CalculateLCP(names, types, input);
  
  log("==== final ====\n");
  foreach(Bucket, buckets) {
    it->Print();
  }
  
  vector<int> result;
  for (vector<Bucket>::iterator it = buckets.begin(); it != buckets.end(); ++it) {
    for (int i = 0; i < (int)it->elements.size(); i++) {
      result.push_back(it->elements[i].lcp);
    }
  }
  return result;
}

void Test1() {
  //string input = "otorinolaringologija$";
  string input = "aaddadadad$";
  
  try {
    vector<int> actual = Test2(input);
    vector<int> expected = BruteForce(input);
    printf("actual:\n"); Print(actual);
    printf("expected:\n"); Print(expected);
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

int Lcp(int a, int b, string& input) {
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
vector<int> BruteForce(string& input) {
  SuffixComparator cmp(input);
  
  vector<int> v;
  for (int i = 0; i < (int)input.length(); i++) {
    v.push_back(i);
  }
  
  sort(v.begin(), v.end(), cmp);
  
  vector<int> result;
  result.push_back(0);
  for (int i = 1; i < (int)v.size(); i++) {
    int l = Lcp(v.at(i-1), v.at(i), input);
    result.push_back(l);
  }
  return result;
}

