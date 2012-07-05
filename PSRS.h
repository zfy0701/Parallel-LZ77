
#ifndef _MYSORT_H
#define _MYSORT_H

#include <iostream>
#include <algorithm>
#include <queue>
#include "cilk.h"
#include "Base.h"
#include "merge.h"
#include "sequence.h"
#include "utils.h"

#define KSIZE 64
#define HSIZE 128

using namespace std;

template <class ET>
void kmerge(ET **st, ET **ed, ET *r, int k) {

    int depth = getDepth(k);
    int nthread = get_threads();

    int i;

    for (i = 0; i < k; ) {
        if (st[i] == ed[i]) {
            st[i] = st[k - 1];
            ed[i] = ed[k - 1];
            k--;
        } else i++;
    }
    if (k == 0) return;

    if (k > KSIZE) {
        std::priority_queue<pair<ET, int>, vector<pair<ET, int> >, greater<pair<ET, int> > > pq;
        for (i = 0; i < k; i++) pq.push( make_pair(*st[i], i) );

        // pair<ET, int> heap[HSIZE];
        // for (i = 0; i < k; i++) heap[i] = make_pair(*st[i], i);
        // std::make_heap(heap, heap + k, greater<pair<ET, int> >() );

        int kk = k;
        while (kk > KSIZE) {
            pair<ET, int> v = pq.top(); pq.pop();
            //pair<ET, int> v = heap[0];
            //pop_heap (heap, heap + kk);

            *(r++) = v.first;

            if (++st[v.second] != ed[v.second]) {
                pq.push(make_pair(*(st[v.second]), v.second));
                // heap[kk-1] = make_pair(*(st[v.second]), v.second);
                // push_heap(heap, heap + kk);
            } else {
                kk--;
            }
        }

        // while (kk > 0) {
        //   pair<ET,int> v = pq.top(); pq.pop();
        //   *(r++) = v.first;
        //   ++st[v.second];
        //   kk--;
        // }

        // delete heap;
    }

    for (i = 0; i < k; ) {
        if (st[i] == ed[i]) {
            st[i] = st[k - 1];
            ed[i] = ed[k - 1];
            k--;
        } else i++;
    }
    if (k == 0) return;


    while (k > 2) {
        ET min_val = *st[0];
        int min_ind = 0;

        for (i = 1; i < k; i++) { //chose the minimum by brute force
            if (*st[i] < min_val) {
                min_val = *st[i];
                min_ind = i;
            }
        }

        st[min_ind]++;
        if (st[min_ind] == ed[min_ind]) {
            st[min_ind] = st[k - 1];
            ed[min_ind] = ed[k - 1];
            k--;
        }
        *(r++) = min_val;
    }

    if (k == 2) {
        std::merge(st[0], ed[0], st[1], ed[1], r);
    } else
        while (st[0] != ed[0]) *(r++) = *(st[0]++);
}


template <class E>
inline void Sublists(E * a, int start, int end, int * subsize, int at, E * pivots, int fp, int lp) {
	int mid = (fp + lp) / 2;
	E pv = pivots[mid];
	int lb = start;
	int ub = end;

	while (lb <= ub) {
		int center = (lb + ub) / 2;
		if (pv < a[center]) ub = center - 1;
		else lb = center + 1;
	}
	subsize[at + mid] = lb;
	if (fp < mid)
		Sublists(a, start, lb - 1, subsize, at, pivots, fp, mid-1);
	if (mid < lp)
		Sublists(a, lb, end, subsize, at, pivots, mid+1, lp);
}

