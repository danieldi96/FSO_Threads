#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include "memoria.h"
#define BLKCHAR 'B'
#define FRNTCHAR 'A'
#define ARG 12

pid_t pid_fill;
int id_ipc, id_ipc_com;
int n_fil, n_col, retard;
char str_f_pil[20], str_c_pil[20];
char str_vel_c[20], str_vel_f[20];
char str_pos_c[20], str_pos_f[20];

int f_pil, c_pil;
float vel_c, vel_f;
float pos_c, pos_f;
int* num_pil;
int* num_pil_fora;
int* dirPaleta;
int* nblocs;
int* fi1;
int* fi2;

/* Si hi ha una col.lisió pilota-bloci esborra el bloc */
void comprovar_bloc(int f, int c)
{
	int col;
	char quin = win_quincar(f, c);
	if (quin == BLKCHAR || quin == FRNTCHAR) {
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

		/* TODO: generar nova pilota */
		if (quin == BLKCHAR){
			//Generar nueva pelota FASE 3
		}
		*nblocs -= 1;
	}
}


/* funcio per a calcular rudimentariament els efectes amb la pala */
/* no te en compta si el moviment de la paleta no és recent */
/* cal tenir en compta que després es calcula el rebot */
void control_impacte(void * ind) {
	int index = (intptr_t) ind;
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
void * mou_pilota(void * ind)
{
	int f_h, c_h;
	char rh, rv, rd;
	int index = (intptr_t) ind;
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
					win_escricar(f_pil, c_pil, '1', INVERS);	/* imprimeix pilota */
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
		if (nblocs==0){
			*fi2 = true;
		}
		win_retard(retard);
	} while (!(*fi2) && !(*fi1));
	exit(0);
}

void arguments(){
	id_ipc = atoi(ll_args[1]);
	id_ipc_com = atoi(ll_args[2]);
	f_pil = atoi(ll_args[3]);
	c_pil = atoi(ll_args[4]);
	vel_f = atoi(ll_args[5]);
	vel_c = atoi(ll_args[6]);
	pos_f = atoi(ll_args[7]);
	pos_c = atoi(ll_args[8]);
	n_fil = atoi(ll_args[9]);
	n_col = atoi(ll_args[10]);
	retard = atoi(ll_args[11]);
}
void main(int n_args, char *ll_args[]){
	if (n_args != ARG){
		fprintf(stderr, "Arguments erronis proces fill.\nArguments necessaris %d. Arguments donats: %d", ARG, n_args);
		exit(1);
	}

	arguments();
	void* pun_mem_compartida;

	pun_mem_compartida = map_mem(id_ipc_com);
	num_pil = pun_mem_compartida;
	num_pil_fora = pun_mem_compartida + 4;
	dirPaleta = pun_mem_compartida + 8;
	nblocs = pun_mem_compartida + 12;
	fi1 = pun_mem_compartida + 16;
	fi2 = pun_mem_compartida + 20;

	mou_pilota(*num_pil);

	if (pid_fill != 0) waitpid(pid_fill, NULL, 0);

	return 0;
}
