#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>
#include "utils.h"
#include <sdsl/int_vector.hpp>
#include <sdsl/util.hpp>
#include <sdsl/sorted_stack_support.hpp>
#include <sdsl/balanced_parentheses_support.hpp>
#include <sdsl/csa_wt.hpp>
#include <sdsl/wavelet_tree_huffman.hpp>
#include <sdsl/algorithms_for_string_matching.hpp> // for backward_search
#include <sdsl/select_support_dummy.hpp>

#include <sdsl/coder.hpp>

#define DEBUG_BPR_CONSTRUCT
//#define DEBUG_BPR_CHECK

//#ifndef SA_K
//	#define SA_K 4
//#endif

using namespace sdsl;

typedef balanced_parentheses_support_sada<> bpr_support;
//typedef balanced_parentheses_support_simple<> bpr_support;

// Construct the balanced parentheses representation (bpr) for PGV and NGV queries
// sa = suffix array
// san = lenght of suffix array
// bpr = reference to the bit_vector of the bpr
void construct_bpr(int *sa, int san, bit_vector &bpr, bpr_support &bprs){
	bpr.resize(2*san);
	util::set_zero_bits(bpr);
	sorted_stack_support vec_stack(san);
#ifdef DEBUG_BPR_CONSTRUCT
	fprintf(stderr, "# begin construction bpr\n");
#endif	
	int k=0; // position in the bitvector
	for(int i=0; i<san; ++i){
		while( vec_stack.size() > 0 and sa[i] > sa[vec_stack.top()] ){
			vec_stack.pop(); 
			++k; // ++k write a closing parenthesis
		} 
		vec_stack.push(i);
		bpr[k++] = 1; // writing an opening parenthesis
	}
	// closing the remaining parentheses is not neccessary
	// as bpr is initialized with closing parentheses
#ifdef DEBUG_BPR_CONSTRUCT
	fprintf(stderr, "# end construction bpr\n");
	fprintf(stderr, "# begin construction bprs\n");
#endif	
	bprs = bpr_support(&bpr);
#ifdef DEBUG_BPR_CONSTRUCT
	fprintf(stderr, "# end construction bprs\n");
#endif	
}

int ngv(int i, const bpr_support &bprs){
	return bprs.rank( bprs.find_close(bprs.select(i+1)) );
}

int naive_ngv(int i, int *sa, int san){
	int r=i+1;
	while( r < san and sa[r] <= sa[i] ){
		++r;
	}
	return r;
}

int pgv(int i, const bpr_support &bprs){
	if( i == 0 )
		return -1;
	int si = bprs.select(i+1);
	int ecsi = bprs.enclose(si);
//	if(i==260)
//	fprintf(stderr, "i=%d si=%d ecsi=%d\n",i, si,ecsi);
	return bprs.rank(ecsi)-1;
}

int naive_pgv(int i, int *sa, int san){
	int r=i-1;
	while(r > -1 and sa[r] <= sa[i] ){
		--r;
	}
	return r;
}


int main(int argc, char **argv){
   const int max_debug_len = 20;
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
      fprintf(stderr,"#Error opening input file %s\n",argv[1]);
      exit(1);
   }
   printf("Test case = %s\n",argv[1]);
   int n = 0;
   fseek(infile,0,SEEK_END);
   n = ftell(infile);
   fseek(infile,0,SEEK_SET);
   unsigned char *x = new unsigned char[n+1];
   if(n != fread(x,sizeof(unsigned char),n,infile)){
      fprintf(stderr,"#Error reading string from file\n");
      exit(1);
   }
   x[n] = 0;
   fclose(infile);   
   fprintf(stderr,"n=%d\n",n);
   char textfilename[256];
   sprintf(textfilename, "%s.int_vector", argv[1]);
   if( !util::store_to_file( char_array_serialize_wrapper<>(x, n+1), textfilename ) ){
   	  fprintf(stderr, "#cannot store text to disk! exiting...");	
	  return 1;
   }
   fprintf(stderr,"#reverse x\n",n);
   for(int i=0, j=n-1; i<j; ++i, --j){
      std::swap(x[i], x[j]);
   }
   if( n < max_debug_len ){
     fprintf(stderr, "x_rev=%s\n", x);
   }

   //read the suffix array of the reversed text
   char safilename[256];
   sprintf(safilename,"%s.r.sa",argv[1]); 
   infile = fopen(safilename,"r"); 
   if(!infile){
      fprintf(stderr,"#Error opening input file %s\n",safilename);
      exit(1);
   }
   int san = 0;
   fseek(infile,0,SEEK_END);
   san = ftell(infile) / sizeof(int); // size of the suffix array
   fseek(infile,0,SEEK_SET);
   int *sa = new int[san];
   if(san != fread(sa,sizeof(int),san,infile)){
      fprintf(stderr,"#Error reading sa from file\n");
      exit(1);
   }
   fclose(infile);

   if( sa[0] != san-1 ){
     fprintf(stderr,"ERROR sa[0]=%d!=%d=n-1", sa[0],san-1);
   }

   int maxFactorLength = 0;
   int longFactors = 0;
   float averageFactorLength = 0;
   int numfactors = 0;
   float compressed_file_size = 0;
   //compute LZ factorization
   double tlz = getTime();

   // create bwt of reverse text
   unsigned char *bwt = new unsigned char[san];
   for(int i=0; i<san; ++i){
	 if( sa[i] > 0 )  
   	 	bwt[i] = x[ (sa[i]-1) ];
	 else
		bwt[i] = 0; 
   }

   delete [] x;

   if( n < max_debug_len ){
     fprintf(stderr, "#bwt=%s\n", bwt);
   }

   char bwtfilename[256];
   sprintf(bwtfilename, "%s.r.bwt",argv[1]);
   if( !util::store_to_file( char_array_serialize_wrapper<>(bwt, san), bwtfilename ) ){
   	  fprintf(stderr, "#cannot store bwt to disk! exiting...");	
	  return 1;
   }
   delete [] bwt;

   bit_vector bpr;
   bpr_support bprs;
   construct_bpr(sa, san, bpr, bprs);

 
   // TODO: free sa here
   // construct wavelet tree from bwt 
//#define DEBUG_BPG_CHECK   
//#ifdef DEBUG_BPR_CHECK
   for(int i=0; i<san; ++i){
     int ngv1 = ngv(i, bprs);
	 int ngv2 = naive_ngv(i, sa, san);
	 if( ngv1 != ngv2 ){
   		fprintf(stderr,"#ERROR i = %d ngv1=%d ngv2=%d\n",i, ngv1, ngv2);
		break;
	 }
     int pgv1 = pgv(i, bprs);
	 int pgv2 = naive_pgv(i, sa, san);
	 if( pgv1 != pgv2 ){
   		fprintf(stderr,"#ERROR i = %d pgv1=%d pgv2=%d\n",i, pgv1, pgv2);
		break;
	 }
	 if(i%10000000==0){
	 	fprintf(stderr,"#i=%d",i);
	 }
   }
//#endif  
   delete [] sa;

  
   return 0;
}
