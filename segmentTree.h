#ifndef _SEGMENT_TREE_H
#define _SEGMENT_TREE_H

class SegmentTree {
private:
	static const int INFI = 1 << 30;
	int n;
	int **table;
	int *all;
	int depth;
	int query(int cur, int d, int l, int r);
public:
	void BuildTree(int *a, int n);
	int Query(int l, int r);
	void DeleteTree();
};

#endif