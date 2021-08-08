# I got tired of makefile errors and didn't want to spend the time to learn Cmake.
# So I just will compile using this one-liner make command :)
# Change gcc to clang-12 on Linux on line 5, or change it to simply clang if running in a powershell terminal on windows.
Interpreter_Program:
	gcc -g -Wall chunk.c compiler.c debug.c main.c memory.c scanner.c value.c vm.c object.c table.c -O0 -o Interpreter_Program

clean:
	rm Interpreter_Program