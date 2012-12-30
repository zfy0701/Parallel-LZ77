#include "segmentTree.h"
#include "parallel.h"
#include "Base.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>

using namespace std;

#define LEFT(i) ((i) << 1)
#define RIGHT(i) (((i) << 1) | 1)
#define PARENT(i) ((i) >> 1)

void SegmentTree::BuildTree(intT * a, intT _n) {
	n = _n;
    depth = getDepth(n);
    table = new intT*[depth];
    
    table[0] = a;
    intT m = n;
    for (intT i = 1; i < depth; i++) {
        m = (m + 1) / 2;
        table[i] = new intT[m];
    }

    m = n;
    for (intT d = 1; d < depth; d++) {
        intT m2 = m / 2;

        parallel_for (intT i = 0; i < m2; i++) {
            table[d][i] = min(table[d - 1][LEFT(i)], table[d - 1][RIGHT(i)]);
        }

        if (m % 2) {
            table[d][m2] = table[d - 1][LEFT(m2)];
        }

        m = (m + 1) / 2;
    }
}

void SegmentTree::DeleteTree() {
	for (intT i = 1; i < depth; i++) delete table[i];
    delete table;
}

intT SegmentTree::Query(intT l, intT r) {
	return this->query(0, depth - 1, l, r);
}

intT SegmentTree::query(intT cur, intT dep, intT l, intT r) {
	intT range = 1 << dep;
	intT leftBound = cur * range;
	intT rightBound = min(leftBound + range - 1, n - 1);	//all the bound is incluisve

	if (leftBound >= n) return INFI;	              //invaild node [since the tree is not 2^n size]

	if (l > rightBound || r < leftBound) return INFI;	//the ranges have no intersection
	else if (l <= leftBound && r >= rightBound) return table[dep][cur];

	return min(this->query(LEFT(cur), dep - 1, l, r), this->query(RIGHT(cur), dep - 1, l, r));
}
