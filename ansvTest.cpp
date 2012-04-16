#include "ansv2.h"
#include <iostream>
#include "cilk.h"
#include "gettime.h"
using namespace std;

int cilk_main(int argc, char* argv[]){
  //srand((unsigned) time(NULL));
  int n = 100;
  if(argc==2) n=atoi(argv[1]);
  //int input[] = {4,2,0,1,3,6,5,8,2,0};
  //int input[] = {2,8,5,6,3,1,0,2,4,9,7};
  //int* a = input;
  int* a = new int[n];
  cilk_for(int i=0;i<n;i++) a[i]=rand()%500;
  cilk_for(int i=0;i<n;i++)a[i]=i;
  startTime();
  ANSV ANSV(a,n);
  int* left = ANSV.getLeftNeighbors();
  int* right = ANSV.getRightNeighbors();
  nextTime("Time for all nearest smaller values");
 // for(int i=0;i<n;i++) cout << left[i] << " " << right[i] << endl;
 // return 0;
  
  int* leftSmaller = new int[n];
  leftSmaller[0]=-1;
  for(int i=1;i<n;i++){
    
    int minValue=a[i-1];
    int minIndex = i-1;
    if(a[i]<=minValue){
      for(int j=i-2;j>=0;j--){
	if(a[j]<minValue){minValue=a[j];minIndex=j;}
	if(a[i]>minValue)break;
      }
      if (a[i]>minValue){leftSmaller[i]=minIndex;}
      else{leftSmaller[i]=-1;}
    }
    else{leftSmaller[i]=minIndex;}
  }
  
  for(int i=0;i<n;i++){
    if(left[i]!=leftSmaller[i]) cout << "incorrect left neighbor at position " << i << endl;
  }
  
  
  int* rightSmaller = new int[n];
  rightSmaller[n-1]=-1;
  for(int i=0;i<n-1;i++){
    
    int minValue=a[i+1];
    int minIndex = i+1;
    if(a[i]<=minValue){
      for(int j=i+2;j<n;j++){
	if(a[j]<minValue){minValue=a[j];minIndex=j;}
	if(a[i]>minValue)break;
      }
      if (a[i]>minValue){rightSmaller[i]=minIndex;}
      else{rightSmaller[i]=-1;}
    }
    else{rightSmaller[i]=minIndex;}
  }
  
  for(int i=0;i<n;i++){
    if(right[i]!=rightSmaller[i]) cout << "incorrect right neighbor at position " << i << endl;
  }
  
}
