#include <stdint.h>		/* intptr_t for 64bits machines */
#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


//Prueba de un cronometro de 5 segundos
int main(){
  clock_t start_t, end_t, total_t;
  double total_time;
  bool no_max = false;
  float max_time = 5.0;
  for (int i=0; i< 2000000000 & !no_max; i++){}
  start_t = clock();
  float l = ((double)start_t/CLOCKS_PER_SEC);
  printf("\n-> %f\n\n", l);
  max_time = (max_time - (((float) clock()/CLOCKS_PER_SEC))) + 5.0;

  printf("\nTiempo total -> %f", max_time);
  start_t = clock();
  float t = ((double)start_t/CLOCKS_PER_SEC);
  printf("\nstart_T : %f\n\n", t);
  for(int i=0; i< 20000000000 & !no_max; i++) {
    end_t = clock();
    if (((double) (end_t - start_t) / CLOCKS_PER_SEC) >= max_time) no_max = true;
   }
  end_t = clock();
  total_time = (double) (end_t - start_t) / CLOCKS_PER_SEC;
  printf("%f\n", total_time);

  return 0;
}
