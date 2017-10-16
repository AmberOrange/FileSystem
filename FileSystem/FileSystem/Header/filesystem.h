#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "memblockdevice.h"
#include <string>

class FileSystem
{
private:
	MemBlockDevice mMemBlockDevice;

	struct dirElement {
		char flags;
		char pointer;
		char name[14];
	};
	struct dirBlock {
		char flags;		// Is this block occupied or not?
		char nrOfElements;
		dirElement elements[31];
		char next;
		char padding[13];
	};
	struct fileBlock {
		char flags;		// Is this block occupied or not?
		char data[510];
		char next;
	};

	// Variables ---
	int occupiedList[250];
	//std::string workDir;
	int workBlock;
	//dirBlock* pWorkDir;

	// Private Functions ---
	std::vector<std::string> splitPath(const std::string path, bool& abs) const;			// TODO: Ersätt en massa med GetBlockNr istället :)
	int getDirBlockIndex(const std::string& path, std::string& name, bool create = true) const;
	int getDirBlockIndex(const std::string& path) const;
	int getFreeBlock() const;
	void setOccupiedBlock(const int blockNr);
	std::stringstream recursivePath(const int blockNr, const int childNr);
	void recursiveRemove(const int blockNr);
	int getBlockNr(const int blockNr, const std::string& name);
	void addDirElement(const int blockNr, dirElement* element);
	//int getElementIndex();
	int createDirBlock();

public:
    FileSystem();
    ~FileSystem();

    /* These API functions need to be implemented
	   You are free to specify parameter lists and return values
    */

    /* Reset the filesystem */
    void format();

    /* This function creates a file in the filesystem */
    void createFile(const std::string& path, const std::string& text);

    /* Creates a folder in the filesystem */
	void createFolder(const std::string& path);

	void changeDirectory(const std::string& path);

	std::string printWorkDirectory();

	std::string listDir(const std::string& path);

	std::string readFile(const std::string& path);

	void removeFile(const std::string& path);

	void copyFile(const std::string& source, const std::string& target);

    /* Removes a file in the filesystem */
    // removeFile(...);

    /* Removes a folder in the filesystem */
    // removeFolder(...);

    /* Function will move the current location to a specified location in the filesystem */
    // goToFolder(...);

    /* This function will get all the files and folders in the specified folder */
    // listDir(...);

    /* Add your own member-functions if needed */
};

#endif // FILESYSTEM_H
