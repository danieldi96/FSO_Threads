#include <stdint.h>		/* intptr_t for 64bits machines */
#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include "memoria.h"

#define BLKCHAR 'B'
#define FRNTCHAR 'A'
#define TOPCHAR 'T'
#define ARG 12

pid_t pid_fill;
int id_ipc, id_ipc_com;
int n_fil, n_col, retard;
int f_pil, c_pil;
float vel_c, vel_f;
float r_vel_f, r_vel_c;
float pos_c, pos_f;
float time_total, time_start, time_end;
void* pun_mem_compartida;
void* pun_mem_pantalla;

// Variables per a transformar en String
char str_retard[20];
char str_n_fil[20], str_n_col[20];
char str_id_ipc[20], str_id_ipc_com[20];
char str_f_pil[20], str_c_pil[20];
char str_vel_c[20], str_vel_f[20];
char str_pos_c[20], str_pos_f[20];

int* num_pil;
int* num_pil_fora;
int* dirPaleta;
int* nblocs;
int* fi1;
int* fi2;
int* blocs_t_invers;
int* max_time;

/* Si hi ha una col.lisió pilota-bloci esborra el bloc */
void comprovar_bloc(int f, int c)
{
	int col;
	char quin = win_quincar(f, c);
	if (quin == BLKCHAR || quin == FRNTCHAR || quin == TOPCHAR) {
		col = c;
		while (win_quincar(f, col) != ' ') {
			win_escricar(f, col, ' ', NO_INV);
			col++;
		}
		col = c - 1;
		while (win_quincar(f, col) != ' ') {
			win_escricar(f, col, ' ', NO_INV);
			col--;
		}
		if (quin == TOPCHAR){
			if (*max_time == 0) *max_time = 5;
			else *max_time += 5;
			*blocs_t_invers = NO_INV;
		}
		/* TODO: generar nova pilota */
		if (quin == BLKCHAR){
			//Generar nova pilota FASE 3
			(*num_pil)++;
			srand(time(NULL));
			r_vel_f = ((rand()%100)+1)*0.01;
			r_vel_c = ((rand()%100)+1)*0.01;

			sprintf(str_retard, "%d", retard);
			sprintf(str_n_fil, "%d", n_fil);
			sprintf(str_n_col, "%d", n_col);
			sprintf(str_id_ipc, "%d", id_ipc);
			sprintf(str_id_ipc_com, "%d", id_ipc_com);
			sprintf(str_f_pil, "%d", f_pil);
			sprintf(str_c_pil, "%d", c_pil);
			sprintf(str_vel_f, "%f", r_vel_f);
			sprintf(str_vel_c, "%f", r_vel_c);
			sprintf(str_pos_f, "%f", pos_f);
			sprintf(str_pos_c, "%f", pos_c);

			pid_fill = fork();
			if (pid_fill == (pid_t) 0){			//Proces fill
				execlp("./pilota4", "pilota4", str_id_ipc, str_id_ipc_com, str_f_pil, str_c_pil, str_vel_f,
				str_vel_c, str_pos_f, str_pos_c, str_n_fil, str_n_col, str_retard, (char *) 0);
			}
		}
		*nblocs -= 1;
	}
}


/* funcio per a calcular rudimentariament els efectes amb la pala */
/* no te en compta si el moviment de la paleta no és recent */
/* cal tenir en compta que després es calcula el rebot */
void control_impacte(int ind) {
	if (*dirPaleta == TEC_DRETA) {
		if (vel_c <= 0.0)	/* pilota cap a l'esquerra */
			vel_c = -(vel_c) - 0.2;	/* xoc: canvi de sentit i reduir velocitat */
		else {	/* a favor: incrementar velocitat */
			if (vel_c <= 0.8)
				vel_c += 0.2;
		}
	} else {
		if (*dirPaleta == TEC_ESQUER) {
			if (vel_c >= 0.0)	/* pilota cap a la dreta */
				vel_c = -(vel_c) + 0.2;	/* xoc: canvi de sentit i reduir la velocitat */
			else {	/* a favor: incrementar velocitat */
				if (vel_c >= -0.8)
					vel_c -= 0.2;
			}
		} else {	/* XXX trucs no documentats */
			if (*dirPaleta == TEC_AMUNT)
				vel_c = 0.0;	/* vertical */
			else {
				if (*dirPaleta == TEC_AVALL)
					if (vel_f <= 1.0)
						vel_f -= 0.2;	/* frenar */
			}
		}
	}

	*dirPaleta=0;	/* reset perque ja hem aplicat l'efecte */

}

