/*
 * Sequential algorithm from CPM 2011 paper
 */

//Note: not working yet...

#include <iostream>
#include <cstdio>
#include <cstring>
#include "Base.h"
#include "test.h"
#include "suffixArray.h"

using namespace std;

void sop(int i, int l, int j, int *lps, int *prev_occ, int bot){
  if( j == 0 and l ==0 and i==0 )
    return;
  if( lps[i] == bot ){
    lps[i] = l;
    prev_occ[i] = j;
  }else{
    if( lps[i] < l ){
      if( prev_occ[i] > j )
	sop(prev_occ[i], lps[i], j, lps, prev_occ, bot);
      else
	sop(j, lps[i],prev_occ[i], lps, prev_occ, bot);
      lps[i] = l;
      prev_occ[i] = j;
    }else{
      if( prev_occ[i] > j )
	sop(prev_occ[i], l, j, lps, prev_occ, bot);
      else
	sop(j, l, prev_occ[i],lps, prev_occ, bot);			
    }
  }
}

pair<pair<int,int>*,int> LempelZiv(int *s, int n) {
  timer lzTm;
  lzTm.start();
  //n--;
  pair<int *, int *> res = suffixArray(s, n, 0);

  int *sa = res.first;
  lzTm.reportNext("\tsuffix array");

  //for(int i=0;i<n;i++) cout<<sa[i]<<" ";cout<<endl;
  //cout<<"n = "<<n<<endl;
  int *phi = new int [n];
  int *prev_occ = new int [n];
  int *lps = new int[n];
  for(int i=0; i<n; ++i)
    prev_occ[i] = s[i];
  int to_add[2] = {-1,n-1};
  for(int i=0; i<n; ++i){
    phi[sa[i]] = sa[i+to_add[i==0]];
  }
  free(sa);
  //for(int i=0;i<n;i++)cout<<prev_occ[i]<<" ";cout<<endl;
  // sa holds now LPS
  for(int i=0; i<n; ++i)
    lps[i] = -1;	   

  int l = 0;
  for(int i=0; i<n; ++i){ 
    int j = phi[i];
    while( s[i+l] == s[j+l] ) ++l;
   
    if( i>j ){
      sop(i,l,j,lps,prev_occ,-1);
    }else{
      sop(j,l,i,lps,prev_occ,-1);
    }
    if( l > 0 ) --l;	 
  }
  // for(int i=0;i<n;i++)cout<<sa[i]<<" ";cout<<endl;
  // for(int i=0;i<n;i++)cout<<prev_occ[i]<<" ";cout<<endl;

  lzTm.reportNext("\tLPF");

  delete phi;

  // //compute LZ array
  // int* LZ = newA(int,n);
  // LZ[0] = 0;
  // int j = 0;
  // while(LZ[j] < n){
  //   LZ[j+1] = LZ[j] + max(1,lps[LZ[j]]);
  //   j++;
  // }

  //lps[0] = 1; prev_occ[0] = -1;

  // for(int i=0;i<n;i++)cout<<lps[i]<<" ";cout<<endl;
  // for(int i=0;i<n;i++)cout<<prev_occ[i]<<" ";cout<<endl;

  //compute LZ array
  pair<int,int>* LZ = new pair<int,int>[n];
  //int* LZ = newA(int,n);
  
  //Comment: prev_occ array is incorrect, wtf?
  LZ[0].first = 0; LZ[0].second = -1;
  int j = 0;
  while(LZ[j].first < n){
    LZ[j+1].first = LZ[j].first + max(1,lps[LZ[j].first]);
    LZ[j+1].second = prev_occ[LZ[j+1].first];
    j++;
  }


  lzTm.reportNext("\tLZ");
  delete lps; delete prev_occ; 
  return make_pair(LZ, j);
}

int parallel_main(int argc, char *argv[]) {
  return test_main(argc, argv, (char *)"Seq LZ77 CPM2011", LempelZiv);
}
