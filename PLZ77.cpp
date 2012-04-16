#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <omp.h>

#include "ANSV.h"
#include "RMQ.h"
#include "PrefixSum.h"
#include "rangeMin.h"

#include "sequence.h"
#include "intSort.h"
#include "utils.h"
#include "Base.h"

#include "ansv2.h"
//using namespace std;

///////////////////////////////////////////////

double stamp;

double gettime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

double start() {
	stamp = gettime();
}

double report(std::string s) {
	double now = gettime();
	printf("%s: %.3fs\n", s.c_str(), now - stamp);
	stamp = now;
}
///////////////////////////////////////////////

inline int getLCP(int **table, int l, int r) {
	//actually need following to ensure correctness all the time
	//if (l + 1 == r) return lcp[r];
	//if (l == r) return n - l;
	return queryRMQ(table, l + 1,  r);
}

void getLPF(int *sa, int n, int *lcp, int *lpf) {
	int d = getDepth(n);
	int *l = myalloc(n), *r = myalloc(n);
	int ** table = (int **)myalloc(d*sizeof(int*)/sizeof(int));
	
	printf("dsf");

	#pragma omp parallel for
	for (int i = 0; i < d; i++) 
		table[i] = myalloc(n);

	start();
	ComputeANSV(sa, n, l, r, table);

	report("ansn");
	
/* 
	printf("%d\n", d);
	for (int i = 0; i < n; i++) printf("%2d ", sa[i]);printf("\n");
	for (int i = 0; i < n; i++) printf("%2d ", l[i] >= 0 ? sa[l[i]] : -1);printf("\n");
	for (int i = 0; i < n; i++) printf("%2d ", r[i] >= 0 ? sa[r[i]] : -1);printf("\n");
	printf("table:\n");
	for (int j = 0; j < d; j++) {
		printf("%2d: ", j);
		for (int i = 0; i < n; i++) printf("%2d ", table[j][i]);
		printf("\n");
	}
 */

	buildRMQ(lcp, n, table);
	report("rmq");
	
	#pragma omp parallel for
	for(int i = 0; i < n; i++) {
  	int llcp = 0, rlcp = 0;
  	if (l[i] != -1) 
  		llcp = getLCP(table, l[i], i);
  	if (r[i] != -1)
  		rlcp = getLCP(table, i, r[i]);
  		
  	lpf[sa[i]] = max(llcp, rlcp);
  }
  report("lpf");
  
	mydealloc(n);
	mydealloc(n);
	mydealloc(n*d+d);
}

//some optimization require n >= 8
int getLZ(int *lpf, int n, int *lz) {
	int l2 = cflog2(n);
	int depth = l2 + 1;
	int nn = 1 << l2;
	
	//printf("%d %d %d\n", nn, n, depth);
	
	int *flag = myalloc(max(nn, n + 1));
//	report("alloc");
	
	#pragma omp parallel for
	for(int i = 0; i < n; i++) {
		flag[i] = 0;
		lpf[i] = min(n, i + max(lpf[i], 1));
	}
	report("prepare"); //combine performance would be better due to cache miss
	
	int sn = (n + l2 - 1) / l2;
	
	int * next = lz, *next2 = lz + sn + 1;
	int * sflag = myalloc(sn);
	
	next[sn] = next2[sn] = sn; 
	
	//build the sub tree
 	#pragma omp parallel for
 	for (int i = 0; i < sn; i ++) {
		int j;
		for(j = lpf[i*l2]; j % l2 && j != n; j = lpf[j]) ;
		if (j == n) next[i] = sn;
		else next[i] = j / l2;
		sflag[i] = 0;
	}
	sflag[0] = 1;
	
	//point jump
	for (int d = 0; sflag[sn] == 0; d++) {
		#pragma omp parallel for
		for(int i = 0; i < sn; i ++) {
			int j = next[i];
			if (sflag[i] == 1 && sflag[j] == 0) sflag[j] = 1;			
			next2[i] = next[j];
		}
		std::swap(next, next2);
	}
	
	//filling the result
 	#pragma omp parallel for
 	for (int i = 0; i < n; i += l2) {
		if (sflag[i / l2]) {
			for(int j = i; j % l2 && j !=n; j = lpf[j]) flag[j] = 1;
		}
	}

	report("point jump");
	
	//for (int i = 0; i <= n; i++) printf("%2d ", flag[i]);printf("\n");
	//exclusiveScan(flag, nn);
	sequence::scan(flag, flag, nn, utils::addF<int>(),0);

	report("prefix sum");
	//for (int i = 0; i <= n; i++) printf("%2d ", flag[i]);printf("\n");
	
	int m = flag[n-1];
	
	#pragma omp parallel for
	for(int i = 0; i < n; i++) {	
		if (flag[i] < flag[i+1]) {
			lz[flag[i]] = i;
		}
	}
	report("combine result");
	
	mydealloc(sn); //sflag
	mydealloc(max(nn, n + 1));

	return m;
}

void checkANSV(int d) {
	int n = 1 << d;
	int *a = new int[n];
	

	for ( int i = 0; i < n; i++)
		a[i] = min(rand() % n, n-1-i);
		
	start();
		int *ql = new int[n], *qr = new int[n];
	int **table = new int*[d+1];
		for (int i = 0; i < d+1; i++) table[i] = new int[n];
		
	ComputeANSV(a, n, ql, qr, table);

	report("ansv");	
	
	ANSV an(a, n);
	report("ansv2");
	sequence::scan(a, a, n, utils::addF<int>(),0);
	report("prefix sum");

	int *l = an.getLeftNeighbors();
	int *r = an.getRightNeighbors();
	int i;
	for (i = 0; i < n; i++) {
		if (l[i] != ql[i]) {printf("l: %d %d %d\n", i, l[i], ql[i]);break;}
		if (r[i] != qr[i]) {printf("l: %d %d %d\n", i, r[i], qr[i]);break;}
	}
	if (i == n) printf("pass\n");
	
	printf("\n");
		
	for (int i = 0; i < d+1; i++) delete table[i];
	delete table;
	
	delete ql;
	delete qr;
	
	delete a;
}