/* funcio per moure la pilota: retorna un 1 si la pilota surt per la porteria,*/
/* altrament retorna un 0 */
void* mou_pilota(int ind)
{
	int f_h, c_h;
	char rh, rv, rd;
	do{
		f_h = pos_f + vel_f;	/* posicio hipotetica de la pilota (entera) */
		c_h = pos_c + vel_c;
		rh = rv = rd = ' ';
		if ((f_h != f_pil) || (c_h != c_pil)) {
		/* si posicio hipotetica no coincideix amb la posicio actual */
			if (f_h != f_pil) {	/* provar rebot vertical */
				rv = win_quincar(f_h, c_pil);	/* veure si hi ha algun obstacle */
				if (rv != ' ') {	/* si hi ha alguna cosa */
					comprovar_bloc(f_h, c_pil);
					if (rv == '0')	/* col.lisió amb la paleta? */
						/* XXX: tria la funció que vulgis o implementa'n una millor */
						control_impacte(ind);
	//					vel_c = control_impacte2(c_pil, vel_c);
					vel_f = -(vel_f);	/* canvia sentit velocitat vertical */
					f_h = pos_f + vel_f;	/* actualitza posicio hipotetica */
				}
			}
			if (c_h != c_pil) {	/* provar rebot horitzontal */
				rh = win_quincar(f_pil, c_h);	/* veure si hi ha algun obstacle */
				if (rh != ' ') {	/* si hi ha algun obstacle */
					comprovar_bloc(f_pil, c_h);
					/* TODO?: tractar la col.lisio lateral amb la paleta */
					vel_c = -vel_c;	/* canvia sentit vel. horitzontal */
					c_h = pos_c + vel_c;	/* actualitza posicio hipotetica */
				}
			}
			if ((f_h != f_pil) && (c_h != c_pil)) {	/* provar rebot diagonal */
				rd = win_quincar(f_h, c_h);
				if (rd != ' ') {	/* si hi ha obstacle */
					comprovar_bloc(f_h, c_h);
					/* TODO?: tractar la col.lisio amb la paleta */
					vel_f = -(vel_f);
					vel_c = -(vel_c);	/* canvia sentit velocitats */
					f_h = pos_f + vel_f;
					c_h = pos_c + vel_c;	/* actualitza posicio entera */
				}
			}
			/* mostrar la pilota a la nova posició */
			if (win_quincar(f_h, c_h) == ' '){	/* verificar posicio definitiva *//* si no hi ha obstacle */
				win_escricar(f_pil, c_pil, ' ', NO_INV);	/* esborra pilota */
				pos_f += vel_f;
				pos_c += vel_c;
				f_pil = f_h;
				c_pil = c_h;	/* actualitza posicio actual */
				if (f_pil != n_fil - 1){	/* si no surt del taulell, */
					char id[4];
					sprintf(id, "%d", ind);
					win_escricar(f_pil, c_pil, *id, *blocs_t_invers);	/* imprimeix pilota */
				}else{
						if (*num_pil != *num_pil_fora){
							*num_pil_fora += 1;
							win_escricar( f_pil, c_pil, ' ', NO_INV); //esborrem la pilota
						} else
							*fi2 = true;
				}
			}
		} else {	/* posicio hipotetica = a la real: moure */
			pos_f += vel_f;
			pos_c += vel_c;
		}
		if ((*max_time) == 0){
			*blocs_t_invers = INVERS;
		}
		if (*nblocs==0){
			*fi2 = true;
		}
		win_retard(retard);
	} while (!(*fi2) && !(*fi1));
	exit(0);
}

void arguments(char *ll_args[]){
	id_ipc = atoi(ll_args[1]);
	id_ipc_com = atoi(ll_args[2]);
	f_pil = atoi(ll_args[3]);
	c_pil = atoi(ll_args[4]);
	vel_f = atof(ll_args[5]);
	vel_c = atof(ll_args[6]);
	pos_f = atof(ll_args[7]);
	pos_c = atof(ll_args[8]);
	n_fil = atoi(ll_args[9]);
	n_col = atoi(ll_args[10]);
	retard = atoi(ll_args[11]);
}

int main(int n_args, char *ll_args[]){
	if (n_args != ARG){
		fprintf(stderr, "Arguments erronis proces fill.\nArguments necessaris %d. Arguments donats: %d", ARG, n_args);
		exit(1);
	}

	arguments(ll_args);

	pun_mem_pantalla = map_mem(id_ipc);
	pun_mem_compartida = map_mem(id_ipc_com);

	win_set(pun_mem_pantalla, n_fil, n_col);

	num_pil = pun_mem_compartida;
	num_pil_fora = pun_mem_compartida + sizeof(int)*1;
	dirPaleta = pun_mem_compartida + sizeof(int)*2;
	nblocs = pun_mem_compartida + sizeof(int)*3;
	fi1 = pun_mem_compartida + sizeof(int)*4;
	fi2 = pun_mem_compartida + sizeof(int)*5;
	blocs_t_invers = pun_mem_compartida + sizeof(int)*6;
	max_time = pun_mem_compartida + sizeof(int)*7;

	mou_pilota(*num_pil);

	if (pid_fill != 0) waitpid(pid_fill, NULL, 0);

	return 0;
}
