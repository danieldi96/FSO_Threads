# una pilota
mur0 : mur0.c winsuport.o winsuport.h
	gcc -Wall -g mur0.c winsuport.o -o mur0 -lcurses

#  amb threads
mur1 : mur1.c winsuport.o winsuport.h
	gcc -Wall -g mur1.c winsuport.o -o mur1 -lcurses -lpthread

mur2 : mur2.c winsuport.o winsuport.h
	gcc -Wall mur2.c winsuport.o -o mur2 -lcurses -lpthread

mur3 : mur3.c winsuport2.o winsuport2.h
	gcc -Wall -g mur3.c winsuport2.o memoria.o -o mur3 -lcurses -lpthread

mur4 : mur4.c winsuport2.o winsuport2.h
	gcc -Wall -g mur4.c winsuport2.o memoria.o semafor.o missatge.o -o mur4 -lcurses -lpthread

# auxiliars per a les curses
winsuport : winsuport.c winsuport.h
	gcc -c -Wall winsuport.c

winsuport2 : winsuport2.c winsuport2.h
	gcc -Wall -c winsuport2.c -o winsuport2.o

pilota3 : pilota3.c winsuport2.c winsuport2.h
	gcc -Wall -g pilota3.c winsuport2.o memoria.o -o pilota3 -lcurses

pilota4 : pilota4.c winsuport2.c winsuport2.h
	gcc -Wall -g pilota4.c winsuport2.o memoria.o missatge.o semafor.o -o pilota4 -lcurses

memoria : memoria.c memoria.h
	gcc -c -Wall memoria.c -o memoria.o

missatge : missatge.c missatge.h
	gcc -c -Wall missatge.c -o missatge.o

semafor : semafor.c semafor.h
	gcc -c -Wall semafor.c -o semafor.o

clean:
	rm -f multifil mf_mutex
