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
	dirElement elements[31];
	char next;
	char padding[15];
    };
    struct fileBlock {
	char data[511];
	char next;
    };
    // Here you can add your own data structures
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
    // createFolderi(...);

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
