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


static int mymem[1 << 28];
static int bm = 0;

inline int * myalloc(int n) {
    int i = bm;
    bm += n;
    if (i+n > (1<<28)) {
        printf("overflow\n");
    }
    return mymem + i;
}

inline void mydealloc(int n) {
    bm -= n;
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
    
    //#pragma omp parallel for
    for (int i = 0; i < d; i++) 
        table[i] = myalloc(n);

    ComputeANSV(sa, n, l, r, table);
    nextTime("ansn");

    buildRMQ(lcp, n, table);
    nextTime("rmq");
    
    #pragma omp parallel for
    for(int i = 0; i < n; i++) {
        int llcp = 0, rlcp = 0;
        if (l[i] != -1) 
            llcp = getLCP(table, l[i], i);
        if (r[i] != -1)
            rlcp = getLCP(table, i, r[i]);
            
        lpf[sa[i]] = max(llcp, rlcp);
    }
    nextTime("lpf");
  
    mydealloc(n);
    mydealloc(n);
    mydealloc(d*sizeof(int*)/sizeof(int));
    mydealloc(n*d);
}

//some optimization require n >= 8
int getLZ(int *lpf, int n, int *lz) {
    int l2 = cflog2(n);
    int depth = l2 + 1;
    int nn = 1 << l2;
    
    //printf("%d %d %d\n", nn, n, depth);
    
    //printf("mem base %d\n", bm);
    int *flag = myalloc(n);
    //int *flag = new int[(max(nn, n + 1))];

    //nextTime("alloc");
    #pragma omp parallel for
    for(int i = 0; i < n; i++) {
        flag[i] = 0;
        lpf[i] = min(n, i + max(lpf[i], 1));
    }
    
    nextTime("prepare"); //combine performance would be better due to cache miss

    //for (int i = 0; i < n; i++) printf("%2d ", lpf[i]);printf("\n");

    int sn = (n + l2 - 1) / l2;
    
    int * next = myalloc(sn+1), *next2 = myalloc(sn+1);
    int * sflag = myalloc(sn+1), *sflag2 = myalloc(sn+1);
    

    //build the sub tree
    #pragma omp parallel for
    for (int i = 0; i < sn; i ++) {
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
        #pragma omp parallel for
        for(int i = 0; i < sn; i ++) {
            int j = next[i];
            if (sflag[i] == 1) {
                // #pragma omp flush(flag)
                sflag[j] = 1;
                //printf("%d ", j);
               //  #pragma omp flush(flag)
               //sflag2[j] = 1;
               //sflag2[i] = 1;
            } 
            // printf("\n");
            next2[i] = next[j];
        }
        std::swap(next, next2);
        //std::swap(sflag, sflag2);
        //        #pragma omp flush(flag)
        //for (int i = 0; i <= sn; i++) printf("%2d ", sflag[i]);printf("\n");

    }

    //omp_set_num_threads(1);
    //filling the result
    #pragma omp parallel for
    for (int i = 0; i < n; i += l2) {
        if (sflag[i / l2]) {
            //printf("adsf");
            flag[i] = 1;
            for(int j = lpf[i]; j % l2 && j != n; j = lpf[j]) {
                flag[j] = 1;
                //printf("write");
            }
        }
    }

    nextTime("point jump");
    
    //for (int i = 0; i <= sn; i++) printf("%2d ", sflag[i]);printf("\n");
    
    //exclusiveScan(flag, nn);
    sequence::scan(flag, flag, n, utils::addF<int>(),0);

    //for (int i = 0; i < n; i++) printf("%2d ", flag[i]);printf("\n");

    nextTime("prefix sum");
    //for (int i = 0; i <= n; i++) printf("%2d ", flag[i]);printf("\n");
    
    int m = flag[n-1];
    //lz = new int[m];
       
    #pragma omp parallel for
    for(int i = 0; i < n; i++) {    
        if (flag[i] < flag[i+1]) {
            lz[flag[i]] = i;
        }
    }
    //nextTime("combine result"); //check a bit about out of boundary
    
    mydealloc(4*(sn+1)); //sflag, next, next2
    
    mydealloc(n);

    return m;
}

