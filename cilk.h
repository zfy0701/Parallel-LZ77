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

#if defined(CILK)
#include <cilk.h>
#define cilk_main cilk_main
#define cilk_for_1 _Pragma("cilk_grainsize = 1") cilk_for
#define _cilk_grainsize_1 _Pragma("cilk_grainsize = 1")
#define _cilk_grainsize_2 _Pragma("cilk_grainsize = 2")
#define _cilk_grainsize_256 _Pragma("cilk_grainsize = 256")
#elif defined(CILKP)
#define cilk_main main
#define cilk_for_1 _Pragma("cilk grainsize = 1") cilk_for
#define _cilk_grainsize_1 _Pragma("cilk grainsize = 1")
#define _cilk_grainsize_2 _Pragma("cilk grainsize = 2")
#define _cilk_grainsize_256 _Pragma("cilk grainsize = 256")
#include <cilk/cilk.h>
#elif defined(OPENMP)
#define cilk_spawn _Pragma("omp task")
#define cilk_sync _Pragma("omp taskwait") 
#define cilk_for_1 _Pragma("omp parallel for schedule (static,1)") for
#define cilk_for _Pragma("omp parallel for") for
#define cilk_main main
#define _cilk_grainsize_1 
#define _cilk_grainsize_2 
#define _cilk_grainsize_256
#include <omp.h>
#else
#define cilk_spawn
#define cilk_sync
#define cilk_for_1 for
#define cilk_for for
#define cilk_main main
#define _cilk_grainsize_1 
#define _cilk_grainsize_2 
#define _cilk_grainsize_256 
#endif
 


