
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "mmsearch.h"
#include "utils.h"

int main(int argc, char **argv){
   if(argc != 2){
      fprintf(stderr,"usage: %s {inputfile}\n",argv[0]);
      exit(1);
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
   lzFactorize(x,sa,n);
   fprintf(stdout,"Time to lz factorize = %.2f secs\n",getTime()-tlz);
   exit(0);

   delete [] x;
   delete [] sa;
   return 0;
}
