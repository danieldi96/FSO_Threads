/*****************************************************************************/
/*                                                                           */
/*                           mur3.c                                          */
/*                                                                           */
/*  Programa inicial d'exemple per a les practiques 2 i 3 d'ISO.             */
/*                                                                           */
/*  Compilar i executar:                                                     */
/*     El programa invoca les funcions definides a "winsuport.c", les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':                                   */
/*                                                                           */
/*       $ gcc -c winsuport.c -o winsuport.o                                 */
/*       $ gcc mur3.c winsuport.o -o mur0 -lcurses                           */
/*                                                                           */
/*  Al tenir una orientació vertical cal tenir un terminal amb prou files.   */
/*  Per exemple:                                                             */
/*               xterm -geometry 80x50                                       */
/*               gnome-terminal --geometry 80x50                             */
/*                                                                           */
/*****************************************************************************/

#include <stdint.h>		/* intptr_t for 64bits machines */
#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include "memoria.h"
#include "missatge.h"
#include "semafor.h"

/* Definicio Structs*/
struct params{
	int f_pil, c_pil;
	float vel_c, vel_f;
	float pos_c, pos_f;
};

/* definicio de constants */
#define MAX_THREADS	10
#define MAXBALLS	(MAX_THREADS-1)
#define MIN_FIL	10		/* dimensions del camp. Depenen del terminal ! */
#define MAX_FIL	50
#define MIN_COL	10
#define MAX_COL	80
#define MIDA_PALETA (MIN_COL-4)	/* podria ser un paràmetre més */
#define BLKSIZE	3
#define BLKGAP	2
#define BLKCHAR 'B'
#define WLLCHAR '#'
#define FRNTCHAR 'A'
#define TOPCHAR 'T'
#define LONGMISS	65
			/* variables globals */
char *descripcio[] = {
	"\n",
	"Aquest programa implementa una versio basica del joc Arkanoid o Breakout:\n",
	"generar un camp de joc rectangular amb una porteria, una paleta que s\'ha\n",
	"de moure amb el teclat per a cobrir la porteria, i una pilota que rebota\n",
	"contra les parets del camp, a la paleta i els blocs. El programa acaba si\n",
	"la pilota surt per la porteria o no queden mes blocs. Tambe es pot acabar\n",
	"amb la tecla RETURN.\n",
	"\n",
	"  Arguments del programa:\n",
	"\n",
	"       $ ./mur3 fitxer_config [retard]\n",
	"\n",
	"     El primer argument ha de ser el nom d\'un fitxer de text amb la\n",
	"     configuracio de la partida, on la primera fila inclou informacio\n",
	"     del camp de joc (valors enters), i la segona fila indica posicio\n",
	"     i velocitat de la pilota (valors reals):\n",
	"          num_files  num_columnes  mida_porteria\n",
	"          pos_fila   pos_columna   vel_fila  vel_columna\n",
	"\n",
	"     on els valors minims i maxims admesos son els seguents:\n",
	"          MIN_FIL <= num_files     <= MAX_FIL\n",
	"          MIN_COL <= num_columnes  <= MAX_COL\n",
	"          0 < mida_porteria < num_files-2\n",
	"        1.0 <= pos_fila     <= num_files-3\n",
	"        1.0 <= pos_columna  <= num_columnes\n",
	"       -1.0 <= vel_fila     <= 1.0\n",
	"       -1.0 <= vel_columna  <= 1.0\n",
	"\n",
	"     Alternativament, es pot donar el valor 0 a num_files i num_columnes\n",
	"     per especificar que es vol que el tauler ocupi tota la pantalla. Si\n",
	"     tambe fixem mida_porteria a 0, el programa ajustara la mida d\'aquesta\n",
	"     a 3/4 de l\'altura del camp de joc.\n",
	"\n",
	"     A mes, es pot afegir un segon argument opcional per indicar el\n",
	"     retard de moviment del joc en mil.lisegons; el valor minim es 10,\n",
	"     el valor maxim es 1000, i el valor per defecte d'aquest parametre\n",
	"     es 100 (1 decima de segon).\n",
	"\n",
	"  Codis de retorn:\n",
	"     El programa retorna algun dels seguents codis:\n",
	"	0  ==>  funcionament normal\n",
	"	1  ==>  numero d'arguments incorrecte\n",
	"	2  ==>  no s\'ha pogut obrir el fitxer de configuracio\n",
	"	3  ==>  algun parametre del fitxer de configuracio es erroni\n",
	"	4  ==>  no s\'ha pogut crear el camp de joc (no pot iniciar CURSES)\n",
	"\n",
	"   Per a que pugui funcionar aquest programa cal tenir instal.lada la\n",
	"   llibreria de CURSES (qualsevol versio).\n",
	"\n",
	"*"
};				/* final de la descripcio */

