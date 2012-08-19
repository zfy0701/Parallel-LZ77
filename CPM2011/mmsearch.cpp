
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "mmsearch.h"
#include "RMQ_succinct.hpp"

static unsigned char *x;
static int *SA;
static int n;

int firstPosInSA[256];
int lastPosInSA[256];
int leftMostValue[256]; //the position in x of the first occ of c
int leftMostRank[256]; //the position in SA of the first occ of c

void computeLZFactorAt(int i, int *pos, int *len);
inline void refineRange(int lo, int hi, int offset, unsigned char c, int *nlo, int *nhi);
inline int binarySearchLB(int lo, int hi, int offset, unsigned char c);
inline int binarySearchRB(int lo, int hi, int offset, unsigned char c);
inline int rmqmanual(int i, int j);

int rmqsAvoided = 0;
int renormalizations = 0;
float averageRMQLength = 0;
float averageFactorLength = 0;
float maxFactorLength = 0;
int longFactors = 0;
int shortRMQs = 0;
int longRMQs = 0;

RMQ_succinct *RMQ;

int symtobin[4] = {0x3,0x0,0x1,0x2};

typedef struct _Interval{
   int lb;
   int rb;
   int min;
   int imin;
}Interval;

#define GRAM 1

Interval intervals[GRAM][65536];
int bits;

//#define ACGT

#ifndef ACGT
int symcodetab[256];
inline int symtocode(int sym){
   return symcodetab[sym]; //errr...
   //return sym;
}
#endif

#ifdef ACGT
inline int symtocode(int sym){
   int code = 0x0; //assume sym is an 'A'
   switch(sym & ~0x20){
      case 'C': code = 0x1; break;
      case 'G': code = 0x2; break;
      case 'T': code = 0x3; break;
   }
   return code;
}
#endif

//Map A,C,G,T to 0,1,2,3 without branching! Never actually used.
inline int symtocode2(int sym){
  return (((sym+sym+sym) & 0x18) >> 3);
}

