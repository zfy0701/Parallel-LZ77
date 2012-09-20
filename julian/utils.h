// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch, Julian Shun and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _BENCH_UTILS_INCLUDED
#define _BENCH_UTILS_INCLUDED
#include <iostream>
#include <algorithm>
#include <string.h>

#if defined(__APPLE__)
#define PTCMPXCH "  cmpxchgl %2,%1\n"
#else
#define PTCMPXCH "  cmpxchgq %2,%1\n"
#include <malloc.h>
static int __ii =  mallopt(M_MMAP_MAX,0);
static int __jj =  mallopt(M_TRIM_THRESHOLD,-1);
#endif

#define newA(__E,__n) (__E*) malloc((__n)*sizeof(__E))

namespace utils {

//Used to parse command line arguments
inline bool getOption(int argc, char** argv, char* option) {
  for (int i = 1; i < argc; i++)
    if (!strcmp(argv[i],option)) return true;
  return false;
}

//Used to parse command line arguments
inline char* getOptionValue(int argc, char** argv, char* option) {
  for (int i = 1; i < argc-1; i++)
    if (!strcmp(argv[i],option)) return argv[i+1];
  return NULL;
}

inline int log2(int i) {
  int a=0;
  int b=i-1;
  while (b > 0) {b = b >> 1; a++;}
  return a;
}

inline int nextPower(int i) {
  int a=0;
  int b=i-1;
  while (b > 0) {b = b >> 1; a++;}
  return (1 << a);
}


inline unsigned int hash(unsigned int a)
{
   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   return a;
}

inline unsigned int hash2(unsigned int a)
{
  return (((unsigned int) 1103515245 * a) + (unsigned int) 12345) %
    (unsigned int) 0xFFFFFFFF;
}


template <class ET>
inline bool CAS(ET **ptr, ET* oldv, ET* newv) {
  unsigned char ret;
  /* Note that sete sets a 'byte' not the word */
  __asm__ __volatile__ (
                "  lock\n"
                "  cmpxchg  %2,%1\n"
		//PTCMPXCH
                "  sete %0\n"
                : "=q" (ret), "=m" (*ptr)
                : "r" (newv), "m" (*ptr), "a" (oldv)
                : "memory");
  return ret;
}

inline bool CAS(long *ptr, long oldv, long newv) {
  unsigned char ret;
  /* Note that sete sets a 'byte' not the word */
  __asm__ __volatile__ (
                "  lock\n"
                "  cmpxchgq %2,%1\n"
                "  sete %0\n"
                : "=q" (ret), "=m" (*ptr)
                : "r" (newv), "m" (*ptr), "a" (oldv)
                : "memory");
  return ret;
}

inline bool CAS(int *ptr, int oldv, int newv) {
  unsigned char ret;
  /* Note that sete sets a 'byte' not the word */
  __asm__ __volatile__ (
                "  lock\n"
                "  cmpxchgl %2,%1\n"
                "  sete %0\n"
                : "=q" (ret), "=m" (*ptr)
                : "r" (newv), "m" (*ptr), "a" (oldv)
                : "memory");
  return ret;
}

inline bool CCAS(int *ptr, int oldv, int newv) {
  unsigned char ret = 0;
  if (*ptr == oldv) {
  /* Note that sete sets a 'byte' not the word */
  __asm__ __volatile__ (
                "  lock\n"
                "  cmpxchgl %2,%1\n"
                "  sete %0\n"
                : "=q" (ret), "=m" (*ptr)
                : "r" (newv), "m" (*ptr), "a" (oldv)
                : "memory");
  }
  return ret;
}

inline bool writeMax(int *a, int b) {
  int c; bool r=0;
  do c = *a; 
  while (c < b && !(r=CAS(a,c,b)));
  return r;
}

inline bool writeMin(int *a, int b) {
  int c; bool r = 0;
  do c = *a; 
  while (c > b && !(r=CAS(a,c,b)));
  return r;
}

inline bool writeMin(long *a, long b) {
  long c; bool r = 0;
  do c = *a; 
  while (c > b && !(r=CAS(a,c,b)));
  return r;
}

static int logUp(unsigned int i) {
  int a=0;
  int b=i-1;
  while (b > 0) {b = b >> 1; a++;}
  return a;
}

static int logUpLong(unsigned long i) {
  int a=0;
  long b=i-1;
  while (b > 0) {b = b >> 1; a++;}
  return a;
}

static void myAssert(int cond, std::string s) {
  if (!cond) {
    std::cout << s << std::endl;
    abort();
  }
}

template <class E>
static void printA(E *A, int n, std::string st, int flag) {
  if (flag) {
    std::cout << st << " ";
    for (int i=0; i < n; i++) std::cout << i << ":" << A[i] << " ";
    std::cout << std::endl;
  }
}

template <class E>
struct identityF { E operator() (const E& x) {return x;}};

template <class E>
struct addF { E operator() (const E& a, const E& b) const {return a+b;}};

template <class E>
struct zeroF { E operator() (const E& a) const {return 0;}};

template <class E>
struct maxF { E operator() (const E& a, const E& b) const {return (a>b) ? a : b;}};

template <class E>
struct minF { E operator() (const E& a, const E& b) const {return (a<b) ? a : b;}};

template <class E1, class E2>
  struct firstF {E1 operator() (std::pair<E1,E2> a) {return a.first;} };

template <class E1, class E2>
  struct secondF {E2 operator() (std::pair<E1,E2> a) {return a.second;} };

}

#endif // _BENCH_UTILS_INCLUDED