int n_fil, n_col;		/* numero de files i columnes del taulell */
int m_por;			/* mida de la porteria (en caracters) */
int f_pal, c_pal;		/* posicio del primer caracter de la paleta */
int retard;			/* valor del retard de moviment, en mil.lisegons */
char strin[LONGMISS];			/* variable per a generar missatges de text */

// Arguments fase 3
int sec=0;
int min=0;
int id_ipc;
int id_ipc_com;
void* pun_mem_compartida;
void* pun_mem_window;
pid_t list_procs[MAXBALLS];
struct params parametres[MAXBALLS];

// Variables per a transformar en String
char str_retard[20];
char str_n_fil[20], str_n_col[20];
char str_id_ipc[20], str_id_ipc_com[20];
char str_f_pil[20], str_c_pil[20];
char str_vel_c[20], str_vel_f[20];
char str_pos_c[20], str_pos_f[20];

// Variables amb memoria compartida
int* num_pil = 0;
int* num_pil_act = 0;
int* dirPaleta = 0;
int* nblocs = 0;
int* fi1 = false;
int* fi2 = false;
int* blocs_t_invers;
int* max_time;
int* id_semafor;
int* id_bustia;
int* pil_fora = 0;


/*funcio que s'encarrega de pasar els valors numerics a una cadena de caracters (Strings) */
void toString(int pilota){
	sprintf(str_retard, "%d", retard);
	sprintf(str_n_fil, "%d", n_fil);
	sprintf(str_n_col, "%d", n_col);
	sprintf(str_id_ipc, "%d", id_ipc);
	sprintf(str_id_ipc_com, "%d", id_ipc_com);
	sprintf(str_f_pil, "%d", parametres[pilota].f_pil);
	sprintf(str_c_pil, "%d", parametres[pilota].c_pil);
	sprintf(str_vel_f, "%f", parametres[pilota].vel_f);
	sprintf(str_vel_c, "%f", parametres[pilota].vel_c);
	sprintf(str_pos_f, "%f", parametres[pilota].pos_f);
	sprintf(str_pos_c, "%f", parametres[pilota].pos_c);
}

// Funcio que s'encarrega de crear les variables amb memoria compartida
void inicialitzar_variables(){
	int tam_mem;
	//Calculem el tamany de la memoria compartida
	tam_mem = sizeof(int)*11;

	//Inicialitzem la memoria compartida i obtenim l'id
	id_ipc_com = ini_mem(tam_mem);

	//Obtenir referencia de la memoria compartida
	pun_mem_compartida = map_mem(id_ipc_com);

	num_pil = pun_mem_compartida;
	num_pil_act = pun_mem_compartida + sizeof(int)*1;
	dirPaleta = pun_mem_compartida + sizeof(int)*2;
	nblocs = pun_mem_compartida + sizeof(int)*3;
	fi1 = pun_mem_compartida + sizeof(int)*4;
	fi2 = pun_mem_compartida + sizeof(int)*5;
	blocs_t_invers = pun_mem_compartida + sizeof(int)*6;
	max_time = pun_mem_compartida + sizeof(int)*7;
	id_semafor = pun_mem_compartida + sizeof(int)*8;
	id_bustia = pun_mem_compartida + sizeof(int)*9;
	pil_fora = pun_mem_compartida + sizeof(int)*10;

	*blocs_t_invers = INVERS;
	*id_semafor = ini_sem(1);
	*id_bustia = ini_mis();
}

