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
		if (this->occupiedList[i] == -1)
			return i;
	}
	throw "No available blocks left";
}

void FileSystem::setOccupiedBlock(const int blockNr)
{
	if (blockNr < 0 || blockNr >= 250)
		throw "Block out of range";
	this->occupiedList[blockNr] = 0;
}

std::stringstream FileSystem::recursivePath(const int blockNr, const int childNr)
{
	std::stringstream ss;
	if (childNr == 0)
	{
		ss << "/";
		return ss;
	} 

	std::string dataStr = this->mMemBlockDevice.readBlock(blockNr).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	bool found = false;
	int parentNr = -1;
	for (int i = 0; i < curDir->nrOfElements && !found; i++)
	{
		if (strcmp(curDir->elements[i].name, "..") == 0)
		{
			found = true;
			parentNr = curDir->elements[i].pointer;		// Can be fixed with index 1 :)
		}
	}
	if (!found)
		throw "Lost child";

	if (childNr == blockNr)
	{
		return this->recursivePath(parentNr, blockNr);
	}

	for (int i = 0; i < curDir->nrOfElements; i++)
	{
		if (curDir->elements[i].pointer == childNr)
		{
			ss << this->recursivePath(parentNr, blockNr).str() << curDir->elements[i].name << "/";
			return ss;
		}
	}
}

void FileSystem::recursiveRemove(const int blockNr)
{
	std::string dataStr = this->mMemBlockDevice.readBlock(blockNr).toString();
	fileBlock* curDir = (fileBlock*)dataStr.c_str();

	if (curDir->next == 0)
		return;
	else
	{
		this->recursiveRemove(curDir->next);
		this->occupiedList[curDir->next] = -1;
	}
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
		this->occupiedList[i] = -1;

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

void FileSystem::createFile(const std::string& path, const std::string& text) {
	std::string name;
	int curBlock = this->getDirBlockIndex(path, name);

	int freeBlock = this->getFreeBlock();

	std::string dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	for (int i = 0; i < curDir->nrOfElements; i++)
	{
		if (strcmp(curDir->elements[i].name, name.c_str()) == 0)
			throw "Name already taken";
	}

	dirElement* curElement = &curDir->elements[curDir->nrOfElements++];
	strncpy_s(curElement->name, name.c_str(), 14);	// Check if NULL TERMINATED
	curElement->pointer = freeBlock;
	curElement->flags = 1;	// TODO, READ/WRITE FLAGS

	this->mMemBlockDevice.writeBlock(curBlock, (char*)curDir);

	this->occupiedList[freeBlock] = text.size();

	fileBlock newBlock;

	//std::stringstream ss(text);
	char* token = new char[text.size()+1];
	memcpy_s(token, text.size(), text.c_str(), text.size());
	token[text.size()] = '\0';
	int nextBlock = 0;
	int i = 0;
	while (i * 510 < text.size())
	{
		memset(&newBlock, 0, sizeof(fileBlock));
		
		memcpy_s(newBlock.data, 510, token + 510*i, 510);
		i++;
		if (i * 510 < text.size())
		{
			nextBlock = this->getFreeBlock();
			newBlock.next = nextBlock;
			this->occupiedList[nextBlock] = 0;
		}
		this->mMemBlockDevice.writeBlock(freeBlock, (char*)&newBlock);
		freeBlock = nextBlock;
	}
	delete[] token;
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
			throw "Name already taken";
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

std::string FileSystem::printWorkDirectory()
{
	return this->recursivePath(this->workBlock, this->workBlock).str();
}

void FileSystem::listDir(const std::string& path)
{
	int curNr;
	if (path == "")
		curNr = this->workBlock;
	else
		curNr = this->getDirBlockIndex(path);

	//std::string dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
	//dirBlock* curDir = (dirBlock*)dataStr.c_str();
}

std::string FileSystem::readFile(const std::string& path)
{
	std::string name;
	int curBlock = this->getDirBlockIndex(path, name);

	std::string dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	bool found = false;

	for (int i = 0; i < curDir->nrOfElements && !found; i++)
	{
		if (strcmp(curDir->elements[i].name, name.c_str()) == 0)
		{
			if (curDir->elements[i].flags == 0)
				throw "Requested file is actually a folder";
			found = true;
			curBlock = curDir->elements[i].pointer;
		}
	}
	if (!found)
		throw "File not found";

	bool readDone = false;
	fileBlock* curFile;
	std::stringstream ss;
	std::string token;
	while (!readDone)
	{
		dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
		curFile = (fileBlock*)dataStr.c_str();
		//ss.write(curFile->data, 510);
		token = std::string(curFile->data, std::find(curFile->data, curFile->data + 510, '\0'));
		ss << token;
		if (curFile->next == 0)
			readDone = true;
		else
			curBlock = curFile->next;
	}

	return ss.str();
}

void FileSystem::removeFile(const std::string& path)
{
	std::string name;
	int curBlock = this->getDirBlockIndex(path, name);
	int fileBlock;

	std::string dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	bool found = false;

	for (int i = 0; i < curDir->nrOfElements && !found; i++)
	{
		if (strcmp(curDir->elements[i].name, name.c_str()) == 0)
		{
			if (curDir->elements[i].flags == 0)
				throw "Requested file is actually a folder";
			found = true;
			fileBlock = curDir->elements[i].pointer;
			//curDir->nrOfElements--;
			curDir->elements[i] = curDir->elements[--curDir->nrOfElements];
			memset(&curDir->elements[curDir->nrOfElements], 0, sizeof(dirElement));
			this->mMemBlockDevice.writeBlock(curBlock, (char*)curDir);
		}
	}
	if (!found)
		throw "File not found";

	this->recursiveRemove(fileBlock);
	this->occupiedList[fileBlock] = -1;
}
