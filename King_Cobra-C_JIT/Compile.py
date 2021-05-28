
# This file is meant to compile everything in
# this current directory
import os

cFiles = []
objectFiles = []
ignoreFiles = ["CobraCompiler.c"]

cFilesString = ""
objectFilesString = ""
outputFileName = "test"
tags = "-Wall"

canClear = False

# Future, make a class for each make object and then proceed with individual make files for larger projects.
# Possibly start off with a template make class and then inherit any subsequent make classes from that class.
#class Make():
#    name = "Make Parent"
#
#    __init__:
#        print("{} \n".format(name))
    
#class Make2(Make):
    
#    __init__:
#        super.__init__

def main():

    # Clear all object files:
    try:
        canClear = False
        while(True):
            currentFilePath = os.curdir
            os.system("cd {}".format(currentFilePath)) # For linux
            cFiles = []
            objectFiles = []
            userInput = input("\nmake or clear\n")
            if(userInput == "make"):
                for file in os.listdir('.'):
                    if file.endswith(".c") and file not in ignoreFiles:
                        cFiles.append(file)
                    if file.endswith(".o"):
                        objectFiles.append(file)

                cfilesString = ' '.join(cFiles)
                objectFilesString = ' '.join(objectFiles)
                os.system("gcc {} {} {} -o {}".format(cfilesString, objectFilesString, tags, outputFileName))
                canClear = True

            elif(userInput == "clear" and canClear):
                for file in os.listdir('.'):
                    if file.endswith(".o"):
                        objectFiles.append(file)
            
                objectFilesString = ' '.join(objectFiles)
                os.system("rm {} ".format(objectFiles))
                os.system("rm {}".format(outputFileName))
    except (KeyboardInterrupt):
        print("\nEnding Make File Program\n")
        exit(1)
    


if __name__ == '__main__': main()
