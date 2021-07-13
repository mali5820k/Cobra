# King Cobra
A programming language inspired from Python's simplicity and C/C++'s speed.
The language itself is going to be compiler-based with focus on portability via a VM (low-level Virtual Machine). Once a proof of concept is made with a 
functional VM, the focus will be to implement KC with LLVM, JVM, or to keep it as a standalone project for experience.

This is an ongoing project that is based off of interest in compiled languages.
The book I am actively reading to accomplish this project is "Crafting Interpretors" By Bob Nystrom which he has made publicly available at: https://craftinginterpreters.com/


#IMPORTANT NOTES TO GET THE INTERPRETER UP AND RUNNING

For Windows 10 users:
To run the interpreter, you need to first install Clang from the pre-built binaries for Windows 10 on Clang's website. Then you can run the python script in powershell, cmd, or a terminal window of your choice to produce the Interpreter_Program.exe program. Finally, you can type away in the program once you have launched it.

For Mac OS users:
I haven't tested anything with Macs as I don't own one. I do know that Macs have Clang pre-installed and it's closest in terms of functionality to Linux as it's Unix-based, so you might be able to run the included clang make file or the python compile file for Linux and have the program compile to an executable binary.

For Linux users:
Since this project is natively coded on Linux, you can simply run the make file for either GCC or Clang. You will need to install Clang first if you go choose to use the Clang make file. The python compile script for Linux will also work as long as you have python 3 and Clang installed. Once the program binary is compiled, you simply need to run the file in a terminal with ./Interpreter_Program .

Keep in mind that this project is being developed solely by me, so bugs are to be expected and with time, the language will take on its own identy that will allow it to be more different than the Lox language implementation described in "Crafting Interpretors" By Bob Nystrom.
