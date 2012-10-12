/*
 * The parallel algirthm for Lempel-ziv 77 compression
 */

#include <cstdio>
#include <iostream>

#include "ANSV.h"
#include "suffixArray.h"
#include "rangeMin.h"
#include "sequence.h"
#include "Base.h"
#include "segmentTree.h"
#include "test.h"
#include "parallel.h"
 
pair< pair<int,int> *, int> ParallelLPFtoLZ(int *lpf, int* prev_occ, int n);

void getLPF_0(int *s, int *sa, int n, int *lcp, int *lpf, int *prev_occ) {
    int d = getDepth(n);
    int *l = new int[n], *r = new int[n];

    nextTime("\tcheckpoint");

    ComputeANSV(sa, n, l, r);
    nextTime("\tansn");

    myRMQ rmq(lcp, n);
    //for(int i=0;i<n;i++)cout<<sa[i]<<" ";cout<<endl;

//    nextTime("rmq");
    //for(int i=0;i<n;i++)cout<<i<<" "<<l[i]<<" "<<r[i]<<endl;
    parallel_for (int i = 0; i < n; i++) {
      int llcp = 0, rlcp = 0;
      int ln = l[i], rn = r[i];
      int sai = sa[i];
        if (ln != -1) {
            llcp = lcp[rmq.query(ln+1, i)];
	    
        }
        if (rn != -1) {
            rlcp = lcp[rmq.query(i+1, rn)];
        }
        //lpf[sai] = max(llcp, rlcp);
	//cout<<i<<" "<<sa[i]<<" "<<ln<<" "<<rn<<" "<<llcp<<" "<<rlcp<<" "<<sa[ln]<<" "<<sa[rn]<<endl;

	if ((ln == -1 && -1 == rn) || !(llcp || rlcp)) {prev_occ[sai] = -1; lpf[sai] = 1;}
	// no neighbor
	else if(llcp > rlcp) {prev_occ[sai] = sa[ln]; lpf[sai] = llcp;}
	else {prev_occ[sai] = sa[rn];lpf[sai] = rlcp;}
    }
    nextTime("\tlpf");
    //for(int i=0;i<100;i++)cout<<prev_occ[i]<<" ";cout<<endl;

    delete l; delete r;
}

void getLPF_1(int *s, int *sa, int n, int *lcp, int *lpf, int* prev_occ) {
    int d = getDepth(n);
    int *leftElements = new int[n], *rightElements = new int[n];

    int *leftLPF = new int[n], *rightLPF = new int[n];
    int *rank = lpf; //reuse the space

    nextTime("\tcheckpoint");
    ComputeANSV(sa, n, leftElements, rightElements);
    nextTime("\tansn");
    
    SegmentTree st;
    st.BuildTree(lcp, n);
    nextTime("\tbuild tree");

    parallel_for (int i = 0; i < n; i++) {
        rank[sa[i]] = i;
    }

    
    int size = 8196;

    //compute lpf for first element
    parallel_for (int i = 0; i < n; i += size) {
      //int j = min(i + size, n);
        int mid = rank[i], left = leftElements[rank[i]], right = rightElements[rank[i]];
        if (left != -1) {
            leftLPF[i] = st.Query(left + 1, mid);
        } else leftLPF[i] = 0;

        if (right != -1) {
            rightLPF[i] = st.Query(mid + 1, right);
        } else rightLPF[i] = 0;

	if (left == -1 && -1 == right) {prev_occ[i] = -1; lpf[i] = 1;}
	// no neighbor
	else if(leftLPF[i] > rightLPF[i]) {
	  prev_occ[i] = sa[left]; lpf[i] = leftLPF[i];}
	else {prev_occ[i] = sa[right];lpf[i] = rightLPF[i];}

    }
    st.DeleteTree();

    //compute lpf for rest elements    
    parallel_for (int i = 0; i < n; i += size) {    
        int j = min(i + size, n);  
        for (int k = i + 1; k < j; k++) {
            int left = leftElements[rank[k]];
            int right = rightElements[rank[k]];

            if (left != -1) {
                int llcp = max(leftLPF[k - 1] - 1, 0);
                while (s[sa[left] + llcp] == s[k + llcp]) llcp++;
                leftLPF[k] = llcp;
            } else leftLPF[k] = 0;

            if (right != -1) {
                int rlcp = max(rightLPF[k - 1] - 1, 0);
                while (s[sa[right] + rlcp] == s[k + rlcp]) rlcp++;
                rightLPF[k] = rlcp;
            } else rightLPF[k] = 0;


	    if (left == -1 && -1 == right) {prev_occ[k] = -1; lpf[k] = 1;}
	    // no neighbor
	    else if(leftLPF[k] > rightLPF[k]) {
	      prev_occ[k] = sa[left]; lpf[k] = leftLPF[k];}
	    else {prev_occ[k] = sa[right];lpf[k] = rightLPF[k];}

        }
    }

    // parallel_for (int i = 0; i < n; i++) {
    //   int left = leftElementsRanked[i];
    //   int right = rightElementsRanked[i];
    //     lpf[i] = max(leftLPF[i], rightLPF[i]);
    // }

    nextTime("\tlpf");
 
    delete leftElements; delete rightElements;
    delete leftLPF; delete rightLPF;
}