pair<int*,int*> suffixArray(int* s, int n, bool findLCPs);

int ParallelLZ77(int *s, int n, int * lz) {
    startTime();

    pair<int *, int*> salcp = suffixArray(s, n, true);
    nextTime("suffix array");

    int *sa = salcp.first;
    //int *lcp = salcp.second - 1;
    //lcp[0] = 0;   //TODO check safety
    int *lcp = new int[n];
    lcp[0] = 0;
    #pragma omp parallel for
    for(int i = 1; i < n; i++) 
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

void checkANSV(int d) {
    int n = 1 << d;
    int *a = new int[n];
    

    for ( int i = 0; i < n; i++)
        a[i] = min(rand() % n, n-1-i);
        
    startTime();
        int *ql = new int[n], *qr = new int[n];
    int **table = new int*[d+1];
        for (int i = 0; i < d+1; i++) table[i] = new int[n];
        
    ComputeANSV(a, n, ql, qr, table);

    nextTime("ansv");   
    
    ANSV an(a, n);
    nextTime("ansv2");
    sequence::scan(a, a, n, utils::addF<int>(),0);
    nextTime("prefix sum");

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

    startTime();
  for (int i = 1; i < n; i++) {
        ql[i] = i + max(1, a[ql[i-1]]);
    }
    nextTime("lz seq");
  
  printf("lz par:\n");
    getLZ(a, n, ql);
    
    printf("\n");
}


inline bool leq(int a1, int a2,   int b1, int b2) { // lexic. order for pairs
  return(a1 < b1 || a1 == b1 && a2 <= b2); 
}                                                   // and triples
inline bool leq(int a1, int a2, int a3,   int b1, int b2, int b3) {
  return(a1 < b1 || a1 == b1 && leq(a2,a3, b2,b3)); 
}
// stably sort a[0..n-1] to b[0..n-1] with keys in 0..K from r
static void radixPass(int* a, int* b, int* r, int n, int K) 
{ // count occurrences
  int* c = new int[K + 1];                          // counter array
  for (int i = 0;  i <= K;  i++) c[i] = 0;         // reset counters
  for (int i = 0;  i < n;  i++) c[r[a[i]]]++;    // count occurences
  for (int i = 0, sum = 0;  i <= K;  i++) { // exclusive prefix sums
     int t = c[i];  c[i] = sum;  sum += t;
  }
  for (int i = 0;  i < n;  i++) b[c[r[a[i]]]++] = a[i];      // sort
  delete [] c;
}

// find the suffix array SA of s[0..n-1] in {1..K}^n
// require s[n]=s[n+1]=s[n+2]=0, n>=2
void suffixArray(int* s, int* SA, int n, int K) {
  int n0=(n+2)/3, n1=(n+1)/3, n2=n/3, n02=n0+n2; 
  int* s12  = new int[n02 + 3];  s12[n02]= s12[n02+1]= s12[n02+2]=0; 
  int* SA12 = new int[n02 + 3]; SA12[n02]=SA12[n02+1]=SA12[n02+2]=0;
  int* s0   = new int[n0];
  int* SA0  = new int[n0];
 
  // generate positions of mod 1 and mod  2 suffixes
  // the "+(n0-n1)" adds a dummy mod 1 suffix if n%3 == 1
  for (int i=0, j=0;  i < n+(n0-n1);  i++) if (i%3 != 0) s12[j++] = i;

  // lsb radix sort the mod 1 and mod 2 triples
  radixPass(s12 , SA12, s+2, n02, K);
  radixPass(SA12, s12 , s+1, n02, K);  
  radixPass(s12 , SA12, s  , n02, K);

  // find lexicographic names of triples
  int name = 0, c0 = -1, c1 = -1, c2 = -1;
  for (int i = 0;  i < n02;  i++) {
    if (s[SA12[i]] != c0 || s[SA12[i]+1] != c1 || s[SA12[i]+2] != c2) { 
      name++;  c0 = s[SA12[i]];  c1 = s[SA12[i]+1];  c2 = s[SA12[i]+2];
    }
    if (SA12[i] % 3 == 1) { s12[SA12[i]/3]      = name; } // left half
    else                  { s12[SA12[i]/3 + n0] = name; } // right half
  }

  // recurse if names are not yet unique
  if (name < n02) {
    suffixArray(s12, SA12, n02, name);
    // store unique names in s12 using the suffix array 
    for (int i = 0;  i < n02;  i++) s12[SA12[i]] = i + 1;
  } else // generate the suffix array of s12 directly
    for (int i = 0;  i < n02;  i++) SA12[s12[i] - 1] = i; 

  // stably sort the mod 0 suffixes from SA12 by their first character
  for (int i=0, j=0;  i < n02;  i++) if (SA12[i] < n0) s0[j++] = 3*SA12[i];
  radixPass(s0, SA0, s, n0, K);

  // merge sorted SA0 suffixes and sorted SA12 suffixes
  for (int p=0,  t=n0-n1,  k=0;  k < n;  k++) {
#define GetI() (SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2)
    int i = GetI(); // pos of current offset 12 suffix
    int j = SA0[p]; // pos of current offset 0  suffix
    if (SA12[t] < n0 ? 
        leq(s[i],       s12[SA12[t] + n0], s[j],       s12[j/3]) :
        leq(s[i],s[i+1],s12[SA12[t]-n0+1], s[j],s[j+1],s12[j/3+n0]))
    { // suffix from SA12 is smaller
      SA[k] = i;  t++;
      if (t == n02) { // done --- only SA0 suffixes left
        for (k++;  p < n0;  p++, k++) SA[k] = SA0[p];
      }
    } else { 
      SA[k] = j;  p++; 
      if (p == n0)  { // done --- only SA12 suffixes left
        for (k++;  t < n02;  t++, k++) SA[k] = GetI(); 
      }
    }  
  } 
  delete [] s12; delete [] SA12; delete [] SA0; delete [] s0; 
}

void calNearestElement(int a[], int n, int leftElements[], int rightElements[]) {
    int i, top;
    int * stack = new int[n];

    for (i = 0, top = -1; i < n; i++) {
        while (top > -1 && a[stack[top]] > a[i]) top--;
        if (top == -1) leftElements[i] = -1;
        else leftElements[i] = stack[top];
        stack[++top] = i;
    }

    for (i = n - 1, top = -1; i >= 0; i--) {
        while (top > -1 && a[stack[top]] > a[i]) top--;
        if (top == -1) rightElements[i] = -1;
        else rightElements[i] = stack[top];
        stack[++top] = i;
    }
    delete stack;
}


int LempelZiv(int *s, int n, int *LZ) {
    int i, lpf, l, r;
    int *leftElements = new int[n], * rightElements = new int[n];
    int *SA = new int[n];
    suffixArray(s, SA, n, 256);
    nextTime("seq suffix array");

    calNearestElement(SA, n, leftElements, rightElements);

    nextTime("seq ANSV");

    int *Rank = new int[n];
    for (i = 0; i < n; i++) {
        Rank[SA[i]] = i;
    }

    int k = 0;
    for (k = i = 0; i < n; i += max(1, lpf)) {      
        int left = leftElements[Rank[i]], right = rightElements[Rank[i]];

        l = r = 0;
    
        if (left != -1) while (s[SA[left] + l] == s[i + l]) 
            l++;
        if (right != -1) while (s[SA[right] + r] == s[i + r])
            r++;

        LZ[k++] = i;
        lpf = max(l, r);
    }
    //printf("%d\n", k);
    //for (i = 0;i < k;i++) printf("%d ", LZ[i] + 1);
    nextTime("seq lpf");
    delete SA;
    delete Rank;
    delete leftElements;
    delete rightElements;
    return k;
}


void checkAll(int np, int d) {
    int n = 1 << d;
    int *a = new int[n+3], *ql = new int[n], *qr = new int[n];
    int i;  
    for ( i = 0; i < n; i++) 
        a[i] = rand() % 4+1;
    a[n] = a[n+1] = a[n+2] = 0;

    int *lz = new int[n];

    printf("sequential\n");
    startTime();
    printf("lz single: %d\n", LempelZiv(a, n, lz));
    reportTime("seq total time");

    for (int p = 1; p <= np; p *= 2) {
        printf("\nNum of procs %d\n", p);
    //nextTime("lz seq");
        omp_set_num_threads(p);

        startTime();
        
        printf("lz par: %d\n", ParallelLZ77(a, n, lz));
        
        reportTime("parallel total time:");
     }
    delete lz;
    printf("\n");
}
int * SuffixArray(int * s, int n) { //s为原串,SA为后缀数组
    //n++;
    int *bucket = new int[n];
    int i, k, t1, t2; //临时交换变量使用
    int *SAk = new int[n], *    Rankk = new int[n], *   SA = new int[n], *Rank = new int[n];
    /////////////////////////////////////////////////////////n//////////////////
    memset(bucket, 0, sizeof(int)*128);

    for (i = 0; i < n; i++) bucket[s[i] + 1]++;     //bucket为字母出现次数的统计
    for (i = 1; i < 128; i++)   bucket[i] += bucket[i - 1]; //按基数进行分段处理,避免了 n*max_size的辅助空间
    for (i = 0; i < n; i++) SA[bucket[s[i]]++] = i;     //收集,把sa2[i]放到sa1中正确的位置

    for (Rank[SA[0]] = 0, i = 1; i < n; i++) {  //构造排名数组
        t1 = SA[i]; t2 = SA[i - 1];
        Rank[t1] = (s[t1] == s[t2]) ? Rank[t2] : Rank[t2] + 1;
    }
    /////////////////////////////////////////////////////////////////////////////////////////
    for (k = 1; k < n - 1 && Rank[SA[n - 1]] < n - 1; k *= 2) { //结束条件2是个很好的剪支条件

        for (i = bucket[0] = 0; i < n; i++) 
bucket[Rank[SA[i]] + 1] = i + 1; //这是一个很好的优化条件,直接就把段也分好了

        for (i = 0; i < n; i++) { //插入SAk中相应位置
            t1 = SA[i];
            if (t1 >= k) SAk[bucket[Rank[t1 - k]]++] = t1 - k; //这些部分找到相应的bucket位置,就插入
            if (t1 >= n - k) SAk[i] = t1; //这些部分是无法移动的,故可直接赋值
        }
        swap(SAk, SA); //SAk与SA互换空间

        for (Rankk[SA[0]] = 0, i = 1; i < n; i++) { //构造排名数组(SA[i] + k的比较构造方法是错误的)
            t1 = SA[i]; t2 = SA[i - 1];
            if (Rank[t1] != Rank[t2] || Rank[t1 + k] != Rank[t2 + k]) Rankk[t1] = Rankk[t2] + 1;
            else Rankk[t1] = Rankk[t2];
        }
        swap(Rankk, Rank);
    } //for k
    return SA;
}


void checkSuffix(int p, int d) {
    int n = 1 << d;
    int *a = new int[n+3];
    int i;  
    for ( i = 0; i < n; i++) 
        a[i] = rand() % 4 + 1;
    a[n] = a[n+1] = a[n+2] = 0;

    int *SA = new int[n];

    //int *SA2 = SuffixArray(a, n);

    //suffixArray(a, SA, n, 2048);
    a[n] = a[n+1] = a[n+2] = 0;

    for (int k = 1; k <= p; k <<= 1) {
            omp_set_num_threads(k);

        printf("num of procs: %d\n", k);
        startTime();
        int  *sa = suffixArray(a, n, false).first;
        delete sa;
        reportTime();
    }

    //for (int i = 0; i< n; i++) printf("%d ", a[i]); printf("\n");
    // for (int i = 0; i < n; i++) {
    //     if (sa[i] != SA[i]) 
    //         printf("sa mismatch %d %d | %d %d\n", i, a[i], sa[i], SA[i]);
    // }
    delete SA;
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
    
    startTime();
    buildRMQ(a, n, table);
    nextTime("rmq1 build");
    #pragma omp parallel for schedule(dynamic, 256)
    for (int i = 0; i < n; i++) {
        queryRMQ(table, ql[i], qr[i]);
        queryRMQ(table, ql[i], qr[i]);
    }
    nextTime("rmq1 query");
    
    startTime();
    myRMQ rq(a, n);
    rq.precomputeQueries();
    nextTime("rmq2 build");

    #pragma omp parallel for schedule(dynamic, 256)
    for (int i = 0; i < n; i++) {
        rq.query(ql[i], qr[i]);
        rq.query(ql[i], qr[i]);
    }
    nextTime("rmq2 query");
    printf("\n");

    //for (int i = 0; i < n; i++) {     printf("%d ", a[i]);    } printf("\n");
    for (int i = 0; i < n; i++) {
        int r1 = queryRMQ(table, ql[i], qr[i]);
        int r2 = a[rq.query(ql[i], qr[i])];
        if (r1 != r2) printf("dismatch [%d %d] %d %d\n",ql[i], qr[i], r1, r2);
        //else printf("match");
    }
    
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
    startTime();
    b[0] = 0;
    for ( i = 1; i < n; i++) 
        b[i] = a[i-1] + b[i-1];
    nextTime("seq  prefix sum");

    sequence::scan(a, c, n, utils::addF<int>(),0);
    nextTime("par prefix sum2");
    
    exclusiveScan(a, n);
    nextTime("par prefix sum1");
    
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
    
    startTime();

    for (int i = 0; i < n; i++) {
        a[i % 128] = cal(i);
    }
    nextTime("seq");
    
    startTime();
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        a[i % 128] = cal(i);
    }
    nextTime("par");
    
    startTime();
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
    nextTime("par2");
    
    startTime();
    for (int i = 0; i < n; i++) {
        a[i % 128] = cal(i);
    }
    nextTime("seq");
    
    delete a;
}

