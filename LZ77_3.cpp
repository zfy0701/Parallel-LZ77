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

void sop(int i, int l, int j, int* LPS, int* PrevOcc){
  if(LPS[i] == -1){
    LPS[i] = l;
    PrevOcc[i] = j;
  } else {
    if(LPS[i] < l){
      if(PrevOcc[i] > j)
	sop(PrevOcc[i],LPS[i],j,LPS,PrevOcc);
      else
	sop(j,LPS[i],PrevOcc[i],LPS,PrevOcc);
      LPS[i] = l;
      PrevOcc[i] = j;
    } else {
      if(PrevOcc[i] > j)
	sop(PrevOcc[i],l,j,LPS,PrevOcc);
      else
	sop(j,l,PrevOcc[i],LPS,PrevOcc);
    } 
  }
}

pair<int*,int> LempelZiv(int *s, int n) {
    timer lzTm;
    lzTm.start();

    pair<int *, int *> res = suffixArray(s, n, 0);

    int *SA = res.first;
    lzTm.reportNext("\tsuffix array");

    for(int i=0;i<n;i++) cout<<SA[i]<<" ";cout<<endl;
    
    // //begin bullshit
    // int* LPS = new int[n];
    // int* PrevOcc = new int[n];
    // for(int i=0;i<n;i++) LPS[i] = PrevOcc[i] = -1;
    // int* Phi = new int[n];

    // Phi[SA[0]] = 0;
    // for(int i=1;i<n;i++) Phi[SA[i]] = SA[i-1];
    // int l = 0;
    // for(int i=0;i<n;i++){
    //   int j = Phi[i];
    //   while(s[i+l] == s[j+l]) l++;
    //   if(i >= j) sop(i,l,j,LPS,PrevOcc);
    //   else sop(j,l,i,LPS,PrevOcc);
    //   l = max(l-1,0);
    // }
    // //end bullshit
    // for(int i=0;i<n;i++) cout<<LPS[SA[i]]<<" ";cout<<endl;
    // for(int i=0;i<n;i++) cout<<PrevOcc[SA[i]]<<" ";cout<<endl;
    
    int san = n+1;
    int *phi = new int [san];
    int *prev_occ = new int [san];
    //for(int i=0; i<n; ++i)
    //prev_occ[i] = s[i];
    int to_add[2] = {-1,san-1};
    phi[SA[0]] = n;

    for(int i=1; i<n; ++i){      
      phi[SA[i]] = SA[i-1];
    }
    phi[n] = SA[n-1];
    for(int i=0;i<san;i++)cout<<phi[i]<<" ";cout<<endl;
    int *LPS = new int[n];
    // sa holds now LPS
    for(int i=0; i<san; ++i)
      LPS[i] = n;	   

    int l = 0;
    for(int i=0; i<n; ++i){
 
      int j = phi[i];
      while( s[i+l] == s[j+l] ) ++l;
      int ii = 0, jj = 0;
      if( i > j){
	ii = i; jj = j;
      }else{
	ii = j; jj = i;
      }
      int ll = l;
      while( LPS[ii] != n ){
	int iii = (prev_occ[ii] > jj) ? prev_occ[ii] : jj;
	int jjj = (prev_occ[ii] > jj) ? jj : prev_occ[ii];
	if( LPS[ii] < ll ){
	  int lll = LPS[ii];
	  LPS[ii] = lll;
	  prev_occ[ii] = jj;
	  ll = lll;
	}
	ii = iii;
	jj = jjj;
      }
      LPS[ii] = ll; prev_occ[ii] = jj;
      if( l > 0 ) --l;	 
    }

    for(int i=0;i<n;i++) cout<<LPS[SA[i]]<<" ";cout<<endl;
    for(int i=0;i<n;i++) cout<<prev_occ[SA[i]]<<" ";cout<<endl;



    int k  = 0;
    int * LZ = new int[n];
    return make_pair(LZ, k);
}

int main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"Seq LZ77 CPM2011", LempelZiv);
}
