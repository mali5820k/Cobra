# This is the most elementary makefile I could make for this purpose

test: chunk.o compiler.o debug.o main.o memory.o scanner.o value.o vm.o
	clang-11 chunk.o compiler.o debug.o main.o memory.o scanner.o value.o vm.o -o3 test

chunk.o: chunk.c chunk.h
	clang-11 -c chunk.c -Wall

compiler.o: compiler.c compiler.h
	clang-11 -c compiler.c -Wall

debug.o: debug.c debug.h
	clang-11 -c debug.c -Wall

main.o: main.c
	clang-11 -c main.c -Wall

memory.o: memory.c memory.h
	clang-11 -c memory.c -Wall

scanner.o: scanner.c scanner.h
	clang-11 -c scanner.c -Wall

value.o: value.c value.h
	clang-11 -c value.c -Wall

vm.o: vm.c vm.h
	clang-11 -c vm.c -Wall

clean:
	rm *.o test
