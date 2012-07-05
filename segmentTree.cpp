#include "segmentTree.h"
#include "cilk.h"
#include "Base.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>

using namespace std;

#define LEFT(i) ((i) << 1)
#define RIGHT(i) (((i) << 1) | 1)
#define PARENT(i) ((i) >> 1)

void SegmentTree::BuildTree(int * a, int _n) {
	n = _n;
    depth = getDepth(n);

    all = new int[n];
    table = new int*[depth];

    table[0] = a;
    table[1] = all;
    for (int i = 2; i < depth; i++) {
        table[i] = table[i - 1] + (1 << (depth - i));
    }

    int m = n;
    for (int d = 1; d < depth; d++) {
        int m2 = m / 2;

        cilk_for (int i = 0; i < m2; i++) {
            table[d][i] = min(table[d - 1][LEFT(i)], table[d - 1][RIGHT(i)]);
        }

        if (m % 2) {
            table[d][m2] = table[d - 1][LEFT(m2)];
        }

        m = (m + 1) / 2;
    }
}

void SegmentTree::DeleteTree() {
	delete all;
	delete table;
}

int SegmentTree::Query(int l, int r) {
	return this->query(0, depth - 1, l, r);
}

int SegmentTree::query(int cur, int dep, int l, int r) {
	int range = 1 << dep;
	int leftBound = cur * range;
	int rightBound = min(leftBound + range - 1, n - 1);	//all the bound is incluisve

	if (leftBound >= n) return INFI;	//invaild node [since the tree is not 2^n size]

	if (l > rightBound || r < leftBound) return INFI;	//the ranges have no intersection
	else if (l <= leftBound && r >= rightBound) return table[dep][cur];

	return min(this->query(LEFT(cur), dep - 1, l, r), this->query(RIGHT(cur), dep - 1, l, r));
}
