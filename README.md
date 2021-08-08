# King Cobra
### Project Goals/Focus:
  - A programming language inspired from Python's simplicity and C/C++'s speed.
  - The language itself is going to be compiler-based with focus on portability via a VM (low-level Virtual Machine). Once a proof of concept is made with a functional VM, the focus will be to implement KC entirely with the help of "Crafting Interpreters" by Robert (Bob) Nystrom, then to remake the language from scratch using my acquired knowledge from the book, or to keep this entire thing as a standalone project for experience.

Again, this is an ongoing project that is based off of interest in compiled languages.
The book I am actively reading to accomplish this project is "Crafting Interpreters" by Bob Nystrom which he has made publicly available at: https://craftinginterpreters.com/


## Important Notes for compiling and running the interpreter

### For Windows 10 users:
- If you want to skip the manual build process, you can grab a pre-built executable from the [releases](https://github.com/mali5820k/KC-Programming-Language/releases) page. Keep in mind that this may not be the most current version.
- To run the interpreter, you need to first install GCC for Windows 10 and if you don't want to use the build program executable, acquire Make for Windows as well.
- If you don't want to use Make or the program executable, you can run the python build script in powershell, cmd, or a terminal window of your choice to produce the Interpreter_Program.exe program. Run the terminal of your choice as an administrator. 
- Finally, you can type away in the program once you have launched it.

### For Mac OS users:
- I haven't tested anything with Macs as I don't own one. I do know that Macs have Clang pre-installed and it's closest in terms of functionality to Linux as it's Unix-based, so you might be able to run the included make file after some tweaks or the python compile file and have the program compile to an executable binary.
- I still recommend Mac users try to acquire GCC in some way as Clang has resulted in strange issues from build to build that weren't present in GCC.

### For Linux users:
- Since this project is natively coded on Linux, you can simply run the make file for GCC. 
- The python build script will also work as long as you have python 3 installed or run the included executable (if there is one included at the time).
- Once the program binary is compiled, you simply need to run the file in a terminal with
  ```./Interpreter_Program```

## P.S.
Keep in mind that I'm only one person and although Nystrom's book is guiding me through this project, functionality that I want to add is going to take time; so bugs are to be expected and with time, the language will diverge and become more of its own thing-if you will-than the Lox programming language implementation described in "Crafting Interpreters" by Bob Nystrom.

Also, a syntax and feature guide will be posted here as well as the releases page with a downloadable PDF later. I have several more features to include before that will be relevant. 

Lastly, it is unclear to me as to how to use the MIT license in terms of the copyright of code. To be safe, I have included the original copyright notice of Bob Nystrom (written as Robert Nystrom in his original license) from his MIT license for MIT licensed code in my MIT license file as well, to ensure all legal bases are covered for the MIT licensed code.

Thank you for checking out my repo!
