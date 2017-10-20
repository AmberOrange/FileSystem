#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "memblockdevice.h"
#include <string>
#include <fstream>

#define BLOCKSIZE 512
#define BLOCKCOUNT 250
#define NAMECAP 13

class FileSystem
{
private:
	MemBlockDevice mMemBlockDevice;

	struct dirElement {
		char flags;
		unsigned char pointer;
		char name[14];
	};
	struct dirBlock {
		char flags;		// Is this block occupied or not?
		char nrOfElements;
		dirElement elements[31];
		unsigned char next;
		char padding[13];
	};
	struct fileBlock {
		char data[511];
		unsigned char next;
	};
	struct binaryBlock {
		char data[BLOCKCOUNT*BLOCKSIZE];
		int occupiedList[BLOCKCOUNT];
	};

	// Variables ---
	int occupiedList[BLOCKCOUNT];
	//std::string workDir;
	int workBlock;
	//dirBlock* pWorkDir;

	// Private Functions ---
	std::vector<std::string> splitPath(const std::string path, bool& abs) const;
	int getDirBlockIndex(const std::string& path, std::string& name, bool create = true) const;
	int getDirBlockIndex(const std::string& path) const;
	int getFreeBlock() const;
	void setOccupiedBlock(const int blockNr);
	std::stringstream recursivePath(const int blockNr, const int childNr);
	void recursiveRemove(const int blockNr);
	int getBlockNr(const int blockNr, const std::string& name, const char blockType = 2, const char permission = 0, int* flags = nullptr) const;	// 0 = Directory; 1 = File; > 2 = Doesn't matter
	void addDirElement(int blockNr, dirElement* element);
	//int getElementIndex();
	int createDirBlock(bool expand = false);
	void recursiveIncreaseDirSize(const int blockNr, const int size);

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

	void writeFile(const std::string& path, const std::string& text);

	void removeFile(const std::string& path);

	void copyFile(const std::string& source, const std::string& target);

	void createImage(const std::string& path);

	void restoreImage(const std::string& path);

	void increaseDirSize(const std::string& path, const int size);

	void chmod(const std::string& path, char permission);

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
