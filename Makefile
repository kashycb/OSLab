kdr60Shell: myshell.o
	gcc myshell.o -o kdr60Shell

myshell.o: myshell.c
	gcc -c myshell.c

clean:
	rm *.o kdr60Shell

all: run

run: kdr60Shell
	ulimit -u
	./kdr60Shell

.PHONY: all run