void lzFactorize(unsigned char *ax, int *sa, int an){
   x = ax;
   n = an;
   SA = sa;

   bits = 1;
   int alpha = 4;
#ifndef ACGT
//If we're not working with DNA, compute a minimal alphabet mapping
   for(int i=0; i<256; i++)
      symcodetab[i] = 0;
   for(int i=0; i<n; i++)
      symcodetab[x[i]] = 1;
   alpha = 0;
   for(int i=0; i<256; i++)
      if(symcodetab[i])
         symcodetab[i] = alpha++;
   printf("# alpha = %d; logbase2(%d) = %d\n",alpha,alpha,logbase2(alpha));
   alpha = (1 << (logbase2(alpha-1)+1));
   bits = logbase2(alpha);
   printf("# bits = %d\n",bits);
#endif

   //compute a fast lookup table for prefixes of length \le 8
   double tintervals = getTime();
   for(int i=0,s=1;i<GRAM;i++){
      s *= alpha;
      for(int j=0;j<s;j++){
         intervals[i][j].lb = -1;
         //intervals[i][j].rb = -1; //don't need to initialize
         intervals[i][j].min = n;
         //intervals[i][j].imin = n; //don't need to initialize
      }
   }

   //The way the intervals are cirrently computed is quite
   //inefficient -- really you sould only need to make one
   //scan of the string, x, not a scan of the SA and random
   //jumps into x at each step as is currently the case.
   for(int i=0;i<n;i++){
      int ccode = 0;
      int suf = SA[i];
      for(int j=0;j<GRAM && suf+j < n;j++){
         //unsigned char sym = (x[suf+j] & ~0x20); //get the symbol and make it a capital
         int code = symtocode(x[suf+j]);
        #ifdef ACGT
         ccode |= (code << (j<<1));
        #endif
        #ifndef ACGT
         ccode |= (code << (j*bits));
        #endif
         Interval *interval = &intervals[j][ccode];
         if(j != GRAM-1){
            if(interval->min > suf)
               interval->min = suf;
         }else{
            if(interval->lb == -1)
               interval->lb = i;
            interval->rb = i;
            if(interval->min > suf){
               interval->min = suf;
               interval->imin = i;
            }
         }
      }
   }

   float avg8groupLength = 0;
   int max8groupLength = 0;
   float num8groups = 0;
   for(int i=0;i<65536;i++){
      if(intervals[GRAM-1][i].lb != -1){
         int gl = (intervals[GRAM-1][i].rb - intervals[GRAM-1][i].lb + 1);
         if(max8groupLength < gl)
            max8groupLength = gl;
         if(gl > 1){
            avg8groupLength += gl;
            num8groups++;
         }
      }
   }
   printf("# Total %d-group Length = %.2f; Number of %d-groups = %.2f\n",GRAM,avg8groupLength,GRAM,num8groups); 
   avg8groupLength /= num8groups;
   printf("# Average %d-group Length = %.2f\n",GRAM,avg8groupLength);

   printf("# Time to compute intervals = %.2f\n",getTime()-tintervals);

   double trmq = getTime();
   RMQ = new RMQ_succinct(SA,n);
   printf("# \n#Time to initialize RMQ = %.2f\n",getTime()-trmq);

   int i = 1;
   int numfactors = 1;
   while(i < n){
      int pos, len;
      computeLZFactorAt(i, &pos, &len);
      numfactors++;
      //printf("\t***LZ Factor: i = %d, pos = %d, len = %d\n",i,pos,len);
      if(len <= 0)
         len = 1;
      i = i + len;
      averageFactorLength += len;
      if(maxFactorLength < len)
         maxFactorLength = len;
   }
   printf("numfactors = %d\n",numfactors);
   averageFactorLength /= numfactors;
   printf("averageFactorLength = %.2f\n",averageFactorLength);
   printf("maxFactorLength = %.2f\n",maxFactorLength);
   printf("longFactors (len>8) = %d\n",longFactors);
   printf("# renormalizations = %d; rmqsavoided = %d\n",renormalizations,rmqsAvoided);
   averageRMQLength /= renormalizations;
   printf("# totalRMQs = %d; averageRMQLength = %.2f\n",renormalizations-rmqsAvoided,averageRMQLength);
   printf("# shortRMQs = %d; longRMQs = %d\n",shortRMQs,longRMQs);
}

void computeLZFactorAt(int i, int *pos, int *len){
   int offset = 0;
   int j = i;
   int ccode = 0;
   int lb=0,rb=0;
   int min,imin;
   int lcpl=0,lcpr=0;
   *pos = i;
   *len = 0;
   for(;j<i+GRAM && j<n; j++){
      int code = symtocode(x[j]);
     #ifdef ACGT
      ccode |= (code << ((j-i)<<1));
     #endif
     #ifndef ACGT
      ccode |= (code << (j-i)*bits);
     #endif
      Interval interval = intervals[j-i][ccode];
      if(interval.min < i){
         *pos = min = interval.min;
         imin = interval.imin;
         *len = offset = (j-i)+1;
         lb = interval.lb;
         rb = interval.rb;
      }else{
         //we're done
         return;
      }
   }
   //if we get here then the lz-factor at i will have len > 8
   longFactors++;
   bool matchToLeft = true;
   int match = min;
   j--;
   offset--;
   while(matchToLeft && j < n){
      j++;
      offset++;
      unsigned char c = x[j];
      renormalizations++;
      int nlb, nrb;
      nlb = binarySearchLB(lb,rb,offset,x[j]);
      nrb = binarySearchRB(lb,rb,offset,x[j]);
      //refineRange(lb,rb,offset,x[j],&nlb,&nrb);

      if(nrb == nlb){ //we are matching ourselves, the game's up
         break;
      }
      if(nlb <= imin && imin <= nrb){
         //the previous minimum is still in this interval so 
         //don't recompute the minimum, just narrow the interval
         lb = nlb; rb = nrb;
         rmqsAvoided++;
      }
      else{
         //compute the range min and it's index
         averageRMQLength += (nrb-nlb+1);
         if(nrb-nlb < 100){
            shortRMQs++;
            min = SA[nlb];
            imin = nlb;
            for(int k=nlb+1; k<=nrb; k++){
               if(SA[k] < min){
                  min = SA[k];
                  imin = k;
               }
            }
         }else{
            longRMQs++;
            imin = RMQ->query(nlb,nrb);
            min = SA[imin];
         }
         //if the new range min < i, narrow the interval
         if(min < i){
            match = min;
            lb = nlb; rb = nrb;
         }else{
            matchToLeft = false;
         }
      }
   }
   *pos = match;
   *len = offset;
}

