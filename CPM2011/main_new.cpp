#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>
#include "utils.h"

void sop(int i, int l, int j, int *lps, int *prev_occ, int bot){
//	printf("i = %d, j = %d, l=%d\n",i,j,l);
	if( j == 0 and l ==0 and i==0 )
		return;
//	if( l == 0 )
//		return;
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

   //compute LZ factorization
   double tlz = getTime();
//   lzFactorize(x,sa,n);
//   fprintf(stderr,"Time to lz factorize = %.2f secs\n",getTime()-tlz);
//   exit(0);
   // compute PHI
   int *phi = new int [san];
   int *prev_occ = new int [san];
   for(int i=0; i<san; ++i)
	   prev_occ[i] = x[i];
   int to_add[2] = {-1,san-1};
   for(int i=0; i<san; ++i){
//	  if( i<20 )
//		 printf("%d %d %d\n",i,sa[i],san-1); 
//	  if(sa[i]==0){
//		 printf("%d %d %d\n",i,sa[i],san-1); 
//	  }
   	 phi[sa[i]] = sa[i+to_add[i==0]];
   }
   fprintf(stderr,"# Time to calc phi = %.2f secs\n",getTime()-tlz);
   double tlz2 = getTime();
   // sa holds now LPS
   for(int i=0; i<san; ++i)
     sa[i] = n;	   

   int maxFactorLength = 0;
   int l = 0;
   for(int i=0; i<n; ++i){
//	 printf("i=%d, l=%d\n",i,l);  
     int j = phi[i];
	 while( x[i+l] == x[j+l] ) ++l;
/*	
	 if(i > j)
	 	sop(i, l, j, sa, prev_occ, n);	 
	 else
	 	sop(j, l, i, sa, prev_occ, n);	 
*/		
///*	 
	 int ii = 0, jj = 0;
	 if( i > j){
	 	ii = i; jj = j;
	 }else{
	 	ii = j; jj = i;
	 }
	 int ll = l;
	 while( sa[ii] != n ){
//	 printf("ii=%d, ll=%d,jj=%d\n",ii,ll,jj);  
//	assert(ii!=jj);	
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
//*/
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
//	  printf("%d %d %d\n",i,sa[i],n); 
	 ++numfactors;
	 if( sa[i] > maxFactorLength )
		 maxFactorLength = sa[i];
	 if( sa[i] > 8 )
		 ++longFactors;
     if( sa[i] < 1 ){
		if( printlz ){
			printf("%d %d ", sa[i], x[i]);
		}
		++i; 
		averageFactorLength += 1;
	 }else{
		if( printlz ){
			printf("%d %d ", sa[i], prev_occ[i]);
		}
	 	i += sa[i];
		averageFactorLength += sa[i];
	 }
   }
   if( printlz )
	   printf("0 0\n");
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