void getLPF_2(int *s, int *sa, int n, int *lcp, int *lpf, int* prev_occ) {
    int d = getDepth(n);
    int *leftElements = new int[n], *rightElements = new int[n];

    int *leftLPF = new int[n], *rightLPF = new int[n];
    int *rank = lpf;

    nextTime("\tcheckpoint");
    ComputeANSV(sa, n, leftElements, rightElements);
    nextTime("\tansn");

    parallel_for (int i = 0; i < n; i++) {
        rank[sa[i]] = i;
    }

    // lcp = GetLCP(s,  n, sa);
    // nextTime("\tlinear lcp test");
    
    //int p = get_threads();
#if defined(CILKP)
    int p = get_threads();
#else
    int p = 8;
#endif
    p *= 2;
    int size = (n + p - 1) / p;
    //int size = 8196;

    parallel_for (int i = 0; i < n; i += size) {
        int j = min(i + size, n);

        //compute lpf for first element
        int mid = rank[i], left = leftElements[rank[i]], right = rightElements[rank[i]];
        int llcp = 0, rlcp = 0;

        if (left != -1) {
            while (s[sa[left] + llcp] == s[i + llcp]) llcp++;
            leftLPF[i] = llcp;
        } else leftLPF[i] = 0;

        if (right != -1) {
             while (s[sa[right] + rlcp] == s[i + rlcp]) rlcp++;
            rightLPF[i] = rlcp;
        } else rightLPF[i] = 0;

	if (left == -1 && -1 == right) {prev_occ[i] = -1; lpf[i] = 1;}
	// no neighbor
	else if(leftLPF[i] > rightLPF[i]) {
	  prev_occ[i] = sa[left]; lpf[i] = leftLPF[i];}
	else {prev_occ[i] = sa[right];lpf[i] = rightLPF[i];}


        //compute lpf for rest elements
        for (int k = i + 1; k < j; k++) {
            left = leftElements[rank[k]];
            right = rightElements[rank[k]];

            if (left != -1) {
                llcp = max(leftLPF[k - 1] - 1, 0);
                while (s[sa[left] + llcp] == s[k + llcp]) llcp++;
                leftLPF[k] = llcp;
            } else leftLPF[k] = 0;

            if (right != -1) {
                rlcp = max(rightLPF[k - 1] - 1, 0);
                while (s[sa[right] + rlcp] == s[k + rlcp]) rlcp++;
                rightLPF[k] = rlcp;
            } else rightLPF[k] = 0;

	    if (left == -1 && -1 == right) {prev_occ[k] = -1; lpf[k] = 1;}
	    // no neighbor
	    else if(leftLPF[k] > rightLPF[k]) {
	      prev_occ[k] = sa[left]; lpf[k] = leftLPF[k];}
	    else {prev_occ[k] = sa[right];lpf[k] = rightLPF[k];}

        }
    }

    // parallel_for (int i = 0; i < n; i++) {
    //     lpf[i] = max(leftLPF[i], rightLPF[i]);
    // }

    nextTime("\tlpf");
 
    delete leftElements; delete rightElements;
    delete leftLPF; delete rightLPF;
}


int flag = 0;

pair<pair<int, int>*,int> ParallelLZ77(int *s, int n) {
    startTime();

    pair<int *, int*> salcp = suffixArray(s, n, flag < 2 ? true : false);
    nextTime("\tsuffix array");

    int *sa = salcp.first;
    int *lcp = new int[n];
    if (flag < 2) {
        lcp[0] = 0;
        parallel_for (int i = 1; i < n; i++) 
            lcp[i] = salcp.second[i-1];
    }
    int* prev_occ = new int[n];
    //parallel_for(int i=0;i<n;i++) prev_occ[i] = -1;
    int *lpf = new int[n];
    if (flag == 0)
      getLPF_0(s, sa, n, lcp, lpf,prev_occ);
    else if (flag == 1)
      getLPF_1(s, sa, n, lcp, lpf,prev_occ);
    else 
      getLPF_2(s, sa, n, lcp, lpf,prev_occ);

    //for (int i = 0; i < n; i++) {printf("%d ", lpf[i]);} puts("");
    delete salcp.first;
    delete salcp.second;
    //for(int i=0;i<10;i++)cout<<lpf[i]<<" ";cout<<endl;
    // for(int i=0;i<100;i++)cout<<prev_occ[i]<<" ";cout<<endl;
    
    pair< pair<int, int>*, int> r = ParallelLPFtoLZ(lpf, prev_occ, n);
    nextTime("\tlpf to lz");
    delete lpf; delete prev_occ;
    return r;
}

int parallel_main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "p:d:r:i:o:f:")) != -1) {
        if (opt == 'f') {
            flag = atoi(optarg);
            break;
        }
    }

    optind = 1;

    return test_main(argc, argv, (char *)"Parallel LZ77 using suffix array", ParallelLZ77);
}

