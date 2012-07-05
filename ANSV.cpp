#include "ANSV.h"
#include "Base.h"
#include "parallel.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>

using namespace std;

#define LEFT(i) ((i) << 1)
#define RIGHT(i) (((i) << 1) | 1)
#define PARENT(i) ((i) >> 1)

const int BLOCK_SIZE = 8192;

inline int getLeft_opt(int **table, int depth, int n, int index, int start) {
	int value = table[0][index];
	if (value == table[depth - 1][0]) return -1;

	int cur = PARENT(start), d, dist = 2;
	for (d = 1; d < depth; d++) {
		if ((cur + 1) * dist > index + 1) cur --;
		if (cur < 0) return -1;

		if (table[d][cur] >= value) cur = PARENT(cur);
		else break;

		dist <<= 1;
	}

	for ( ; d > 0; d--) {
		if (table[d - 1][RIGHT(cur)] < value) cur = RIGHT(cur);
		else cur = LEFT(cur);
	}
	return cur;
} 

inline int getRight_opt(int **table, int depth, int n, int index, int start) {
	int value = table[0][index];
	if (value == table[depth - 1][0]) return -1;

	int cur = PARENT(start), d, dist = 2;
	for (d = 1; d < depth; d++) {
		if (cur * dist < index) cur ++;
		if (cur * dist >= n) return -1;

		if (table[d][cur] >= value) cur = PARENT(cur);
		else break;

		dist <<= 1;
	}

	for ( ; d > 0; d--) {
		if (table[d - 1][LEFT(cur)] < value) cur = LEFT(cur);
		else cur = RIGHT(cur);
	}
	return cur;
}


void ComputeANSV_Linear(int a[], int n, int leftElements[], int rightElements[], int offset) {
    int i, top;
    int *stack = new int[n];

    for (i = 0, top = -1; i < n; i++) {
        while (top > -1 && a[stack[top]] > a[i]) top--;
        if (top == -1) leftElements[i] = -1;
        else leftElements[i] = stack[top] + offset;
        stack[++top] = i;
    }

    for (i = n - 1, top = -1; i >= 0; i--) {
        while (top > -1 && a[stack[top]] > a[i]) top--;
        if (top == -1) rightElements[i] = -1;
        else rightElements[i] = stack[top] + offset;
        stack[++top] = i;
    }
    delete stack;
}

void ComputeANSV(int * a, int n, int *left, int *right) {
    int l2 = cflog2(n);
    int depth = l2 + 1;

	int *all = new int[n];
	int **table = new int*[depth];

	table[0] = a;
	table[1] = all;
	for (int i = 2; i < depth; i++) {
		table[i] = table[i - 1] + (1 << (depth - i));
	}

	int m = n;
	for (int d = 1; d < depth; d++) {
		int m2 = m / 2;

		parallel_for (int i = 0; i < m2; i++) {
			table[d][i] = min(table[d - 1][LEFT(i)], table[d - 1][RIGHT(i)]);
		}

		if (m % 2) {
			table[d][m2] = table[d - 1][LEFT(m2)];
		}

		m = (m + 1) / 2;
	}

  	parallel_for (int i = 0; i < n; i += BLOCK_SIZE) {
  		int j = min(i + BLOCK_SIZE, n);
  		ComputeANSV_Linear(a + i, j - i, left + i, right + i, i);

  		int tmp = i;
  		for (int k = i; k < j; k++) {
  			if (left[k] == -1) {
  				if ( tmp != -1 && a[tmp] >= a[k]) {
  					//if (left[tmp] != -1)
  					//	tmp = left[tmp];
					tmp = getLeft_opt(table, depth, n, k, tmp);
				}
				left[k] = tmp;
  			}
  		}

  		tmp = j - 1;
  		for (int k = j - 1; k >=  i; k--) {
  			if (right[k] == -1) {
  				if (tmp != -1 && a[tmp] >= a[k]) {
  					//if (right[tmp] != -1)
  					//	tmp = right[k];
	  				tmp = getRight_opt(table, depth, n, k, tmp);
  				}
  				right[k] = tmp;
  			}
  		}
  	}

  	delete table;
	delete all;
}