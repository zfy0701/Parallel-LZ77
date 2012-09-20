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
 
pair<int *, int> ParallelLPFtoLZ(int *lpf, int n);

void getLPF_0(int *s, int *sa, int n, int *lcp, int *lpf) {
    int d = getDepth(n);
    int *l = new int[n], *r = new int[n];

    nextTime("\tcheckpoint");
    ComputeANSV(sa, n, l, r);
    nextTime("\tansn");

    myRMQ rmq(lcp, n);

//    nextTime("rmq");
    
    parallel_for (int i = 0; i < n; i++) {
        int llcp = 0, rlcp = 0;
        if (l[i] != -1) {
            llcp = lcp[rmq.query(l[i]+1, i)];
        }
        if (r[i] != -1) {
            rlcp = lcp[rmq.query(i+1, r[i])];
        }

        lpf[sa[i]] = max(llcp, rlcp);
    }
    nextTime("\tlpf");
    
    delete l; delete r;
}

void getLPF_1(int *s, int *sa, int n, int *lcp, int *lpf) {
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
        int j = min(i + size, n);
        int mid = rank[i], left = leftElements[rank[i]], right = rightElements[rank[i]];
        if (left != -1) {
            leftLPF[i] = st.Query(left + 1, mid);
        } else leftLPF[i] = 0;

        if (right != -1) {
            rightLPF[i] = st.Query(mid + 1, right);
        } else rightLPF[i] = 0;
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
        }
    }

    parallel_for (int i = 0; i < n; i++) {
        lpf[i] = max(leftLPF[i], rightLPF[i]);
    }

    nextTime("\tlpf");
 
    delete leftElements; delete rightElements;
    delete leftLPF; delete rightLPF;
}

void getLPF_2(int *s, int *sa, int n, int *lcp, int *lpf) {
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
        }
    }

    parallel_for (int i = 0; i < n; i++) {
        lpf[i] = max(leftLPF[i], rightLPF[i]);
    }

    nextTime("\tlpf");
 
    delete leftElements; delete rightElements;
    delete leftLPF; delete rightLPF;
}


int flag = 0;

pair<int *, int> ParallelLZ77(int *s, int n) {
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

    int *lpf = new int[n];
    if (flag == 0)
        getLPF_0(s, sa, n, lcp, lpf);
    else if (flag == 1)
        getLPF_1(s, sa, n, lcp, lpf);
    else 
        getLPF_2(s, sa, n, lcp, lpf);

    //for (int i = 0; i < n; i++) {printf("%d ", lpf[i]);} puts("");
    delete salcp.first;
    delete salcp.second;

    pair<int *, int> r = ParallelLPFtoLZ(lpf, n);
    delete lpf;
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