/* funcio per carregar i interpretar el fitxer de configuracio de la partida */
/* el parametre ha de ser un punter a fitxer de text, posicionat al principi */
/* la funcio tanca el fitxer, i retorna diferent de zero si hi ha problemes  */
int carrega_configuracio(FILE * fit)
{
	int ret = 0;

	fscanf(fit, "%d %d %d\n", &n_fil, &n_col, &m_por);	/* camp de joc */
	fscanf(fit, "%f %f %f %f\n", &parametres[0].pos_f, &parametres[0].pos_c, &parametres[0].vel_f, &parametres[0].vel_c);	/* pilota */
	if ((n_fil != 0) || (n_col != 0)) {	/* si no dimensions maximes */
		if ((n_fil < MIN_FIL) || (n_fil > MAX_FIL) || (n_col < MIN_COL) || (n_col > MAX_COL))
			ret = 1;
		else if (m_por > n_col - 3)
			ret = 2;
		else if ((parametres[0].pos_f < 1) || (parametres[0].pos_f >= n_fil - 3) || (parametres[0].pos_c < 1)
			 || (parametres[0].pos_c > n_col - 1))	/* tres files especials: línia d'estat, porteria i paleta */
			ret = 3;
	}
	if ((parametres[0].vel_f < -1.0) || (parametres[0].vel_f > 1.0) || (parametres[0].vel_c < -1.0) || (parametres[0].vel_c > 1.0))
		ret = 4;

	if (ret != 0) {		/* si ha detectat algun error */
		fprintf(stderr, "Error en fitxer de configuracio:\n");
		switch (ret) {
		case 1:
			fprintf(stderr,
				"\tdimensions del camp de joc incorrectes:\n");
			fprintf(stderr, "\tn_fil= %d \tn_col= %d\n", n_fil,
				n_col);
			break;
		case 2:
			fprintf(stderr, "\tmida de la porteria incorrecta:\n");
			fprintf(stderr, "\tm_por= %d\n", m_por);
			break;
		case 3:
			fprintf(stderr, "\tposicio de la pilota incorrecta:\n");
			fprintf(stderr, "\tpos_f= %.2f \tpos_c= %.2f\n", parametres[0].pos_f,
				parametres[0].pos_c);
			break;
		case 4:
			fprintf(stderr,
				"\tvelocitat de la pilota incorrecta:\n");
			fprintf(stderr, "\tvel_f= %.2f \tvel_c= %.2f\n", parametres[0].vel_f,
				parametres[0].vel_c);
			break;
		}
	}
	fclose(fit);
	return (ret);
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
/* retorna diferent de zero si hi ha algun problema */
int inicialitza_joc(void)
{
	int i, retwin;
	int i_port, f_port;	/* inici i final de porteria */
	int c, nb, offset;

	retwin = win_ini(&n_fil, &n_col, '+', INVERS);	/* intenta crear taulell */

	if (retwin < 0) {	/* si no pot crear l'entorn de joc amb les curses */
		fprintf(stderr, "Error en la creacio del taulell de joc:\t");
		switch (retwin) {
		case -1:
			fprintf(stderr, "camp de joc ja creat!\n");
			break;
		case -2:
			fprintf(stderr,
				"no s'ha pogut inicialitzar l'entorn de curses!\n");
			break;
		case -3:
			fprintf(stderr,
				"les mides del camp demanades son massa grans!\n");
			break;
		case -4:
			fprintf(stderr, "no s'ha pogut crear la finestra!\n");
			break;
		}
		return (retwin);
	}

	id_ipc = ini_mem(retwin);											//memoria compartida = retwin + 4 B/int * 6 variables globals
	pun_mem_window = map_mem(id_ipc);
	win_set(pun_mem_window, n_fil, n_col);										//TODO: No se si hay que sumarle el offset al pun_mem en win_set. Preguntar a la Moncusi

	if (m_por > n_col - 2)
		m_por = n_col - 2;	/* limita valor de la porteria */
	if (m_por == 0)
		m_por = 3 * (n_col - 2) / 4;	/* valor porteria per defecte */

	i_port = n_col / 2 - m_por / 2 - 1;	/* crea el forat de la porteria */
	f_port = i_port + m_por - 1;
	for (i = i_port; i <= f_port; i++)
		win_escricar(n_fil - 2, i, ' ', NO_INV);

	n_fil = n_fil - 1;	/* descompta la fila de missatges */
	f_pal = n_fil - 2;	/* posicio inicial de la paleta per defecte */
	c_pal = (n_col-MIDA_PALETA) / 2;	/* a baix i centrada */
	for (i = 0; i < MIDA_PALETA; i++)	/* dibuixar paleta inicial */
		win_escricar(f_pal, c_pal + i, '0', INVERS);

	/* generar la pilota */
	if (parametres[0].pos_f > n_fil - 1)
		parametres[0].pos_f = n_fil - 1;	/* limita posicio inicial de la pilota */
	if (parametres[0].pos_c > n_col - 1)
		parametres[0].pos_c = n_col - 1;
	parametres[0].f_pil = parametres[0].pos_f;
	parametres[0].c_pil = parametres[0].pos_c;		/* dibuixar la pilota inicialment */
	win_escricar(parametres[0].f_pil, parametres[0].c_pil, '1', INVERS);

	/* generar els blocs */
	nb = 0;
	*nblocs = n_col / (BLKSIZE + BLKGAP) - 1;
	offset = (n_col - (*nblocs) * (BLKSIZE + BLKGAP) + BLKGAP) / 2;					/* offset de columna inicial */
	for (i = 0; i < (*nblocs); i++) {
		for (c = 0; c < BLKSIZE; c++) {
			win_escricar(3, offset + c, TOPCHAR, INVERS);
			nb++;
			win_escricar(4, offset + c, BLKCHAR, NO_INV);
			nb++;
			win_escricar(5, offset + c, FRNTCHAR, INVERS);
			nb++;
		}
		offset += BLKSIZE + BLKGAP;
	}
	*nblocs = nb / BLKSIZE;
	/* generar les defenses */
	nb = n_col / (BLKSIZE + 2 * BLKGAP) - 2;
	offset = (n_col - nb * (BLKSIZE + 2 * BLKGAP) + BLKGAP) / 2;						/* offset de columna inicial */
	for (i = 0; i < nb; i++) {
		for (c = 0; c < BLKSIZE + BLKGAP; c++) {
			win_escricar(6, offset + c, WLLCHAR, NO_INV);
		}
		offset += BLKSIZE + 2 * BLKGAP;
	}
	sprintf(strin,
		"Tecles: \'%c\'-> Esquerra, \'%c\'-> Dreta, RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL);
	win_escristr(strin);
	return (0);
}

/* funcio que escriu un missatge a la línia d'estat i tanca les curses */
void mostra_final(char *miss)
{	int lmarge;
	char marge[LONGMISS];
	char new_message[LONGMISS];

	//Afegim el temps al string del missatge
	sprintf(new_message, "%s Temps-> %d : %d", miss, min, sec);

	/* centrar el misssatge */
	lmarge=(n_col+strlen(new_message))/2;
	sprintf(marge,"%%%ds",lmarge);
	sprintf(strin, marge,new_message);
	signalS(*id_semafor);
	win_escristr(strin);

	win_update();
	/* espera tecla per a que es pugui veure el missatge */
	getchar();
}

/* funcio per moure la paleta segons la tecla premuda */
/* retorna un boolea indicant si l'usuari vol acabar */
void * mou_paleta(void * nul)
{
	int tecla;

	do{
		tecla = win_gettec();
		if (tecla != 0) {
			if ((tecla == TEC_DRETA)
				&& ((c_pal + MIDA_PALETA) < n_col - 1)) {
					waitS(*id_semafor);
					win_escricar(f_pal, c_pal, ' ', NO_INV);										/* esborra primer bloc */
					signalS(*id_semafor);
					c_pal++;																										/* actualitza posicio */
					waitS(*id_semafor);
					win_escricar(f_pal, c_pal + MIDA_PALETA - 1, '0', INVERS);	/*esc. ultim bloc */
					signalS(*id_semafor);
			}
			if ((tecla == TEC_ESQUER) && (c_pal > 1)) {
				waitS(*id_semafor);
				win_escricar(f_pal, c_pal + MIDA_PALETA - 1, ' ', NO_INV);	/*esborra ultim bloc */
				signalS(*id_semafor);
				c_pal--;																										/* actualitza posicio */
				waitS(*id_semafor);
				win_escricar(f_pal, c_pal, '0', INVERS);										/* escriure primer bloc */
				signalS(*id_semafor);
			}
			if (tecla == TEC_RETURN){
				*fi1 = true;
			}
			*dirPaleta = tecla;																							/* per a afectar al moviment de les pilotes */
		}
		win_retard(retard);
	} while (!(*fi1) && !(*fi2));
	exit(0);
}

/* programa principal */
int main(int n_args, char *ll_args[])
{
	int i;
	FILE *fit_conf;
	pthread_t th_paleta;

	if ((n_args != 2) && (n_args != 3)) {												/* si numero d'arguments incorrecte */
		i = 0;
		do
			fprintf(stderr, "%s", descripcio[i++]);										/* imprimeix descripcio */
		while (descripcio[i][0] != '*');														/* mentre no arribi al final */
		exit(1);
	}

	fit_conf = fopen(ll_args[1], "rt");	/* intenta obrir el fitxer */
	if (!fit_conf) {
		fprintf(stderr, "Error: no s'ha pogut obrir el fitxer \'%s\'\n",
			ll_args[1]);
		exit(2);
	}

	if (carrega_configuracio(fit_conf) != 0)										/* llegir dades del fitxer  */
		exit(3);																										/* aborta si hi ha algun problema en el fitxer */

	if (n_args == 3) {																					/* si s'ha especificat parametre de retard */
		retard = atoi(ll_args[2]);																	/* convertir-lo a enter */
		if (retard < 10)
			retard = 10;																							/* verificar limits */
		if (retard > 1000)
			retard = 1000;
	} else
		retard = 100;																								/* altrament, fixar retard per defecte */

	printf("Joc del Mur: prem RETURN per continuar:\n");
	getchar();
	inicialitzar_variables();
	//Inicialitzzem el joc i aconseguim la mida del taulell
	if (inicialitza_joc() != 0)																		/* intenta crear el taulell de joc */
		exit(4);																										/* aborta si hi ha algun problema amb taulell */
	win_update();
	//Creem el thread de la paleta
	pthread_create(&th_paleta, NULL, mou_paleta, (void *) NULL);

	//Inicialitzem els recursos necessaris

	toString(*num_pil);
	list_procs[*num_pil] = fork();
	if (list_procs[0] == (pid_t) 0){			//Proces fill
		execlp("./pilota4", "pilota4", str_id_ipc, str_id_ipc_com, str_f_pil, str_c_pil, str_vel_f,
		str_vel_c, str_pos_f, str_pos_c, str_n_fil, str_n_col, str_retard, (char *) 0);
		exit(0);
	} else {
		(*num_pil)++;										//Proces pare
		(*num_pil_act)++;
	}

	char temps[10];
	int tem=0;
	do {
/********** bucle principal del joc **********/
		tem += retard;
		if (tem%1000==0){
			sec++;
			if (sec>=60){
				min++;
				sec=0;
			}
			sprintf(temps,"Temps %d : %d", min, sec);
			if ((*max_time) != 0){
				sprintf(temps,"Temps %d : %d\t Temps bloc T: %d", min, sec, (*max_time));
				waitS(*id_semafor);
				(*max_time)--;
				signalS(*id_semafor);
			}
			waitS(*id_semafor);
			win_escristr(temps);
			signalS(*id_semafor);
			tem=0;
		}
		waitS(*id_semafor);
		win_update();
		signalS(*id_semafor);
		win_retard(retard);	/* retard del joc */
	} while (!(*fi1) && !(*fi2));

	if (*nblocs == 0)
		mostra_final("YOU WIN !");
	else
		mostra_final("GAME OVER");

	waitpid(list_procs[0], NULL, 0);
	pthread_join(th_paleta, NULL);

	win_fi();		/* tanca les curses */
	elim_sem(*id_semafor);
	elim_mis(*id_bustia);
	elim_mem(id_ipc);
	elim_mem(id_ipc_com);

	return (0);		/* retorna sense errors d'execucio */
}
