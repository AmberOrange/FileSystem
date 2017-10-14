#include <iostream>
#include "../Header/memblockdevice.h"
#include <stdexcept>
/*
 * Testfile for MemBlockDevice-class
 */

const int SIZE = 512;
const int BLOCKNR = 250;

int main(void) {
	std::cout << "MEMBLOCKDEVICE TEST\n\n";
    MemBlockDevice *a = new MemBlockDevice(-3);
	std::cout << "Testing constructor, size: " << a->size() << std::endl;

    /* Testing writeBlock with char-array */
    char cArr[SIZE];
    for (int i = 0; i < SIZE; ++i)
        cArr[i] = (('a' + i)%25 + 'a');

	std::cout << "Testing writeBlock cArr[]: \n 1 = Success, -2 not same dimensions, -1 blockNr out of range\nReturned: \t" << a->writeBlock(1, cArr) << std::endl;
	std::cout << a->operator [](1) << std::endl << std::endl;

    /* Testing writeBlock with vector<char> */
    std::vector<char> vec;
    for (int i = 0; i < SIZE; ++i)
        vec.push_back(('a' + i)%25 + 'a');

	std::cout << "Testing writeBlock vector<char> :\n 1 = Success, -2 not same dimensions, -1 blockNr out of range\nReturned: \t" << a->writeBlock(123,vec) << std::endl << std::endl;

    /* Testing writeBlock with std::string */
	std::string strBlock;
    strBlock.resize(SIZE);
    for (int i = 0; i < SIZE; ++i)
        strBlock[i] = (('a' + i)%25 + 'a');

	std::cout << "Testing writeBlock std::string :\n 1 = Success, -2 not same dimensions, -1 blockNr out of range\nReturned: \t" << a->writeBlock(122,strBlock) << std::endl << std::endl;

    /* Testing readBlock */
    try {
		std::cout << a->readBlock(123) << std::endl << std::endl;;
    }
    catch (const std::out_of_range &err) {
		std::cout << "Exception error: " << err.what() << std::endl;
    }

    /* Testing copy-constructor */
    MemBlockDevice b(*a);

    /* Testing assignment operator */
    b = *a;

    /* Testing [ ]-operator */
	std::cout << "Testing [ ]-operator" << std::endl;
	std::cout << "output b[x]: " << b[122] << std::endl;
    b[122] = b[0];
	std::cout << "output b[x]: " << b[122] << std::endl;  // should be empty

	std::cout << "\nEND\n";

    delete a;
	getchar();
    return 0;
}
