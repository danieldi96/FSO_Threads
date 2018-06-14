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

  for (int i=0; i< 200000000 & !no_max; i++){}
  start_t = clock();
  printf("\nCiclos de reloj: %d\n", ((int) start_t));
  float time_start = ((float) start_t/CLOCKS_PER_SEC);
  printf("\nTiempo: %f\n", time_start);

  for (int i=0; i< 2000000000 & !no_max; i++){}

  end_t = clock();
  printf("\nCiclos de reloj: %d\n", ((int) end_t));
  float time_end = ((float) end_t/CLOCKS_PER_SEC);
  printf("\nTiempo: %f\n", time_end);

  float time_total;
  time_total = time_end-time_start;
  printf("El tiempo total ha sido %f\n", time_total);

  /*
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
  */
  return 0;
}
