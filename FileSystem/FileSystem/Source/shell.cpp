#include <iostream>
#include <sstream>
#include "../Header/filesystem.h"
#include <string>

const int MAXCOMMANDS = 8;
const int NUMAVAILABLECOMMANDS = 18;

FileSystem fileSystem;

std::string availableCommands[NUMAVAILABLECOMMANDS] = {
    "quit","format","ls","create","cat","createImage","restoreImage",
    "rm","cp","append","mv","mkdir","cd","pwd","help","clear","chmod", "debug"
};

/* Takes usercommand from input and returns number of commands, commands are stored in strArr[] */
int parseCommandString(const std::string &userCommand, std::string strArr[]);
int findCommand(std::string &command);
bool quit();
std::string help();

void format();
void ls(const std::string& path);
void create(const std::string& path);
void cat(const std::string& path);
void createImage(const std::string& path);
void restoreImage(const std::string& path);
void rm(const std::string& path);
void cp(const std::string& source, const std::string& target);
void append(const std::string& source, const std::string& target);
void mv(const std::string& source, const std::string& target);
void mkdir(const std::string& path);
void cd(const std::string& path);
void chmod(const std::string& path, const std::string& permission);
std::string pwd();

void debug();

/* More functions ... */

int main(void) {

	std::string userCommand, commandArr[MAXCOMMANDS];
	std::string user = "user@DV1492";    // Change this if you want another user to be displayed
	std::string currentDir = "/";

    bool bRun = true;

    do {
		currentDir = pwd();    // current directory, used for output
        std::cout << user << ":" << currentDir << "$ ";
        getline(std::cin, userCommand);

        int nrOfCommands = parseCommandString(userCommand, commandArr);
        if (nrOfCommands > 0) {

            int cIndex = findCommand(commandArr[0]);
			try {
				switch(cIndex) {

				case 0: //quit
					bRun = quit();                
					break;
				case 1: // format
					format();
					break;
				case 2: // ls
					ls(commandArr[1]);
					break;
				case 3: // create
					create(commandArr[1]);
					break;
				case 4: // cat
					cat(commandArr[1]);
					break;
				case 5: // createImage
					createImage(commandArr[1]);
					break;
				case 6: // restoreImage
					restoreImage(commandArr[1]);
					break;
				case 7: // rm
					rm(commandArr[1]);
					break;
				case 8: // cp
					cp(commandArr[1], commandArr[2]);
					break;
				case 9: // append
					append(commandArr[1], commandArr[2]);
					break;
				case 10: // mv
					mv(commandArr[1], commandArr[2]);
					break;
				case 11: // mkdir
					mkdir(commandArr[1]);
					break;
				case 12: // cd
					cd(commandArr[1]);
					break;
				case 13: // pwd
					std::cout << pwd() << std::endl;
					break;
				case 14: // help
					std::cout << help() << std::endl;
					break;
				case 15: // clear
					std::cout << "\033[2J\033[;H";	// Clear ONLY WORKS ON LINUX
					break;
				case 16: // chmod
					chmod(commandArr[1], commandArr[2]);
					break;
				case 17: // debug
					debug();
					break;
				default:
					std::cout << "Unknown command: " << commandArr[0] << std::endl;
				}
			}
			catch (char const* e)
			{
				std::cout << "Error: " << e << std::endl;
			}
        }
		for (int i = 0; i < MAXCOMMANDS; i++)
			commandArr[i] = "";
    } while (bRun == true);

    return 0;
}

int parseCommandString(const std::string &userCommand, std::string strArr[]) {
    std::stringstream ssin(userCommand);
    int counter = 0;
    while (ssin.good() && counter < MAXCOMMANDS) {
        ssin >> strArr[counter];
        counter++;
    }
    if (strArr[0] == "") {
        counter = 0;
    }
    return counter;
}
int findCommand(std::string &command) {
    int index = -1;
    for (int i = 0; i < NUMAVAILABLECOMMANDS && index == -1; ++i) {
        if (command == availableCommands[i]) {
            index = i;
        }
    }
    return index;
}

bool quit() {
	std::cout << "Exiting\n";
	return false;
}

std::string help() {
    std::string helpStr;
    helpStr += "OSD Disk Tool .oO Help Screen Oo.\n";
    helpStr += "-----------------------------------------------------------------------------------\n" ;
    helpStr += "* quit:                             Quit OSD Disk Tool\n";
    helpStr += "* format;                           Formats disk\n";
    helpStr += "* ls     <path>:                    Lists contents of <path>.\n";
    helpStr += "* create <path>:                    Creates a file and stores contents in <path>\n";
    helpStr += "* cat    <path>:                    Dumps contents of <file>.\n";
    helpStr += "* createImage  <real-file>:         Saves disk to <real-file>\n";
    helpStr += "* restoreImage <real-file>:         Reads <real-file> onto disk\n";
    helpStr += "* rm     <file>:                    Removes <file>\n";
    helpStr += "* cp     <source> <destination>:    Copy <source> to <destination>\n";
    helpStr += "* append <source> <destination>:    Appends contents of <source> to <destination>\n";
    helpStr += "* mv     <old-file> <new-file>:     Renames <old-file> to <new-file>\n";
    helpStr += "* mkdir  <directory>:               Creates a new directory called <directory>\n";
    helpStr += "* cd     <directory>:               Changes current working directory to <directory>\n";
    helpStr += "* pwd:                              Get current working directory\n";
    helpStr += "* help:                             Prints this help screen\n";
    return helpStr;
}

/* Insert code for your shell functions and call them from the switch-case */
void format()
{
	fileSystem.format();
}

void ls(const std::string& path)
{
	std::cout << fileSystem.listDir(path);
}

void create(const std::string& path)
{
	std::string text;
	std::cout << "Enter text: " << std::endl;
	std::getline(std::cin, text);
	fileSystem.createFile(path, text);
}

void cat(const std::string& path)
{
	std::cout << fileSystem.readFile(path) << std::endl;
}

void createImage(const std::string& path)
{
	fileSystem.createImage(path);
}

void restoreImage(const std::string& path)
{
	fileSystem.restoreImage(path);
}

void rm(const std::string& path)
{
	fileSystem.removeFile(path);
}
void cp(const std::string& source, const std::string& target)
{
	fileSystem.copyFile(source, target);
}

void append(const std::string& source, const std::string& target)
{
	std::stringstream ss;
	ss << fileSystem.readFile(source) << fileSystem.readFile(target);
	fileSystem.writeFile(source, ss.str());
}

void mv(const std::string& source, const std::string& target)
{
	fileSystem.copyFile(source, target);
	fileSystem.removeFile(source);
}

void mkdir(const std::string& path)
{
	fileSystem.createFolder(path);
}

void cd(const std::string& path)
{
	fileSystem.changeDirectory(path);
}

std::string pwd()
{
	return fileSystem.printWorkDirectory();
}

void chmod(const std::string& path, const std::string& permission)
{
	fileSystem.chmod(path, (char)std::stoi(permission.c_str()));
}

void debug()
{
	std::string path = "";
	std::string text = "Wubba Lubba Dub Dub!";
	for(int i = 1; i < 243; i++)
	{
		path = "file" + std::to_string(i);
		try
		{
			fileSystem.createFile(path, text);	

		}
		catch (std::out_of_range e)
		{
			std::cout << e.what() << ", AT: " << i << std::endl;
		}
	}

	for (int i = 1; i < 242; i++)
	{
		path = "file" + std::to_string(i);
		try
		{
			fileSystem.removeFile(path);

		}
		catch (char const* e)
		{
			std::cout << e << ", AT: " << i << std::endl;
		}
	}
}
