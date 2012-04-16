#include "ANSV.h"
#include "Base.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

using namespace std;

#define LEFT(i) ((i) << 1)
#define RIGHT(i) (((i) << 1) | 1)
#define PARENT(i) ((i) >> 1)

inline int getLeft(int **table, int depth, int n, int index) {
	int value = table[0][index];
	if (value == table[depth - 1][0]) return -1;
	
	int cur = PARENT(index), d, dist = 2;
	for (d = 1; d < depth; d++) {		
		if ((cur + 1) * dist > index + 1) cur --;
		if (cur < 0) return -1;
		
		if (table[d][cur] >= value)	cur = PARENT(cur);
		else break;
		
		dist <<= 1;
	}	
	
	for ( ; d > 0; d--) {
		if (table[d-1][RIGHT(cur)] < value) cur = RIGHT(cur);
		else cur = LEFT(cur);
	}
	return cur;
}

inline int getRight(int **table, int depth, int n, int index) {
	int value = table[0][index];
	if (value == table[depth - 1][0]) return -1;
	
	int cur = PARENT(index), d, dist = 2;
	for (d = 1; d < depth; d++) {	
		if (cur * dist < index) cur ++;
		if (cur * dist >= n) return -1;
		
		if (table[d][cur] >= value)	cur = PARENT(cur);
		else break;
		
		dist <<= 1;
	}	
	
	for ( ; d > 0; d--) {
		if (table[d-1][LEFT(cur)] < value) cur = LEFT(cur);
		else cur = RIGHT(cur);
	}
	return cur;
}

void ComputeANSV(int * a, int n, int *left, int *right, int **table) {
	int depth = getDepth(n);

	#pragma omp parallel for 
	for(int i = 0; i < n; i++) {
      table[0][i] = a[i];
  }

	int m = n;
	for (int d = 1; d < depth; d++) {
		int m2 = m / 2;
		
		#pragma omp parallel for 
		for(int i = 0; i < m2; i++) {
    	table[d][i] = min(table[d-1][LEFT(i)], table[d-1][RIGHT(i)]);
		}
		
		if (m % 2) {
			table[d][m2] = table[d-1][LEFT(m2)];
		}
		
		m = (m + 1) / 2;
	}	
 
	#pragma omp parallel for
	for(int i = 0; i < n; i++) {
		left[i] = getLeft(table, depth, n, i);
		right[i] = getRight(table, depth, n, i);
  }
  
  #pragma omp parallel for
	for(int i = 0; i < n; i++) {
//		right[i] = getRight(table, depth, n, i);
  }
  //mydealloc(n*2);
}