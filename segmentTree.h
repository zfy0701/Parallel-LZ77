#ifndef _SEGMENT_TREE_H
#define _SEGMENT_TREE_H
#include "parallel.h"
class SegmentTree {
private:
	static const int INFI = 1 << 30;
	intT n;
	intT **table;
	intT *all;
	intT depth;
	intT query(intT cur, intT d, intT l, intT r);
public:
	void BuildTree(intT *a, intT n);
	intT Query(intT l, intT r);
	void DeleteTree();
};

#endif
