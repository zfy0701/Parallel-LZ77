#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>
#include "utils.h"
#include "divsufsort.h"

void sop(int i, int l, int j, int *lps, int *prev_occ, int bot){
	if( j == 0 and l ==0 and i==0 )
		return;
	assert(i>j);
	assert(i!=j);
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

int main(int argc, char **argv){
   if(argc < 2){
      fprintf(stderr,"usage: %s {inputfile} [printlz]\n",argv[0]);
      exit(1);
   }
   bool printlz = false;
   if(argc == 3){
     if( strcmp(argv[2],"printlz")==0 ) 
		printlz = true; 
   }

   //read input string
   FILE *infile = fopen(argv[1],"r");
   if(!infile){
      fprintf(stderr,"Error opening input file %s\n",argv[1]);
      exit(1);
   }
   int n = 0;
   fseek(infile,0,SEEK_END);
   n = ftell(infile);
   fseek(infile,0,SEEK_SET);
   unsigned char *x = new unsigned char[n+1];
   if(n != fread(x,sizeof(unsigned char),n,infile)){
      fprintf(stderr,"Error reading string from file\n");
      exit(1);
   }
   x[n] = 0;
   fclose(infile);   

   int * sa = new int[n];
   divsufsort(x, sa, n);
   //for(int i=n-100;i<n;i++)printf("%d ",sa[i]);printf("\n");
/*
   //read the suffix array
   char safilename[256];
   sprintf(safilename,"%s.sa",argv[1]);
   infile = fopen(safilename,"r");
   if(!infile){
      fprintf(stderr,"Error opening input file %s\n",safilename);
      exit(1);
   }
   int san = 0;
   fseek(infile,0,SEEK_END);
   san = ftell(infile) / sizeof(int);
   fseek(infile,0,SEEK_SET);
   int *sa = new int[san];
   if(san != fread(sa,sizeof(int),san,infile)){
      fprintf(stderr,"Error reading sa from file\n");
      exit(1);
   }
   fclose(infile);
*/

   //compute LZ factorization
   double tlz = getTime();
   // compute PHI
   int *phi = new int [n];
   int *prev_occ = new int [n];
   for(int i=0; i<n; ++i)
	   prev_occ[i] = x[i];
   int to_add[2] = {-1,n-1};
   for(int i=0; i<n; ++i){
   	 phi[sa[i]] = sa[i+to_add[i==0]];
   }
   //for(int i=0;i<100;i++)printf("%d ",prev_occ[i]);printf("\n");
   fprintf(stderr,"# Time to calc phi = %.2f secs\n",getTime()-tlz);
   double tlz2 = getTime();
   // sa holds now LPS
   for(int i=0; i<n; ++i)
     sa[i] = -1;	   

   int maxFactorLength = 0;
   int l = 0;
   for(int i=0; i<n; ++i){ 
     int j = phi[i];
	 while( x[i+l] == x[j+l] ) ++l;
	 
	 if( i>j ){
	 	sop(i,l,j,sa,prev_occ,-1);
	 }else{
	 	sop(j,l,i,sa,prev_occ,-1);
	 }
/*	 
	 int ii = 0, jj = 0;
	 if( i > j){
	 	ii = i; jj = j;
	 }else{
	 	ii = j; jj = i;
	 }
	 int ll = l;
	 while( sa[ii] != n ){
		int iii = (prev_occ[ii] > jj) ? prev_occ[ii] : jj;
		int jjj = (prev_occ[ii] > jj) ? jj : prev_occ[ii];
		if( sa[ii] < ll ){
			int lll = sa[ii];
			sa[ii] = lll;
			prev_occ[ii] = jj;
			ll = lll;
		}
		ii = iii;
		jj = jjj;
	 }
	 sa[ii] = ll; prev_occ[ii] = jj;
*/	 
//	 if( l > maxFactorLength) 
//		 maxFactorLength = l;
     if( l > 0 ) --l;	 
   }
   int numfactors = 0;
   int longFactors = 0;
   float averageFactorLength = 0; 
   if( printlz ){
	   printf("#lz=\n");
	   printf("%d\n", n);
   }
   sa[0] = 0;
   for(int i=0; i<n; ){
	 ++numfactors;
	 if( sa[i] > maxFactorLength )
		 maxFactorLength = sa[i];
	 if( sa[i] > 8 )
		 ++longFactors;
     if( sa[i] < 1 ){
		if( printlz ){
			printf("(%d %c) ", sa[i], x[i]);
		}
		averageFactorLength += 1;
		++i; 
	 }else{
		if( printlz ){
			printf("(%d %d) ", sa[i], prev_occ[i]);
		}
		averageFactorLength += sa[i];
	 	i += sa[i];
	 }
   }
   if( printlz )
	   printf("(0 0)\n");
   double tlz3 = getTime();
   fprintf(stderr,"# Time to calc plcp = %.2f secs\n",tlz3-tlz2);
   printf("Time to calc lz = %.2f secs\n", tlz3-tlz);
   printf("maxFactorLength = %d\n",maxFactorLength);
   printf("numfactors = %d\n",numfactors);
   printf("longFactors (len>8) = %d\n", longFactors);
   averageFactorLength /= numfactors;
   printf("averageFactorLength = %.2f\n", averageFactorLength);

   delete [] x;
   delete [] sa;
   delete [] phi;
   delete [] prev_occ;
   return 0;
}
