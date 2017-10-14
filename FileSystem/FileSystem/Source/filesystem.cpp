#include "../Header/filesystem.h"
#include <sstream>


int FileSystem::getDirBlockIndex(const std::string& path) const
{
	std::stringstream stream = std::stringstream(path);
	std::string token, dataStr;
	dirBlock* curDir;
	char nextBlock;

	if (std::getline(stream, token, '/'))
	{
		if (token == "")
		{
			nextBlock = 0;
		}
		else {
			dataStr = this->workDir;
			curDir = (dirBlock*)dataStr.c_str();
			bool found = false;
			for (int i = 0; i < curDir->nrOfElements && !found; i++)
			{
				if (curDir->elements[i].name == token)	// If name is found and first bit of flags is set to "Directory" (0)
				{
					if ((curDir->elements[i].flags & 0x1) == 0)
					{
						nextBlock = curDir->elements[i].pointer;
						found = true;
					}
					else {
						// ERROR, requested folder is actually a file ¯\_("/)_/¯
						throw "Requested folder is actually a file";
					}
				}
			}
			if (!found)
			{
				// ERROR, Couldn't find requested file
				throw "Couldn't find requested folder";
			}
		}
	}

	while (std::getline(stream,token,'/'))
	{
		dataStr = this->mMemBlockDevice.readBlock(nextBlock).toString();
		curDir = (dirBlock*)dataStr.c_str();
		bool found = false;
		
		for (int i = 0; i < curDir->nrOfElements && !found; i++)
		{
			if (curDir->elements[i].name == token)	// If name is found and first bit of flags is set to "Directory" (0)
			{
				if ((curDir->elements[i].flags & 0x1) == 0)
				{
					nextBlock = curDir->elements[i].pointer;
					found = true;
				}
				else {
					// ERROR, requested folder is actually a file ¯\_("/)_/¯
					throw "Requested folder is actually a file";
				}
			}
		}
		if (!found)
		{
			// ERROR, Couldn't find requested folder
			throw "Couldn't find requested folder";
		}
	}

	return nextBlock;
}

FileSystem::FileSystem() {

}

FileSystem::~FileSystem() {

}

/* Please insert your code */

void FileSystem::format() {
	mMemBlockDevice.reset();
}

void FileSystem::createFile(const std::string& path) {
	//TODO: Implementera denna =D
}
