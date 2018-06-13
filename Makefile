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

# auxiliars per a les curses
winsuport : winsuport.c winsuport.h
	gcc -c -Wall winsuport.c

winsuport2 : winsuport2.c winsuport2.h
	gcc -Wall -c winsuport2.c -o winsuport2.o

pilota3 : pilota3.c winsuport2.c winsuport2.h
	gcc -Wall -g pilota3.c winsuport2.o memoria.o -o pilota3 -lcurses

memoria : memoria.c memoria.h
	gcc -c -Wall memoria.c -o memoria.o 

clean:
	rm -f multifil mf_mutex
