//some optimization require n >= 8

#include <cstdio>
#include <iostream>
#include "Base.h"
#include "sequence.h"
#include "parallel.h"

using namespace std;

//this module is share by others
//not test for n < 8

pair< pair<intT, intT>*, intT> ParallelLPFtoLZ(intT *lpf, intT* prev_occ, intT n) {

    // if(n <= 16)
    // for (int i = 0; i< n; i++)
    //     printf("%d\t", lpf[i]);
    //    printf("\n");
    intT l2 = cflog2(n);
    intT depth = l2 + 1;
    int nn = 1 << l2;
    intT* pointers = new intT[n];
    //printf("%d %d %d\n", nn, n, depth);

    //printf("mem base %d\n", bm);
    intT *flag = new intT[n+1];
    //int *flag = new int[(max(nn, n + 1))];

    //nextTime("alloc");
    parallel_for (intT i = 0; i < n; i++) {
        flag[i] = 0;
        pointers[i] = min(n, i + max<intT>(lpf[i], 1));
    }
    flag[n] = 0;

//    nextTime("\tprepare"); //combine performance would be better due to cache miss

    l2 = max<intT>(l2, 256);
    intT sn = (n + l2 - 1) / l2;

    intT * next = new intT[sn+1], *next2 = new intT[sn+1];
    intT * sflag = new intT[sn+1];

    //build the sub tree
    parallel_for (intT i = 0; i < sn; i ++) {
        intT j;
        for(j = pointers[i*l2]; j % l2 && j != n; j = pointers[j]) ;
        if (j == n) next[i] = sn;
        else next[i] = j / l2;
        sflag[i] = 0;
    }

    next[sn] = next2[sn] = sn;
    sflag[0] = 1; sflag[sn] = 0;

    //point jump
    intT dep = getDepth(sn); ;
    for (intT d = 0; d < dep; d++) {
        parallel_for(intT i = 0; i < sn; i ++) {
            intT j = next[i];
            if (sflag[i] == 1) {
                sflag[j] = 1;
            }
            // printf("\n");
            next2[i] = next[j];
        }
        std::swap(next, next2);
    }

    //filling the result
    parallel_for (intT i = 0; i < n; i += l2) {
        if (sflag[i / l2]) {
            //printf("adsf");
            flag[i] = 1;
            for(intT j = pointers[i]; j % l2 && j != n; j = pointers[j]) {
                flag[j] = 1;
            }
        }
    }
    delete sflag; delete next; delete next2; delete pointers;
//    nextTime("\tpoint jump");

    sequence::scan(flag, flag, n+1, utils::addF<intT>(),(intT)0);

//    nextTime("\tprefix sum");

    intT m = flag[n];
    pair<intT,intT>* lz = new pair<intT,intT>[m];


    parallel_for(intT i = 0; i < n; i++) {
        if (flag[i] < flag[i+1]) {
      lz[flag[i]] = make_pair(i,prev_occ[i]);
        }
    }
    delete flag;

    return make_pair(lz, m);
}