#define MAXN 64

int sa[MAXN] =  {8, 9, 3, 12, 10, 0, 4, 13, 7, 2, 11, 6, 1, 5};
int lcp[MAXN] = {0, 2, 3, 1,  2,  2, 3, 0,  1, 3, 2,  1, 4, 2};

int lpf[MAXN];
//int lz[MAXN];
int *lz = NULL;

void checkCorrect() {

    char *a = (char *)"abbaabbbaaabab";
    int n = strlen(a);
    int *s = new int[n];

    for (int i = 0; i < n; i++) {
        s[i] = a[i];
    }

    int m = ParallelLZ77(s, n, lz);

    // int n = 14;

    // getLPF(sa, n, lcp, lpf);
    // printf("lpf:\n"); for (int i = 0; i < n; i++) printf("%2d ", lpf[i]); printf("\n");  

    // int m = getLZ(lpf, n, lz);

    printf("lz: "); for (int i = 0; i < m; i++) printf("%2d ", lz[i]); printf("\n");
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
//    initlog2();

//  printf("%d %d\n", p, d); 
    //checkSimple(p,d);
    //checkScan(p, d);
    //checkRMQ(d);
    //checkLZ(d);
    //checkANSV(d);
  checkSuffix(p, d);
    //checkCorrect();
  //  checkAll(p, d);
//  for (int i = 0; i < 17; i++) 
//      printf("%d %d\n", (int)log2(i), fflog2(i)); 

    
    return 0;
}




