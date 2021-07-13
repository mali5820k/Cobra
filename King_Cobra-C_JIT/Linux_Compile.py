# This is a utility script for compiling the KC programming language interpretor on 
# Linux 64-bit distros, specifically Ubuntu-based Pop-OS and Manjaro distros. 
# 32-bit systems have not been tested.

# This file is meant to compile everything in
# this current directory
import os

def main():
    cFiles = []
    objectFiles = []
    ignoreFiles = []

    cFilesString = ""
    objectFilesString = ""
    outputFileName = "Interpreter_Program"
    tags = "-Wall -o2"
    compiler = "clang"
    canClear = False

    print("This program assumes that you have clang-12 installed on your pc, if not, please install that first\n")
    print("If you're using a different version of clang, please type in the compiler name to change it\n")
    print("\nYour current compiler's name is: {}\n".format(compiler))

    # Clear all object files:
    try:
        canClear = False
        while(True):
            currentFilePath = os.curdir
            os.system("cd {}".format(currentFilePath)) # For linux
            cFiles = []
            objectFiles = []
            userInput = input("\nEnter a custom compiler name (if you have one)\n   or type in one of the following options:\n    make or clear\n")
            if(userInput == "make"):
                for file in os.listdir('.'):
                    if file.endswith(".c") and file not in ignoreFiles:
                        cFiles.append(file)
                    if file.endswith(".o"):
                        objectFiles.append(file)

                cfilesString = ' '.join(cFiles)
                objectFilesString = ' '.join(objectFiles)
                print("{} {} {} {} -o {}".format(compiler, cfilesString, objectFilesString, tags, outputFileName))
                os.system("{} {} {} {} -o {}".format(compiler, cfilesString, objectFilesString, tags, outputFileName))
                canClear = True

            elif(userInput == "clear" and canClear):
                for file in os.listdir('.'):
                    if file.endswith(".o"):
                        objectFiles.append(file)
            
                objectFilesString = ' '.join(objectFiles)
                os.system("rm {} ".format(objectFiles))
                os.system("rm {}".format(outputFileName))
            else:
                compiler = userInput
                print("\nCompiler is now set to: {}\n".format(userInput))
    except (KeyboardInterrupt):
        print("\nEnding Make File Program\n")
        exit(1)
    


if __name__ == '__main__': main()