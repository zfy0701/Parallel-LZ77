#include "PrefixSum.h"
#include "Base.h"
#include "sequence.h"

#include <stdio.h>
#include <iostream>
#include <omp.h>
using namespace std;

void exclusiveScan(int *a, int n) {
	sequence::scan(a, a, n, utils::addF<int>(),0);
	return;
	
	int depth = getDepth(n);

	#pragma omp parallel
	{
		int tid = omp_get_thread_num();
		
		//up sweep
		int dist = 2;
		for (int d = 1; d < depth; d++) {
			#pragma omp for schedule(dynamic, 1024) 
			for(int i = dist - 1; i < n; i += dist) {
					a[i] += a[i - (dist>>1)];
			}
			
			dist <<= 1;
			#pragma omp barrier
		}
		
		if (tid == 0) a[n-1] = 0; 
		#pragma omp barrier
		
		//down sweep
		dist = 1 << (depth - 1);
		for (int d = depth - 2; d >= 0; d--) {		
			#pragma omp for schedule(dynamic, 1024) 
			for(int i = dist - 1; i < n; i += dist) {
					int j = i-(dist>>1);
					int t = a[j];
					a[j] = a[i];
					a[i] += t;
			}
			dist >>= 1;
			#pragma omp barrier
		}
	}
}