inline int rmqmanual(int i, int j){
   int min = SA[i];
   for(int k=i+1;k<=j;k++)
      if(SA[k] < min)
         min = SA[k];
   return min;
}

//This method isn't really any faster than calling binarySearchLB followed by binarySearchRB
inline void refineRange(int lo, int hi, int offset, unsigned char c, int *nlo, int *nhi){
   int low = lo, high = hi, rhi = hi, rlo = lo;
   while (low <= high){
      int mid = (low + high) >> 1;
      unsigned char midVal = x[SA[mid]+offset];
      if (midVal < c)
         low = mid + 1;
      else if (midVal > c)
         high = rhi = mid - 1;
      else{ //midVal == c
         rlo = mid;
         if(mid == lo){
            *nlo = mid; // leftmost occ of key found
            break;
         }
         unsigned char  midValLeft = x[SA[mid-1]+offset];
         if(midValLeft == midVal)
            high = mid - 1; //discard mid and the ones to the right of mid
         else{ //midValLeft must be less than midVal == c
            *nlo = mid; //leftmost occ of key found
            break;
         }
      }
   }
   low = rlo, high = rhi;
   while (low <= high){
      int mid = (low + high) >> 1;
      unsigned char midVal = x[SA[mid]+offset];
      if (midVal < c)
         low = mid + 1;
      else if (midVal > c)
         high = mid - 1;
      else{ //midVal == c
         if(mid == hi){
            *nhi = mid; // rightmost occ of key found
            return;
         }
         unsigned char  midValRight = x[SA[mid+1]+offset];
         if(midValRight == midVal)
            low = mid + 1; //discard mid and the ones to the left of mid
         else{ //midValRight must be greater than midVal == c
            *nhi = mid; //rightmost occ of key found
            return;
         }
      }
   }
}

inline int binarySearchLB(int lo, int hi, int offset, unsigned char c){
   int low = lo, high = hi;
   while (low <= high){
      int mid = (low + high) >> 1;
      unsigned char midVal = x[SA[mid]+offset];
      if (midVal < c)
         low = mid + 1;
      else if (midVal > c)
         high = mid - 1;
      else{ //midVal == c
         if(mid == lo)
            return mid; // leftmost occ of key found
         unsigned char  midValLeft = x[SA[mid-1]+offset];
         if(midValLeft == midVal){
            high = mid - 1; //discard mid and the ones to the right of mid
         }else{ //midValLeft must be less than midVal == c
            return mid; //leftmost occ of key found
         }
      } 
   }
   return -(low + 1);  // key not found.
}

inline int binarySearchRB(int lo, int hi, int offset, unsigned char c){
   int low = lo, high = hi;
   while (low <= high){
      int mid = (low + high) >> 1;
      unsigned char midVal = x[SA[mid]+offset];
      if (midVal < c)
         low = mid + 1;
      else if (midVal > c)
         high = mid - 1;
      else{ //midVal == c
         if(mid == hi)
            return mid; // rightmost occ of key found
         unsigned char  midValRight = x[SA[mid+1]+offset];
         if(midValRight == midVal){
            low = mid + 1; //discard mid and the ones to the left of mid
         }else{ //midValRight must be greater than midVal == c
            return mid; //rightmost occ of key found
         }
      }
   }
   return -(low + 1);  // key not found.
}

