#include "../Header/filesystem.h"
#include <sstream>


int FileSystem::getDirBlockIndex(const std::string& path, std::string& name) const
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

	std::getline(stream, token);	// If this doesn't work, use the Marcus Algorith 
	name = token;					// std::string hello[hello.size()] = '/';

	return nextBlock;
}

int FileSystem::getFreeBlock() const
{
	for (int i = 0; i < 250; i++)
	{
		if (!this->occupiedList[i])
			return i;
	}
	throw "No available blocks left";
}

void FileSystem::setOccupiedBlock(const int blockNr)
{
	if (blockNr < 0 || blockNr >= 250)
		throw "Block out of range";
	this->occupiedList[blockNr] = true;
}

FileSystem::FileSystem() {
}

FileSystem::~FileSystem() {

}

/* Please insert your code */

void FileSystem::format() {
	mMemBlockDevice.reset();
	this->occupiedList[0] = true;	// Root directory
	for (int i = 1; i < 250; i++)
		this->occupiedList[i] = false;
}

void FileSystem::createFile(const std::string& path) {
	//TODO: Implementera denna =D
}

void FileSystem::createFolder(const std::string& path)
{
	std::string name;
	int curBlock = this->getDirBlockIndex(path, name);

	std::string dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	curDir->elements[curDir->nrOfElements++] =
}
