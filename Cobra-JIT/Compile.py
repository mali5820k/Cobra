
# This file is meant to compile everything in
# this current directory
import os

cFiles = []
objectFiles = []
ignoreFiles = ["CobraCompiler.c"]

cFilesString = ""
objectFilesString = ""
outputFileName = "test"
tags = ""

canClear = False

def main():

    # Clear all object files:
    try:
        while(True):
            os.system("cd ~/Desktop/Cobra-Programming-Language/Cobra-JIT/")
            cFiles = []
            objectFiles = []
            userInput = input("\nmake or clear\n")
            if(userInput == "make"):
                for file in os.listdir('.'):
                    if file.endswith(".c") and file not in ignoreFiles:
                        cFiles.append(file)
                    if file.endswith(".o"):
                        objectFiles.append(file)
                #print("\n")
                #print(cFiles)
                #print("\n")
                #print(objectFiles)

                cfilesString = ' '.join(cFiles)
                #print(cfilesString + "\n")
                objectFilesString = ' '.join(objectFiles)
                #print(objectFilesString + "\n")
                os.system("gcc {} {} {} -o {}".format(cfilesString, objectFilesString, tags, outputFileName))
                canClear = True

            elif(userInput == "clear" and canClear):
                for file in os.listdir('.'):
                    if file.endswith(".o"):
                        objectFiles.append(file)
            
                objectFilesString = ' '.join(objectFiles)
                os.system("rm -rf {} ".format(objectFiles))
                os.system("rm {}".format(outputFileName))
    except (KeyboardInterrupt):
        print("\nEnding Make File Program\n")
        exit(1)
    


if __name__ == '__main__': main()
