/*
 * The parallel algirthm for Lempel-ziv 77 compression
 */

#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <omp.h>

#include "ANSV.h"
#include "RMQ.h"
//#include "PrefixSum.h"
#include "rangeMin.h"

#include "sequence.h"
#include "intSort.h"
#include "utils.h"
#include "Base.h"
#include "mysort.h"
#include "merge.h"

//#include "ansv2.h"
//using namespace std;

///////////////////////////////////////////////
pair<int*,int*> suffixArray(int* s, int n, bool findLCPs);

inline int getLCP(int **table, int l, int r) {
    //actually need following to ensure correctness all the time
    //if (l + 1 == r) return lcp[r];
    //if (l == r) return n - l;
    return queryRMQ(table, l + 1,  r);
}

void getLPF(int *sa, int n, int *lcp, int *lpf) {
    int d = getDepth(n);
    int *l = new int[n], *r = new int[n];

    ComputeANSV(sa, n, l, r);
    //nextTime("ansn");

    // int *table[32];
  //  for (int i = 0; i < d; i++) table[i] = new int[n];
  //  buildRMQ(lcp, n, table);
    myRMQ rmq(lcp, n);
    //rmq.precomputeQueries();

    //nextTime("rmq");
    
    cilk_for (int i = 0; i < n; i++) {
        int llcp = 0, rlcp = 0;
        if (l[i] != -1) {
            //llcp = getLCP(table, l[i], i);
            llcp = lcp[rmq.query(l[i]+1, i)];
        }
        if (r[i] != -1) {
            //rlcp = getLCP(table, i, r[i]);
            rlcp = lcp[rmq.query(i+1, r[i])];
        }

        lpf[sa[i]] = max(llcp, rlcp);
    }
    //nextTime("lpf");
    
    delete l; delete r;

    // for (int i = 0; i < d; i++) 
    //      delete table[i] ;
}

//some optimization require n >= 8
int getLZ(int *lpf, int n, int *lz) {
    int l2 = cflog2(n);
    int depth = l2 + 1;
    int nn = 1 << l2;
    
    //printf("%d %d %d\n", nn, n, depth);
    
    //printf("mem base %d\n", bm);
    int *flag = new int[n+1];
    //int *flag = new int[(max(nn, n + 1))];

    //nextTime("alloc");
    cilk_for (int i = 0; i < n; i++) {
        flag[i] = 0;
        lpf[i] = min(n, i + max(lpf[i], 1));
    }
    flag[n] = 0;
    
    nextTime("\tprepare"); //combine performance would be better due to cache miss

    //for (int i = 0; i < n; i++) printf("%2d ", lpf[i]);printf("\n");

    l2 = max(l2, 256);
    int sn = (n + l2 - 1) / l2;
    
    int * next = new int[sn+1], *next2 = new int[sn+1];
    int * sflag = new int[sn+1];
    

    //build the sub tree
    cilk_for (int i = 0; i < sn; i ++) {
        int j;
        for(j = lpf[i*l2]; j % l2 && j != n; j = lpf[j]) ;
        if (j == n) next[i] = sn;
        else next[i] = j / l2;
        sflag[i] = 0;
    }
    next[sn] = next2[sn] = sn; 
    sflag[0] = 1; sflag[sn] = 0;
 //    for (int i = 0; i <= sn; i++) printf("%2d ", i);printf("\n");
 //   for (int i = 0; i <= sn; i++) printf("%2d ", next[i]);printf("\n");
    //point jump
    int dep = getDepth(sn); ;
    for (int d = 0; d < dep; d++) {
        cilk_for(int i = 0; i < sn; i ++) {
            int j = next[i];
            if (sflag[i] == 1) {
                sflag[j] = 1;
                //printf("%d ", j);
               //sflag2[j] = 1;
               //sflag2[i] = 1;
            } 
            // printf("\n");
            next2[i] = next[j];
        }
        std::swap(next, next2);
    }

    //filling the result
    cilk_for (int i = 0; i < n; i += l2) {
        if (sflag[i / l2]) {
            //printf("adsf");
            flag[i] = 1;
            for(int j = lpf[i]; j % l2 && j != n; j = lpf[j]) {
                flag[j] = 1;
                //printf("write");
            }
        }
    }

    nextTime("\tpoint jump");
    
    //for (int i = 0; i <= sn; i++) printf("%2d ", sflag[i]);printf("\n");
    
    //exclusiveScan(flag, nn);
    sequence::scan(flag, flag, n+1, utils::addF<int>(),0);

    //for (int i = 0; i < n; i++) printf("%2d ", flag[i]);printf("\n");

    nextTime("\tprefix sum");
    //for (int i = 0; i <= n; i++) printf("%2d ", flag[i]);printf("\n");
    
    int m = flag[n];
    //lz = new int[m];
    
    cilk_for(int i = 0; i < n; i++) {    
        if (flag[i] < flag[i+1]) {
            lz[flag[i]] = i;
        }
    }
    //nextTime("combine result"); //check a bit about out of boundary
    
    delete flag; delete sflag; delete next; delete next2;

    return m;
}

int ParallelLZ77(int *s, int n, int * lz) {
    startTime();

    pair<int *, int*> salcp = suffixArray(s, n, true);
    nextTime("\tsuffix array");

    int *sa = salcp.first;
    //int *lcp = salcp.second - 1;
    //lcp[0] = 0;   //TODO check safety
    int *lcp = new int[n];
    lcp[0] = 0;
    cilk_for (int i = 1; i < n; i++) 
        lcp[i] = salcp.second[i-1];

    //for (int i = 0; i < n; i++) {printf("%d ", sa[i]);} puts("");
    //for (int i = 0; i < n; i++) {printf("%d ", lcp[i]);} puts("");

    int *lpf = new int[n];
    getLPF(sa, n, lcp, lpf);

    //for (int i = 0; i < n; i++) {printf("%d ", lpf[i]);} puts("");
    int m = getLZ(lpf, n, lz);

    delete salcp.first;
    delete salcp.second;
    delete lpf;

    return m;
}
