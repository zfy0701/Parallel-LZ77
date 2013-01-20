#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
//#define V0 //prints just the runs
//#define V1 //prints debugging info

//Written by Simon Puglisi
//July 2006

double
getTime(){
        double usertime, systime;
        struct rusage usage;
        getrusage (RUSAGE_SELF, &usage);
        usertime = (double) usage.ru_utime.tv_sec +
                (double) usage.ru_utime.tv_usec / 1000000.0;
        systime = (double) usage.ru_stime.tv_sec +
                (double) usage.ru_stime.tv_usec / 1000000.0;
        return (usertime + systime);
}

//this function prints any char in a readable form
void pretty_putchar(int c){
  if(c == ' ')
    printf("\\s");
  else if(c>=32 && c<127)      // printable char
    printf(" %c", c);
  else if(c=='\n')
    printf("\\n");        // \n
  else if(c==13)
    printf("\\l");        // linefeed
  else if(c=='\t')
    printf("\\t");        // \t
  else
    printf("%02x", c);      // print hex code
}

/*lookup table used by logbase2 function*/
static const char LogTable256[] =
{
  0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

/*returns the log (base 2) of v*/
int logbase2(unsigned int v){
   unsigned r = 0;
   register unsigned int t, tt;
   if (tt = v >> 16){
     r = (t = v >> 24) ? 24 + LogTable256[t] : 16 + LogTable256[tt & 0xFF];
   }
   else{
     r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
   }
   return r;
}

void printCharArray(char *name, char *a, int n){
   printf("%s",name);
   for(int i=0;i<n;i++){
      pretty_putchar(a[i]);
   }
   printf("\n");
}

