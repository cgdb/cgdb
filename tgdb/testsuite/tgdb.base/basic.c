#include <stdio.h>
#include <unistd.h>

struct global_software_watchpoint {
	int i;
	char string[1000];
} software_watchpoint;

int two(int a){
   return a*a;
}

int one(int a, int b){
   return two(a) + two(b);
}

void short_func(void){
   int i = 0;
   sleep(1);
   i = 1;
   i = 2;
}

void long_func(void){
   int k = 0;
   sleep(3);
   k = 1;
   k = 2;
}

#define SIZE 100000

struct test {
   int a;
   int b;
   int c;  
} a = { 1, 2, 3 };

int main(int argc, char **argv){
   int i = 3;
   int j = 4;
   char temp[SIZE];

   long_func();

   fprintf(stderr, "ABCDEFGHI\n");
   fprintf(stderr, "ABCDEFGH\n");
   fprintf(stderr, "ABCDEFG\n");
   fprintf(stderr, "ABCDEF\n");
   fprintf(stderr, "ABCDE\n");
   fprintf(stderr, "ABCD\n");
   fprintf(stderr, "ABC\n");
   fprintf(stderr, "AB\n");
   fprintf(stderr, "A\n");

   for ( i = 0; i < 100; i++)
       fprintf(stderr, "Yo\n");

   long_func();

   short_func();
   short_func();
   short_func();

   /* A simple loop */
   for(i = 0; i < 100000; ++i)
      j++;

   i = one(i, j);
   j = two(i);

   for(i = 0; i < 6; ++i)
      fprintf(stderr, "returned from init\n");

   for(i = 0; i < SIZE; ++i)
      temp[i] = (i % 40) + 40;

   i += j;
   
   return 0;
}

