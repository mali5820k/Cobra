# This is the most elementary makefile I could make for this purpose
test: chunk.o debug.o main.o memory.o value.o vm.o main.o
	gcc chunk.o debug.o main.o memory.o value.o -o test

chunk.o: chunk.c chunk.h
	gcc -c chunk.c -Wall

debug.o: debug.c debug.h
	gcc -c debug.c -Wall

main.o: main.c
	gcc -c main.c -Wall

memory.o: memory.c memory.h
	gcc -c memory.c -Wall

value.o: value.c value.h
	gcc -c value.c -Wall

vm.o: vm.c vm.h
	gcc -c vm.c -Wall

clean:
	rm *.o test