void checkLZ(int d) {
	int n = 1 << d;
	int *a = new int[n+1], *ql = new int[n+1], *qr = new int[n+1];
	int i;	
	for ( i = 0; i < n; i++) 
		a[i] = min(rand() % n, n-1-i);
	a[0] = 0;
	
	start();
  for (int i = 1; i < n; i++) {
		ql[i] = i + max(1, a[ql[i-1]]);
	}
	report("lz seq");
  
  printf("lz par:\n");
	getLZ(a, n, ql);
	
	printf("\n");
}

void checkRMQ( int d) {
	int n = 1 << d;
	int *a = new int[n], *ql = new int[n], *qr = new int[n];
	int **table = new int*[d+1];
	
	int i;
	for (i = 0; i < d+1; i++) table[i] = new int[n];
	
	for ( i = 0; i < n; i++) {
		a[i] = min(rand() % n, n-1-i);
		
		int l = rand() % n;
		int r = rand() % n;
		if (l == n - 1) l--;
		if (r <= l) r = l + 1;
		ql[i] = l;
		qr[i] = r;
	} 
	
	start();
 buildRMQ(a, n, table);
	report("rmq1 build");
	#pragma omp parallel for
	for (int i = 0; i < n; i++) {
		queryRMQ(table, ql[i], qr[i]);
		queryRMQ(table, ql[i], qr[i]);
	}
	report("rmq1 query");
	
	start();
	myRMQ rq(a, n);
	rq.precomputeQueries();
	report("rmq2 build");
	#pragma omp parallel for
	for (int i = 0; i < n; i++) {
		rq.query(ql[i], qr[i]);
		rq.query(ql[i], qr[i]);
	}
	report("rmq2 query");
	printf("\n");
	
	for (i = 0; i < d+1; i++) delete table[i];
	delete table;
	delete a;
	delete ql;
	delete qr;
}

void checkScan( int d) {
	int n = 1 << d;
	int *a = new int[n], *b = new int[n], *c = new int[n];
	
	int i;
	
	for ( i = 0; i < n; i++) {
		a[i] = rand() % n;
	} 
	start();
	b[0] = 0;
	for ( i = 1; i < n; i++) 
		b[i] = a[i-1] + b[i-1];
	report("seq  prefix sum");

	sequence::scan(a, c, n, utils::addF<int>(),0);
	report("par prefix sum2");
	
	exclusiveScan(a, n);
	report("par prefix sum1");
	
	for ( i = 1; i < n; i++) 
		if (c[i] != b[i] || b[i] != a[i]) break;

	if (i == n) printf("prefix sum pass\n");
	else {
		printf("prefix sum fail\n");
		for (i = 1; i < 10; i++) {
			printf("(%d %d %d)", a[i], b[i], c[i]);
		}	
		printf("\n");
	}

	delete a;
	delete b;
	delete c;
}

int cal(int i) {
		int k = 0;
		for (int j = 0; j < 1000; j++) {
			k += j;
		}
		return k;
}

void checkSimple( int d) {
	int n = 1 << d;
	int *a = new int[128];
	
	printf("tot: %d\n", n);
	
	start();

 	for (int i = 0; i < n; i++) {
		a[i % 128] = cal(i);
	}
	report("seq");
	
	start();
	#pragma omp parallel for
	for (int i = 0; i < n; i++) {
		a[i % 128] = cal(i);
	}
	report("par");
	
	start();
	int p = omp_get_num_threads();
	#pragma omp parallel
 {
		int tid = omp_get_thread_num();
		int work = n / p;
		int l = tid * work;
		int r = min(n, l+work);
		//printf("%d %d %d\n", tid, l, r);
		for(int i = l; i < r; i++) {
				a[i % 128] = cal(i);
		}
	}
	report("par2");
	
	start();
	for (int i = 0; i < n; i++) {
		a[i % 128] = cal(i);
	}
	report("seq");
	
	delete a;
}

#define MAXN 64

int sa[MAXN] =  {8, 9, 3, 12, 10, 0, 4, 13, 7, 2, 11, 6, 1, 5};
int lcp[MAXN] = {0, 2, 3, 1,  2,  2, 3, 0,  1, 3, 2,  1, 4, 2};

int lpf[MAXN];
int lz[MAXN];

void checkCorrect() {
	int n = 14;

	getLPF(sa, n, lcp, lpf);
	printf("lpf:\n"); for (int i = 0; i < n; i++) printf("%2d ", lpf[i]); printf("\n");	


	int m = getLZ(lpf, n, lz);

	printf("lz: "); for (int i = 0; i < m; i++)	printf("%2d ", lz[i]); printf("\n");
	printf("st:  0  1  2  3  4  7 10 12\n");

}

int main(int argc, char *argv[]) {

	int p = 2, d = 28;
	for (int i = 1; i < argc; i++) {
		//printf("%s\n", argv[i]);
		if (i == 1) p = atoi(argv[i]);
		if (i == 2) d = atoi(argv[i]);
	}
	omp_set_num_threads(p);


//	printf("%d %d\n", p, d); 
	//checkSimple(p,d);
	//checkScan(p, d);
	//checkRMQ( d);
	//checkLZ(d);
	checkANSV(d);
	//checkCorrect();
	
//	for (int i = 0; i < 17; i++) 
//		printf("%d %d\n", (int)log2(i), fflog2(i)); 

	
	return 0;
}




