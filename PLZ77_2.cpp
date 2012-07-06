/*
 * The parallel algirthm for Lempel-ziv 77 compression
 */

#include <cstdio>
#include <iostream>

#include "suffixTree.h"
#include "sequence.h"
#include "Base.h"
#include "test.h"

using namespace std;

pair<int *, int> ParallelLZ77(int *s, int n) {
	startTime();

	suffixTree st = buildSuffixTree(s, n);
	nextTime("\tSuffix Tree");

	int * minLabel = new int[st.m];
	
	int m = 1;
	int * LZ = new int[m]; LZ[0] = 0;
	return make_pair(LZ, m);
}

int main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"Parallel LZ77 using suffix tree (nlogn work)", ParallelLZ77);
}

