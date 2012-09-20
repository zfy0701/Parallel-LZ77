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

#include "Base.h"

#if defined(CILK)
//#include <cilk/cilk.h>
//#include <cilk/cilk_api.h>
#define parallel_main cilk_main
#define parallel_for cilk_for
#define parallel_for_1 _Pragma("cilk_grainsize = 1") cilk_for
#define parallel_grainsize_1 _Pragma("cilk_grainsize = 1")
#define parallel_grainsize_2 _Pragma("cilk_grainsize = 2")
#define parallel_grainsize_256 _Pragma("cilk_grainsize = 256")
#define parallel_spawn cilk_spawn
#define parallel_sync cilk_sync
//#define set_threads(p) __cilkrts_set_param("nworkers", itoa(p))
//#define get_threads() __cilkrts_get_nworkers()

#elif defined(CILKP)
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#define parallel_main main
#define parallel_for cilk_for
#define parallel_for_1 _Pragma("cilk grainsize = 1") cilk_for
#define parallel_grainsize_1 _Pragma("cilk grainsize = 1")
#define parallel_grainsize_2 _Pragma("cilk grainsize = 2")
#define parallel_grainsize_256 _Pragma("cilk grainsize = 256")
#define parallel_spawn cilk_spawn
#define parallel_sync cilk_sync
#define set_threads(p) __cilkrts_set_param("nworkers", itoa(p))
#define get_threads() __cilkrts_get_nworkers()

#elif defined(OPENMP)
#include <omp.h>
#define parallel_main main
#define parallel_for _Pragma("omp parallel for") for
#define parallel_for_1 _Pragma("omp parallel for schedule (static,1)") for
#define parallel_grainsize_1 
#define parallel_grainsize_2 
#define parallel_grainsize_256
#define parallel_spawn _Pragma("omp task")
#define parallel_sync _Pragma("omp taskwait") 
#define set_threads(p) omp_set_num_threads(p)
#define get_threads() omp_get_max_threads()

#else
#define parallel_spawn
#define parallel_sync
#define parallel_for_1 for
#define parallel_for for
#define parallel_main main
#define parallel_grainsize_1 
#define parallel_grainsize_2 
#define parallel_grainsize_256 
#define set_threads(p) 
#define get_threads()  1
#endif
 


#include <limits.h>

#if defined(LONG)
typedef long intT;
typedef unsigned long uintT;
#define INT_T_MAX LONG_MAX
#else
typedef int intT;
typedef unsigned int uintT;
#define INT_T_MAX INT_MAX
#endif
