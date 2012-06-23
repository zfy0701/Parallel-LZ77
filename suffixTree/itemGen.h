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

#ifndef _ITEMGEN_INCLUDED
#define _ITEMGEN_INCLUDED

#include <iostream>
#include <algorithm>
#include "cilk.h"
#include "utils.h"

namespace dataGen {

  using namespace std;

  enum dtype { none, intT, doubleT, stringT};

 dtype paramsGetType(int argc, char* argv[]) {
  dtype dt = intT;
  for (int i = 1; i < argc; i++)
    if ((string) argv[i] == "-t") {
      if ((string) argv[i+1] == "double") dt = doubleT;
      else if ((string) argv[i+1] == "string") dt = stringT;
      else if ((string) argv[i+1] == "int") dt = intT;
    }
  return dt;
}

pair<char*,char*> paramsGetIOFileNames(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Too few arguments: needs at least an in and out file" << endl;
    abort();
  }    
  return pair<char*,char*>(argv[argc-2],argv[argc-1]);
}

 pair<int,char*> paramsGetSizeAndFileName(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Too few arguments: needs at least size and filename" << endl;
    abort();
  }    
  return pair<int,char*>(std::atoi(argv[argc-2]),(char*) argv[argc-1]);
}

 bool paramsGetOption(int argc, char* argv[], string option) {
  for (int i = 1; i < argc; i++)
    if ((string) argv[i] == option) return true;
  return false;
 }

 bool paramsGetBinary(int argc, char* argv[]) {
   return paramsGetOption(argc,argv,"-b");
 }

#define HASH_MAX_INT (1 << 30)

  template <class T> T hash(int i);
  
  template <>
  int hash<int>(int i) {
    return utils::hash(i) & (HASH_MAX_INT-1);}

  template <>
  unsigned int hash<unsigned int>(int i) {
    return utils::hash(i);}

  template <>
  double hash<double>(int i) {
    return ((double) hash<int>(i)/((double) HASH_MAX_INT));}

};

#endif // _ITEMGEN_INCLUDED
