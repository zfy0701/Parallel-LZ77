/*
 * The sequential algorithm for Lempel-ziv 77 compression
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include "test.h"
#include "ANSV.h"
#include "suffixArray.h"
using namespace std;

pair< pair<intT, intT>*,intT> LempelZiv(intT *s, intT n) {
    timer lzTm;
    lzTm.start();

    intT i, lpf, l, r;
    intT *leftElements = new intT[n], * rightElements = new intT[n];

    pair<intT *, intT *> res = suffixArray(s, n, 0);

    intT *SA = res.first;
    lzTm.reportNext("\tsuffix array");

    ComputeANSV_Linear(SA, n, leftElements, rightElements);

    lzTm.reportNext("\tANSV");

    intT *Rank = new intT[n];
    for (i = 0; i < n; i++) {
        Rank[SA[i]] = i;
    }

    intT k = 0;
    //intT *LZ = Rank;
    pair<intT,intT> *LZ = new pair<intT,intT>[n];

    for (k = i = 0; i < n; i += max<intT>(1, lpf)) {
        intT left = leftElements[Rank[i]], right = rightElements[Rank[i]];

        l = r = 0;

        if (left != -1) while (s[SA[left] + l] == s[i + l])
                l++;
        if (right != -1) while (s[SA[right] + r] == s[i + r])
                r++;

        //LZ[k++] = i;
	LZ[k].first = i;
	if(l==0 && 0==r) { LZ[k].second = -1;}
	else if(l > r) { LZ[k].second = SA[left]; }
	else { LZ[k].second = SA[right];}
        lpf = max<intT>(l, r);
	k++;
	
    }
    
    //for(int i=0;i<k;i++) cout<<"("<<LZ[i].first<<","<<LZ[i].second<<") ";cout<<endl;
    //printf("%d\n", k);
    //for (i = 0;i < k;i++) printf("%d ", LZ[i] + 1);
    delete SA;
    delete leftElements;
    delete rightElements;
    delete Rank;
    lzTm.reportNext("\tlpf");
    return make_pair(LZ, k);
}

int parallel_main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"Seq LZ77 with ANSV", LempelZiv);
}
