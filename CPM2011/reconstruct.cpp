// This program reconstructs the text from the lz factorization
#include <stdio.h>
#include <string.h>

using namespace std;

const int bs = 1000000;
char buffer[bs];

int main(){
	while( fgets(buffer, bs,stdin)	){
		if( strcmp(buffer, "#lz=\n") == 0 ){
//			printf("Found LZ\n");
			fgets(buffer, bs, stdin);
			int n;
			sscanf(buffer, "%d", &n);
			unsigned char *x = new unsigned char[n+1];
			int a,b,i=0;
			while( scanf("%d %d", &a, &b) == 2 and !(a==0 and b==0) ){
				if( a ){
					if( false and i  >= b+a ){
						strncpy((char*)x+i, (char*)x+b, a);	
					}else{
						while(a){
							x[i++] = x[b++];
							--a;
						}
					}
				}else{
					x[i++] = (unsigned char)b;
				}
			}
			x[i++] = 0;
//			PRINTF("n=%d\n",n);
			fputs((char*)x, stdout);
		}
	}
}
