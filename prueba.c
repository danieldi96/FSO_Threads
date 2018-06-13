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
  float max_time = 5;

  bool no_max = false;
  start_t = clock();
  printf("HOLA\n\n");
  for(int i=0; i< 20000000000 & !no_max; i++) {
    end_t = clock();
    if (((double) (end_t - start_t) / CLOCKS_PER_SEC) > max_time) no_max = true;
   }
  end_t = clock();
  total_time = (double) (end_t - start_t) / CLOCKS_PER_SEC;
  printf("%f\n", total_time);

  return 0;
}
