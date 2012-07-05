/*
 * The parallel algirthm for Lempel-ziv 77 compression
 */

#include <stdio.h>
#include <iostream>
#include <sys/time.h>

#include "ANSV.h"
#include "suffixArray.h"
#include "rangeMin.h"
#include "sequence.h"
#include "Base.h"
#include "segmentTree.h"
#include "test.h"

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

    int p = get_threads();

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

//some optimization require n >= 8
pair<int *, int> getLZ(int *lpf, int n) {
    int l2 = cflog2(n);
    int depth = l2 + 1;
    int nn = 1 << l2;
    
    //printf("%d %d %d\n", nn, n, depth);
    
    //printf("mem base %d\n", bm);
    int *flag = new int[n+1];
    //int *flag = new int[(max(nn, n + 1))];

    //nextTime("alloc");
    parallel_for (int i = 0; i < n; i++) {
        flag[i] = 0;
        lpf[i] = min(n, i + max(lpf[i], 1));
    }
    flag[n] = 0;
    
    nextTime("\tprepare"); //combine performance would be better due to cache miss

    l2 = max(l2, 256);
    int sn = (n + l2 - 1) / l2;
    
    int * next = new int[sn+1], *next2 = new int[sn+1];
    int * sflag = new int[sn+1];
    
    //build the sub tree
    parallel_for (int i = 0; i < sn; i ++) {
        int j;
        for(j = lpf[i*l2]; j % l2 && j != n; j = lpf[j]) ;
        if (j == n) next[i] = sn;
        else next[i] = j / l2;
        sflag[i] = 0;
    }
    next[sn] = next2[sn] = sn; 
    sflag[0] = 1; sflag[sn] = 0;

    //point jump
    int dep = getDepth(sn); ;
    for (int d = 0; d < dep; d++) {
        parallel_for(int i = 0; i < sn; i ++) {
            int j = next[i];
            if (sflag[i] == 1) {
                sflag[j] = 1;
            } 
            // printf("\n");
            next2[i] = next[j];
        }
        std::swap(next, next2);
    }

    //filling the result
    parallel_for (int i = 0; i < n; i += l2) {
        if (sflag[i / l2]) {
            //printf("adsf");
            flag[i] = 1;
            for(int j = lpf[i]; j % l2 && j != n; j = lpf[j]) {
                flag[j] = 1;
            }
        }
    }

    nextTime("\tpoint jump");
        
    sequence::scan(flag, flag, n+1, utils::addF<int>(),0);

    nextTime("\tprefix sum");
    
    int m = flag[n];
    int * lz = new int[m];
    
    parallel_for(int i = 0; i < n; i++) {    
        if (flag[i] < flag[i+1]) {
            lz[flag[i]] = i;
        }
    }
    //nextTime("combine result"); //check a bit about out of boundary
    
    delete flag; delete sflag; delete next; delete next2;

    return make_pair(lz, m);
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

    //for (int i = 0; i < n; i++) {printf("%d ", sa[i]);} puts("");
    //for (int i = 0; i < n; i++) {printf("%d ", lcp[i]);} puts("");

    int *lpf = new int[n];
    // printf("lpf1");
    if (flag == 0)
        getLPF_0(s, sa, n, lcp, lpf);
    // printf("lpf2");
    // getLPF_2(s, sa, n, lcp, lpf);
    // printf("lpf3");
    else if (flag == 1)
        getLPF_1(s, sa, n, lcp, lpf);
    else 
        getLPF_2(s, sa, n, lcp, lpf);

    //getLPF(s, sa, n, lcp, lpf);

    //for (int i = 0; i < n; i++) {printf("%d ", lpf[i]);} puts("");
    delete salcp.first;
    delete salcp.second;

    pair<int *, int> r = getLZ(lpf, n);
    delete lpf;

    return r;
}

int main(int argc, char *argv[]) {
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




