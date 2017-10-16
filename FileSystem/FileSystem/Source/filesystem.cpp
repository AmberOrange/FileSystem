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
		//dataStr = this->mMemBlockDevice.readBlock(nextBlock).toString();
		//curDir = (dirBlock*)dataStr.c_str();
		//bool found = false;
		//
		//for (int j = 0; j < curDir->nrOfElements && !found; j++)
		//{
		//	if (curDir->elements[j].name == pathNames[i])	// If name is found and first bit of flags is set to "Directory" (0)
		//	{
		//		if ((curDir->elements[j].flags & 0x1) == 0)
		//		{
		//			nextBlock = curDir->elements[j].pointer;
		//			found = true;
		//		}
		//		else {
		//			// ERROR, requested folder is actually a file ¯\_("/)_/¯
		//			throw "Requested folder is actually a file";
		//		}
		//	}
		//}
		//if (!found)
		//{
		//	// ERROR, Couldn't find requested folder
		//	throw "Couldn't find requested folder";
		//}

		nextBlock = this->getBlockNr(nextBlock, pathNames[i], 0);
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
	for (int i = 0; i < BLOCKCOUNT; i++)
	{
		if (this->occupiedList[i] == -1)
			return i;
	}
	throw "No available blocks left";
}

void FileSystem::setOccupiedBlock(const int blockNr)
{
	if (blockNr < 0 || blockNr >= BLOCKCOUNT)
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

int FileSystem::getBlockNr(const int dirBlockNr, const std::string& name, const char blockType, const char permission) const
{
	std::string dataStr = this->mMemBlockDevice.readBlock(dirBlockNr).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	bool found = false;
	int index;
	do
	{
		for (int i = 0; i < curDir->nrOfElements && !found; i++)
		{
			if (strcmp(curDir->elements[i].name, name.c_str()) == 0)
			{
				if (blockType == 0 && (curDir->elements[i].flags & 1) == 1)
					throw "The folder is actually a file";
				else if (blockType == 1 && (curDir->elements[i].flags & 1) == 0)
					throw "The file is actually a folder";

				if (blockType == 1)
				{
					if (permission == 0 && (curDir->elements[i].flags & 2) == 0)
						throw "You don't have permission to read";
					else if (permission == 1 && (curDir->elements[i].flags & 4) == 0)
						throw "You don't have permission to write";
				}

				found = true;
				index = curDir->elements[i].pointer;
			}
		}
		if (curDir->next != 0)
		{
			dataStr = this->mMemBlockDevice.readBlock(curDir->next).toString();
			curDir = (dirBlock*)dataStr.c_str();
		}
	} while (curDir->next != 0);

	if (!found)
		throw "Couldn't find the file/folder";

	return index;
}

void FileSystem::addDirElement(const int blockNr, dirElement* element)
{
	std::string dataStr = this->mMemBlockDevice.readBlock(blockNr).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	if (curDir->nrOfElements > 31)
	{
		curDir->next = this->createDirBlock();
		dataStr = this->mMemBlockDevice.readBlock(curDir->next).toString();
		curDir = (dirBlock*)dataStr.c_str();
	}

	curDir->elements[curDir->nrOfElements++] = *element;

	this->mMemBlockDevice.writeBlock(blockNr, (char*)curDir);
}

int FileSystem::createDirBlock()
{
	dirBlock newBlock;
	memset(&newBlock, 0, sizeof(dirBlock));
	int blockNr = this->getFreeBlock();

	strncpy_s(newBlock.elements[0].name, ".", 2);
	newBlock.elements[0].pointer = 0;
	strncpy_s(newBlock.elements[1].name, "..", 3);
	newBlock.elements[1].pointer = 0;

	newBlock.nrOfElements = 2;

	this->setOccupiedBlock(blockNr);

	this->mMemBlockDevice.writeBlock(blockNr, (char*)&newBlock);
	return blockNr;
}

void FileSystem::recursiveIncreaseDirSize(const int blockNr, const int size)
{
	std::string dirStr = this->mMemBlockDevice.readBlock(blockNr).toString();
	dirBlock* curDir = (dirBlock*)dirStr.c_str();

	if (blockNr != 0)														// if not root
		this->recursiveIncreaseDirSize(curDir->elements[1].pointer, size);	// Call recursive on parent directory

	this->occupiedList[blockNr] += size;
}

FileSystem::FileSystem() {
}

FileSystem::~FileSystem() {

}

/* Please insert your code */

void FileSystem::format() {
	mMemBlockDevice.reset();
	//this->occupiedList[0] = true;	// Root directory
	for (int i = 0; i < BLOCKCOUNT; i++)
		this->occupiedList[i] = -1;

	//dirBlock newBlock;
	//memset(&newBlock, 0, sizeof(dirBlock));

	//strncpy_s(newBlock.elements[0].name, ".", 2);
	//newBlock.elements[0].pointer = 0;
	//strncpy_s(newBlock.elements[1].name, "..", 3);
	//newBlock.elements[1].pointer = 0;

	//newBlock.nrOfElements = 2;

	//this->setOccupiedBlock(0);

	//this->mMemBlockDevice.writeBlock(0, (char*)&newBlock);

	this->createDirBlock();

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
	strncpy_s(curElement->name, name.c_str(), NAMECAP);	// Check if NULL TERMINATED
	curElement->pointer = freeBlock;
	curElement->flags = 7;	// FLAGS: 11100000 (FILE,READ,WRITE)

	this->mMemBlockDevice.writeBlock(curBlock, (char*)curDir);
	this->occupiedList[freeBlock] = text.size();
	this->increaseDirSize(path, text.size());

	fileBlock newBlock;

	//std::stringstream ss(text);
	char* token = new char[text.size()+1];
	memcpy_s(token, text.size(), text.c_str(), text.size());
	token[text.size()] = '\0';
	int nextBlock = 0;
	int i = 0;
	while (i * 511 < text.size())
	{
		memset(&newBlock, 0, sizeof(fileBlock));
		
		memcpy_s(newBlock.data, 511, token + 511*i, 511);
		i++;
		if (i * 511 < text.size())
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
	strncpy_s(curElement->name, name.c_str(), NAMECAP);	// Check if NULL TERMINATED
	curElement->pointer = freeBlock;
	curElement->flags = 6;	// FLAGS: 01100000 (FOLDER,READ,WRITE)

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

std::string FileSystem::listDir(const std::string& path)
{
	std::stringstream ss;
	int curNr;
	if (path == "")
		curNr = this->workBlock;
	else
		curNr = this->getDirBlockIndex(path);

	std::string dataStr = this->mMemBlockDevice.readBlock(curNr).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	ss << "Type\t\tName\t\tPermissions\tSize" << std::endl;
	for (int i = 0; i < curDir->nrOfElements; i++)
	{
		ss	<< ((curDir->elements[i].flags & 1) == 0 ? "Folder" : "File") << "\t\t"
			<< curDir->elements[i].name << ((strlen(curDir->elements[i].name) < 8) ? "\t\t" : "\t")
			<< ((curDir->elements[i].flags & 2) == 2 ? "R" : "") << ((curDir->elements[i].flags & 4) == 4 ? "W" : "") << "\t\t"
			<< this->occupiedList[curDir->elements[i].pointer] << " byte" << std::endl;
	}
	return ss.str();
}

std::string FileSystem::readFile(const std::string& path)
{
	std::string name;
	int curBlock = this->getDirBlockIndex(path, name);

	//std::string dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
	//dirBlock* curDir = (dirBlock*)dataStr.c_str();

	//bool found = false;

	//for (int i = 0; i < curDir->nrOfElements && !found; i++)
	//{
	//	if (strcmp(curDir->elements[i].name, name.c_str()) == 0)
	//	{
	//		if ((curDir->elements[i].flags & 1) == 0)
	//			throw "Requested file is actually a folder";
	//		found = true;
	//		curBlock = curDir->elements[i].pointer;
	//	}
	//}
	//if (!found)
	//	throw "File not found";

	curBlock = this->getBlockNr(curBlock, name, 1, 0);

	bool readDone = false;
	std::string dataStr;
	fileBlock* curFile;
	std::stringstream ss;
	std::string token;
	while (!readDone)
	{
		dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
		curFile = (fileBlock*)dataStr.c_str();
		//ss.write(curFile->data, 510);
		token = std::string(curFile->data, std::find(curFile->data, curFile->data + 511, '\0'));
		ss << token;
		if (curFile->next == 0)
			readDone = true;
		else
			curBlock = curFile->next;
	}

	return ss.str();
}

void FileSystem::writeFile(const std::string& path, const std::string& text)
{
	std::string name;
	int curBlock = this->getDirBlockIndex(path, name);
	this->getBlockNr(curBlock, name, 1, 1);				// Just to check that the user have permission to write to this file
	this->removeFile(path);
	this->createFile(path, text);
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
			if ((curDir->elements[i].flags & 1) == 0)
				throw "Requested file is actually a folder";
			found = true;
			fileBlock = curDir->elements[i].pointer;
			curDir->elements[i] = curDir->elements[--curDir->nrOfElements];
			memset(&curDir->elements[curDir->nrOfElements], 0, sizeof(dirElement));
			this->mMemBlockDevice.writeBlock(curBlock, (char*)curDir);
		}
	}
	if (!found)
		throw "File not found";

	this->recursiveRemove(fileBlock);
	this->increaseDirSize(path, -this->occupiedList[fileBlock]);
	this->occupiedList[fileBlock] = -1;
}

void FileSystem::copyFile(const std::string& source, const std::string& target)
{
	std::string sourceName, targetName;
	int sourceDir = this->getDirBlockIndex(source, sourceName);
	int targetDir = this->getDirBlockIndex(target, targetName);

	int sourceFile = this->getBlockNr(sourceDir, sourceName, 1);
	bool fileExists = false;
	try
	{
		this->getBlockNr(targetDir, targetName, 1);
		fileExists = true;
	}
	catch (char* e)
	{

	}

	if (fileExists)
		throw "Can't overwrite existing file";

	int targetIndex = this->getFreeBlock();
	int targetStartIndex = targetIndex;
	do
	{
		this->occupiedList[targetIndex] = this->occupiedList[sourceFile];

		std::string sourceStr = this->mMemBlockDevice.readBlock(sourceFile).toString();
		fileBlock* copiedBlock = (fileBlock*)sourceStr.c_str();
		if (copiedBlock->next != 0)
		{
			sourceFile = copiedBlock->next;
			copiedBlock->next = this->getFreeBlock();
		} 
		this->mMemBlockDevice.writeBlock(targetIndex, (char*)copiedBlock);
		targetIndex = copiedBlock->next;
	} while (targetIndex != 0);

	dirElement element;
	element.flags = 1;
	strncpy_s(element.name, targetName.c_str(), NAMECAP);
	element.pointer = targetStartIndex;
	this->addDirElement(targetDir, &element);

	this->increaseDirSize(target, this->occupiedList[targetStartIndex]);
}

void FileSystem::createImage(const std::string& path)
{
	binaryBlock image;
	for (int i = 0; i < BLOCKCOUNT; i++)
	{
		memcpy_s(&image.data[i * BLOCKSIZE], BLOCKSIZE, this->mMemBlockDevice.readBlock(i).toString().c_str(), BLOCKSIZE);
		image.occupiedList[i] = this->occupiedList[i];
	}
	std::ofstream file;
	file.open(path, std::ofstream::binary);
	if(!file.is_open())
		throw "Couldn't save to file";
	file.write((char*)&image, sizeof(image));
	file.close();
}

void FileSystem::restoreImage(const std::string& path)
{
	binaryBlock image;
	std::ifstream file;
	file.open(path, std::ifstream::binary);
	if (!file.is_open())
		throw "Couldn't open file";
	file.read((char*)&image, sizeof(image));

	char blockData[BLOCKSIZE];
	for (int i = 0; i < BLOCKCOUNT; i++)
	{
		memcpy_s(blockData, BLOCKSIZE, image.data + i * BLOCKSIZE, BLOCKSIZE);
		this->mMemBlockDevice.writeBlock(i, blockData);
		this->occupiedList[i] = image.occupiedList[i];
	}
	file.close();
	this->workBlock = 0;
}

void FileSystem::increaseDirSize(const std::string& path, const int size)
{
	std::string junk;
	this->recursiveIncreaseDirSize(this->getDirBlockIndex(path, junk), size);
}

void FileSystem::chmod(const std::string& path, char permission)
{
	if (permission > 3)
		throw "Incorrect permission input";
	permission <<= 1;

	std::string name;
	int curBlock = this->getDirBlockIndex(path, name);


	std::string dataStr = this->mMemBlockDevice.readBlock(curBlock).toString();
	dirBlock* curDir = (dirBlock*)dataStr.c_str();

	bool found = false;

	for (int i = 0; i < curDir->nrOfElements && !found; i++)
	{
		if (strcmp(curDir->elements[i].name, name.c_str()) == 0)
		{
			if ((curDir->elements[i].flags & 1) == 0)
				throw "Requested file is actually a folder";
			found = true;
			curDir->elements[i].flags = (curDir->elements[i].flags & 1) | permission;
		}
	}
	if (!found)
		throw "File not found";

	this->mMemBlockDevice.writeBlock(curBlock, (char*)curDir);
}
