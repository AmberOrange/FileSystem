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
	bool occupiedList[250];
	std::string workDir;
	dirBlock* pWorkDir;

	// Private Functions ---
	int getDirBlockIndex(const std::string& path, std::string& name) const;
	int getFreeBlock() const;
	void setOccupiedBlock(const int blockNr);

public:
    FileSystem();
    ~FileSystem();

    /* These API functions need to be implemented
	   You are free to specify parameter lists and return values
    */

    /* Reset the filesystem */
    void format();

    /* This function creates a file in the filesystem */
    void createFile(const std::string& path);

    /* Creates a folder in the filesystem */
	void createFolder(const std::string& path);

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
