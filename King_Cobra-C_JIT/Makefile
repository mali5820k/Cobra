# This is the most elementary makefile I could make for this purpose

test: chunk.o compiler.o debug.o main.o memory.o scanner.o value.o vm.o object.o
	clang-12 chunk.o compiler.o debug.o main.o memory.o scanner.o value.o vm.o object.o -O2 -o test

chunk.o: chunk.c chunk.h
	clang-12 -c chunk.c -Wall

compiler.o: compiler.c compiler.h
	clang-12 -c compiler.c -Wall

debug.o: debug.c debug.h
	clang-12 -c debug.c -Wall

main.o: main.c
	clang-12 -c main.c -Wall

memory.o: memory.c memory.h
	clang-12 -c memory.c -Wall

scanner.o: scanner.c scanner.h
	clang-12 -c scanner.c -Wall

value.o: value.c value.h
	clang-12 -c value.c -Wall

vm.o: vm.c vm.h
	clang-12 -c vm.c -Wall

object.o: object.c object.h
	clang-12 -c object.c -Wall

clean:
	rm *.o test
