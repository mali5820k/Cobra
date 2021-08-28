# Will automatically check if you're running on windows, mac, or linux
# You can run this script or the executable included in the main project directory
import sys
import os

def compileForLinux(compiler, tags, outputFileName):
	currentFilePath = os.curdir
	os.system("cd {}".format(currentFilePath))
	cFiles = []
	
	for file in os.listdir('.'):
		if file.endswith(".c"):
			cFiles.append(file)

	cfilesString = ' '.join(cFiles)
	print("sudo {} {} {} -o {}".format(compiler, tags, cfilesString, outputFileName))
	os.system("sudo {} {} {} -o {}".format(compiler, tags, cfilesString, outputFileName))
	print("Done compiling. Program can be found in directory: {}\n".format(currentFilePath))
	exit(1)

def compileForWindows(compiler, tags, outputFileName):
	currentFilePath = os.curdir
	os.system("cd {}".format(currentFilePath))
	cFiles = []
	
	for file in os.listdir('.'):
		if file.endswith(".c"):
			cFiles.append(file)

	cfilesString = ' '.join(cFiles)
	print("{} {} {} -o {}.exe".format(compiler, tags, cfilesString, outputFileName))
	os.system("{} {} {} -o {}.exe".format(compiler, tags, cfilesString, outputFileName))
	print("Done compiling. Program can be found in directory: {}\n".format(currentFilePath))
	exit(1)
	pass

def compileForMac(compiler, tags, outputFileName):
	pass


def main():
	osPlatform = sys.platform
	compiler = ""

	gcc = "gcc"
	outputFileName = "Interpreter_Program"
	tags = "-Wall -O2"

	try:
		# For Windows Users:
		if osPlatform == 'win32':
			print("If you're having too many complications running this script, please look at pre-built binaries on the releases page on the Github Repo.\n")
			print("Please pick your compiler by typing in the number associated with either one of the options provided.\n")
			print("If you want to use a custom compiler, please choose the 'Custom compiler name' option below.\n")
			userInput = int(input("\n1	Custom compiler name\n2   Windows 10: GCC compiler\n\n=> "))
			while(True):
				if(userInput == 1):
					compiler = input("\nPlease enter your compiler's name now:\n=> ")
					print("\nYour current compiler's name is set to: '{}\n'".format(compiler))
					break
				elif userInput == 2:
					compiler = gcc
					print("\nYour current compiler's name is set to: '{}\n'".format(compiler))
					break
				else:
					print("Why? That's not an option, please try again")


			compileForWindows(compiler, tags, outputFileName)

		# For Linux Users:
		elif osPlatform == 'linux':
			print("If you'd like to change your compiler, please select either the alternatives provided\n")
			print("or choose the 'Custom compiler name' option below for custom compilers by typing in that option's associated number\n")
			userInput = int(input("\n1	Custom compiler name\n2	Linux: GCC compiler\n\n=> "))
			while(True):
				if(userInput == 1):
					compiler = input("\nPlease enter your compiler's name now:\n=> ")
					print("\nYour current compiler's name is set to: '{}\n'".format(compiler))
					break
				elif userInput == 2:
					compiler = gcc
					print("\nYour current compiler's name is set to: '{}\n'".format(compiler))
					break
				else:
					print("Why? That's not an option, please try again")
			
			compileForLinux(compiler, tags, outputFileName)

		# For Mac Users:
		elif osPlatform == 'darwin':
			print("If you'd like to change your compiler, please select either the alternatives provided\n")
			print("or choose the 'Custom compiler name' option below for custom compilers by typing in that option's associated number\n")
			userInput = int(input("I honestly have never used a mac for programming before so forgive me. For now, ctrl+c this program to quit\n"))
			while(True):
				if(userInput == 1):
					break
				elif userInput == 2:
					break
				elif userInput == 3:
					break
				elif userInput == 4:
					break
				else:
					print("Why? That's not an option, please try again")
		else:
			print("Your OS is not currently supported by this script.\n You'll have to manually build the files or use a supported OS.")
	
		compileForMac(compiler, tags, outputFileName)

	except(KeyboardInterrupt):
		print("\nEnding Make File Program\n")
		exit(1)
	return

if __name__ == "__main__": main()


