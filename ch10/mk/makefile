main: main.o ctty.o
	gcc -o main main.o ctty.o

ctty.o: ctty.c ctty.h
	gcc -c ctty.c

main.o: main.c ctty.h
	gcc -c main.c

clean:
	rm main.o ctty.o main
