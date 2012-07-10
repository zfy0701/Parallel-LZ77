#include "BWT.h"

#include <iostream>
#include <cstdio>

#include "sequence.h"
#include "intSort.h"
#include "utils.h"
using namespace std;

inline void radixSortPair(pair<int,int> *A, int n, long m) {
  //intSort::iSort(A,n,m,utils::firstF<int,int>());
}

int * BurrowsWheelerTransform(int * a, int n) {
	timer bwtTm;
    bwtTm.start();
	
	int k = 1 + sequence::reduce(a, n, utils::maxF<int>());
	pair<int, int> *b = new pair<int, int>[n];

	parallel_for(int i = 0; i < n; i++) {
		b[i].first = a[i];
		b[i].second = i;
	}

	radixSortPair(b, n, k);

	int * c = new int[n];
	parallel_for (int i = 0; i < n; i++) {
		c[i] = a[(b[i].second + n) % n];
	}
	bwtTm.reportNext("\t****** BWT Proprecess *****");
	return c;
}
