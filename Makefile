all: dph cons prod mycall
dph: dph.c
	gcc dph.c -o dph -lpthread
cons:cons.c
	gcc cons.c -o cons -lpthread -lm
prod:prod.c
	gcc prod.c -o prod -lpthread -lm
mycall:mycall.c
	gcc mycall.c -o mycall
clean:
	rm *.o dph cons prod mycall -f
