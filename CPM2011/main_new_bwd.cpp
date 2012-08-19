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

//typedef balanced_parentheses_support_sada<> bpr_support;
typedef balanced_parentheses_support_simple<> bpr_support;

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
#ifdef DEBUG_BPR_CHECK
   for(int i=0; i<san; ++i){
     int ngv1 = ngv(i, bprs);
	 int ngv2 = naive_ngv(i, sa, san);
	 if( ngv1 != ngv2 ){
   		fprintf(stderr,"# i = %d ngv1=%d ngv2=%d\n",i, ngv1, ngv2);
		break;
	 }
     int pgv1 = pgv(i, bprs);
	 int pgv2 = naive_pgv(i, sa, san);
	 if( pgv1 != pgv2 ){
   		fprintf(stderr,"# i = %d pgv1=%d pgv2=%d\n",i, pgv1, pgv2);
		break;
	 }
   }
#endif  
   sprintf(safilename, "%s.r.int_vector.sa", argv[1]);
   FILE *outfile = fopen(safilename,"w+");
   int_vector<32>::size_type bit_len = san*32;
   fwrite( (char*)&bit_len, 1, sizeof(bit_len), outfile );
   fwrite( (char*)sa, 1, 4*san, outfile );
   fclose(outfile);
   delete [] sa;

   int_vector_file_buffer<8> bwt_buf( bwtfilename );

   int_vector_file_buffer<32> sa_buf( safilename );
//   fprintf(stderr,"sa len = %d\n", sa_buf.int_vector_size);
//   sa_buf.reset();
//   sa_buf.load_next_block();
/*   if(san>6)
	   for(int i=0; i<7; ++i){
	   	fprintf(stderr,"SA[%d]=%d\n", i, sa_buf[i]);
	   }
*/	   
   typedef wavelet_tree_huffman<rank_support_v<>, select_support_dummy, select_support_dummy> tWTH;
//   csa_wt<wavelet_tree_huffman<>, SA_K, 1000000, 32> csa_wt(bwt_buf, sa_buf);
   csa_wt<tWTH, SA_K, 1000000, 32> csa_wt(bwt_buf, sa_buf);

 
   int j = san-1;
   int isaj = 0; // ISA[san-1] = 0

   if( n < max_debug_len ){
	   while( j >= 0 ){
		fprintf(stderr, "# j = %2d isaj = %2d\n", j, isaj);
		--j;
		isaj = csa_wt.psi(isaj);
	   }
	   j = san-1;
	   isaj = 0;
   }

   int_vector_file_buffer<8> textbuf(textfilename);
   int r = textbuf.load_next_block();
   int r_sum = 0;
   unsigned char xi='\0';

   if( printlz ){
	   printf("#lz=\n");
	   printf("%d\n",n);
   }
//   --j;
//   isaj = csa_wt.psi(isaj);
   int i=0;
   while( i < san-1 ){
     // lzfactor
	 int jj = j;  
	 int isajj = isaj;
	 int ii = i;
//	 fprintf(stdout, "i=%2d j=%2d isaj=%2d\n", i, j, isaj);
	 uint64_t lb = 0, rb = san-1, sp = 0, ep=san-1;
	 int lps = -1;
	 int prev_occ = 0;
	 int t=0;
	 do{
		lps++; 
		j = jj; isaj = isajj; 
		if(jj == 0 )
			break;
		--jj; isajj = csa_wt.psi(isajj);
	 	lb = sp; rb = ep;

		if( ii >= r+r_sum ){
			r_sum += r;
			r = textbuf.load_next_block();
		}
		xi = textbuf[ii-r_sum];
/*
		if(xi!=x[jj]){
			fprintf(stderr,"i=%d %c!=%c !!!\n", i,xi,x[jj]);
			return 1;
		}
*/
//		algorithm::backward_search(csa_wt, sp, ep, x[jj], sp, ep);
		if(lps>0){
			algorithm::backward_search(csa_wt, sp, ep, xi, sp, ep);
		}else{
			uint32_t c2c = csa_wt.char2comp[xi];
			sp = csa_wt.C[c2c];
			ep = csa_wt.C[c2c+1]-1;
		}
		++ii;
	 }
	 while( ep > sp and ( ngv(isajj, bprs) <= ep or pgv(isajj, bprs) >= sp ) );

	 if( 0 == lps ){
	 	prev_occ = xi;
	 }else if( (t=ngv(isaj, bprs)) <= rb ){
	 	prev_occ = san-csa_wt[t]-lps-1;
	 }else{	
		prev_occ = san-csa_wt[pgv(isaj, bprs)]-lps-1;
	 }

//	 fprintf(stdout, "i=%2d lps=%2d prev_occ=%2d\n", i, lps, prev_occ);

	 if( lps > maxFactorLength )
		maxFactorLength = lps;
	 if( lps > 8 )
		 ++longFactors;

	 if( 0 == lps ){
		if( printlz ){
			printf("%d %d ", lps, xi);
			compressed_file_size += coder::fibonacci::encoding_length(lps+1) +  8;
		}
	 	++i;
		j--; isaj = csa_wt.psi(isaj);
	 	averageFactorLength += 1;
	 }else{
		if( printlz ){
			printf("%d %d ", lps, prev_occ);
			compressed_file_size += coder::fibonacci::encoding_length(lps+1) +  
									coder::fibonacci::encoding_length( (i-prev_occ) +1);
		}
	 	i += lps;
	 	averageFactorLength += lps;
	 }
	 ++numfactors;	 
   }
   if( printlz )
	   printf("0 0\n");
   printf("Time to calc lz = %.2f secs\n",getTime()-tlz);
   printf("maxFactorLength = %d\n",maxFactorLength);
   printf("numfactors = %d\n",numfactors);
   printf("longFactors (len>8) = %d\n", longFactors);
   averageFactorLength /= numfactors;
   printf("averageFactorLength = %.2f\n", averageFactorLength);

   printf("size of csa_wt in bytes per input symbol = %.2f\n", util::get_size_in_bytes(csa_wt)/(double)san );
   printf("size of bpr in bytes per input symbol =  %.2f\n", util::get_size_in_bytes(bpr)/(double)san );
   printf("size of bprs in bytes per input symbol = %.2f\n", util::get_size_in_bytes(bprs)/(double)san );
   printf("size = %.2f\n", (
			   util::get_size_in_bytes(csa_wt) +
			   util::get_size_in_bytes(bpr) +
			   util::get_size_in_bytes(bprs) 
			   )/(double)san );
   if( printlz ){
		printf("compressed file size in bits = %.2f\n", compressed_file_size );
		printf("compressed file size in Mb = %.2f\n", (compressed_file_size/8.0)/(1024.0*1024.0) );
   }
  
   return 0;
}
