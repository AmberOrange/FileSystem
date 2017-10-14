#include "../Header/filesystem.h"
#include <sstream>


std::vector<std::string> FileSystem::splitPath(const std::string path, bool& abs) const
{
	std::stringstream stream = std::stringstream(path);
	std::string token;
	std::vector<std::string> returnVec;
	abs = false;
	while (std::getline(stream, token, '/'))
	{
		returnVec.push_back(token);
	}
	if (returnVec.size() > 0 && returnVec[0] == "")
	{
		abs = true;
		returnVec.erase(returnVec.begin());
	}

	if (returnVec.size() <= 0)
		throw "Path entry missing";

	return returnVec;
}



int FileSystem::getDirBlockIndex(const std::string& path, std::string& name, bool create) const
{
	std::stringstream stream = std::stringstream(path);
	std::string dataStr;
	dirBlock* curDir;
	char nextBlock;

	bool abs;
	std::vector<std::string> pathNames = this->splitPath(path, abs);
	if (abs)
		nextBlock = 0;
	else
		nextBlock = this->workBlock;

	int limit = 0;
	if (create)
		limit = -1;

	for(int i = 0; i < pathNames.size()+limit; i++)
	{
		dataStr = this->mMemBlockDevice.readBlock(nextBlock).toString();
		curDir = (dirBlock*)dataStr.c_str();
		bool found = false;
		
		for (int j = 0; j < curDir->nrOfElements && !found; j++)
		{
			if (curDir->elements[j].name == pathNames[i])	// If name is found and first bit of flags is set to "Directory" (0)
			{
				if ((curDir->elements[j].flags & 0x1) == 0)
				{
					nextBlock = curDir->elements[j].pointer;
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

	name = pathNames.back();

	return nextBlock;
}

int FileSystem::getDirBlockIndex(const std::string& path) const
{
	std::string junk;
	return this->getDirBlockIndex(path, junk, false);
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
	//this->occupiedList[0] = true;	// Root directory
	for (int i = 1; i < 250; i++)
		this->occupiedList[i] = false;

	dirBlock newBlock;
	memset(&newBlock, 0, sizeof(dirBlock));

	strncpy_s(newBlock.elements[0].name, ".", 2);
	newBlock.elements[0].pointer = 0;
	strncpy_s(newBlock.elements[1].name, "..", 3);
	newBlock.elements[1].pointer = 0;

	newBlock.nrOfElements = 2;

	this->setOccupiedBlock(0);

	this->mMemBlockDevice.writeBlock(0, (char*)&newBlock);

	workBlock = 0;
}

void FileSystem::createFile(const std::string& path) {
	//TODO: Implementera denna =D
}

void FileSystem::createFolder(const std::string& path)
{
	std::string name;
	int curBlock = this->getDirBlockIndex(path, name);

	int freeBlock = this->getFreeBlock();

	std::string dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	for (int i = 0; i < curDir->nrOfElements; i++)
	{
		if (strcmp(curDir->elements[i].name, name.c_str()) == 0)
			throw "Folder already exists";
	}

	dirElement* curElement = &curDir->elements[curDir->nrOfElements++];
	strncpy_s(curElement->name, name.c_str(), 14);	// Check if NULL TERMINATED
	curElement->pointer = freeBlock;
	curElement->flags = 0;	// TODO, READ/WRITE FLAGS

	this->mMemBlockDevice.writeBlock(curBlock, (char*)curDir);

	dirBlock newBlock;
	memset(&newBlock, 0, sizeof(dirBlock));

	strncpy_s(newBlock.elements[0].name, ".", 2);
	newBlock.elements[0].pointer = freeBlock;
	strncpy_s(newBlock.elements[1].name, "..", 3);
	newBlock.elements[1].pointer = curBlock;
	
	newBlock.nrOfElements = 2;

	this->setOccupiedBlock(freeBlock);

	this->mMemBlockDevice.writeBlock(freeBlock, (char*)&newBlock);
}

void FileSystem::changeDirectory(const std::string& path)
{
	this->workBlock = this->getDirBlockIndex(path);
}
