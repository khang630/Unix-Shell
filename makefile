output: major2.o path.o history.o alias.o cd.o
	gcc -o output major2.o path.o history.o alias.o cd.o

major2.o: major2.c
	gcc -c major2.c

path.o: path.c
	gcc -c path.c

history.o: history.c
	gcc -c history.c

alias.o: alias.c
	gcc -c alias.c
	
cd.o: cd.c
	gcc -c cd.c

clean:
	rm *.o output