template <class E>
void ParallelSortRS(E * a, int n) {
	//todo ensure sequentail sort for n < BSIZE
	int p = get_threads();

	if (p == 1) {
		std::sort(a, a+n);
		return;
	}
	
	//timer t;
	//t.start();

	int size = (n + p - 1) / p;
	int rsize = (size + p - 1) / (p); 
	int sample_size = p * (p - 1);
	E *sample = new E[sample_size];
	E *pivots = new E[p];
	int *subsize = new int[p*p+p];
	int *bucket = new int[p];
	E *b = new E[n];

	
	//t.reportNext("\t\t\t prepare");
	//printf("para: size %d, rsize %d sample_size %d\n", size, rsize, sample_size);
	cilk_for (int i = 0; i < p; i++) {
		int start = i * size;
		int end = min(n, start + size);

		std::sort(a + start, a + end);
		//for (int j = start; j < end; j++) printf("%d ", a[j]); printf("\n");

		for (int j = 0; j < p - 1; j++) {
			int k = start + (j+1)*rsize;
			sample[(p-1)*i + j] = k < end ? a[k] : a[n-1];
		}
	}
	std::sort(sample, sample + sample_size);
	//t.reportNext("\t\t\t local sort");

//	for (int i = 0; i < sample_size; i++) printf("%d ", sample[i]); printf("\n");

	//pivots[0] = sample[0];
	for (int i = 0; i < p - 1; i++) {
		pivots[i+1] = sample[i*p+p/2];
	}
	//for (int i = 0; i < p; i++) printf("%d ", pivots[i]); printf("\n");

	cilk_for (int i = 0; i < p; i++) {
		int start = i * size;
		int end = min(n, start + size);

		subsize[i*(p+1)] = start;
		subsize[i*(p+1)+p] = end;
		Sublists(a, start, end - 1, subsize, i*(p+1), pivots, 1, p - 1);
	}
	//t.reportNext("\t\t\t divide");

	// for (int i = 0; i < p; i++) {
	// 	for (int j = 0; j <= p; j++) {
	// 		int k = i * (p+1) + j;
	// 		printf("%d ", subsize[k]);
	// 	}
	// 	printf("\n");
	// }

	cilk_for (int i = 0; i < p; i++) {
		bucket[i] = 0;
		for (int j = i; j <  p * (p+1); j += p+1) {
			bucket[i] += subsize[j+1] - subsize[j];
		}
	}

	//for (int i = 0; i < p; i++) printf("%d ", bucket[i]); printf("\n");

	//eclusive scan 
	sequence::scan(bucket, bucket, p, utils::addF<int>(),0);
	//t.reportNext("\t\t\t bucket");

	//for (int i = 0; i < p; i++) printf("%d ", bucket[i]); printf("\n");

	cilk_for (int i = 0; i < p; i++) {
		//SeqMerge(a, subsize[i+j*(p+1)], a, subsize[i+j*(p+1)+1], b + bucket2[i], f);
		E **st = new E*[p], **ed = new E*[p];
		for (int j = 0; j < p; j++) {
			int k = i + j * (p+1);
			st[j] = a + subsize[k];
			ed[j] = a + subsize[k+1];
			//printf("%d %d\n", subsize[k], subsize[k+1]);

		}
		kmerge(st, ed, b + bucket[i], p);
		//printf("%d ", bucket[i]);
		//for (int i = 0; i < n; i++) printf("%d ", b[i]); printf("\n");
		//printf("merged\n");
		delete st; delete ed;
	}
	//t.reportNext("\t\t\t merge");
	
	cilk_for (int i = 0; i < n; i++) a[i] = b[i];
	
	delete b;
	delete bucket;
	delete subsize;
	delete sample;
	delete pivots;
	//t.reportNext("\t\t\t done");
	//t.stop();

}

template <class E, class F>
void ParallelMergeSort(E * o, int n, F f) {
	E * b = new E[n];
	E * a = new E[n]; //might be the problem, why?

	int p = get_threads();

	int size = (n + p - 1) / p;
	
	int dep = getDepth(p);

	cilk_for (int i = 0; i < n; i++) 
		a[i] = o[i];
	
	cilk_for (int i = 0; i < n; i+= size) {
		int end = min(n, i + size);
		std::sort(a + i, a + end, f);
	}

	int dist = size;
	for (int d = 1; d < dep; d++) {
		int dist2 = dist << 1;
		for (int i = 0; i < n; i+= dist2) {
			int start = i;
			int end = start + dist;
			if (end < n) {
				merge(a+start, dist, a+end, min(dist, n - end), b+start, f);
				//std::merge(a+start, a+end, a+end, a+min(start+dist2, n), b+start, f);
			} 
			else {
				cilk_for (int i = start; i < n; i++) {
					b[i] = a[i];
				}
			}
		}
		dist = dist2;
		swap(a, b);
	}

	// if (a == o) {
	// 	delete b;
	// } else {
		cilk_for (int i = 0; i < n; i++) {
			o[i] = a[i];
		}
		
		delete a; delete b;
	// 	}
	// 	delete a;
	// }
}

#endif
