#include <iostream>
#include "../Header/block.h"
#include "../Header/blockdevice.h"
#include <stdexcept>      // std::out_of_range

const int SIZE = 512;

int main(void) {
    Block *a = new Block(SIZE);

    /* Testing writeBlock with char-arr */
	std::cout << "Testing writeBlock char-array" << std::endl;
    char tmpArr[SIZE-3];
    for (int i = 0; i < SIZE-3; ++i)
        tmpArr[i] = (('a'+ i)%25 + 'a');
    std::cout << "Writing char arr to block: ";

    a->writeBlock(tmpArr);
    //std::cout << *a << std::endl << std::endl << std::endl;

    /* Testing writeBlock with vec<char> */
    std::vector<char> blas;
    for (int i = 0; i < SIZE+3; ++i) {
        blas.push_back(('a' + i)%25 + 'a');
    }
	std::cout << "Vec size: " << blas.size() << std::endl;

	std::cout << "Writing vec into block: ";
    if (a->writeBlock(blas) == 1)
        std::cout << "succeded!\n";
    else
        std::cout << "Failed!\n";

	std::cout << "Testing [ ] - operator!\n";
    for (int i = 0; i < SIZE; ++i) {
        try {
				std::cout << a->operator [](i);
        }
        catch (const std::out_of_range& e) {
				std::cout << "Error: " << e.what() << std::endl;
        }
    }
	std::cout << std::endl << std::endl;
    /* Testing writeBlock with std::string */
	std::string strBlock;
    strBlock.resize(SIZE);
    for (int i = 0; i < SIZE; ++i) {
        strBlock[i] = (('a' + i)%25 + 'a');
    }
	std::cout << "StrBlock:\n\t" << strBlock << std::endl;
	std::cout << "str size: " << strBlock.size() << std::endl << std::endl;

	std::cout << "Writing std-str into block: ";
    if (a->writeBlock(strBlock) == 1)
        std::cout << "succeded!\n";
    else
        std::cout << "Failed!\n";

	std::cout << "Testing [ ] - operator!\n";
    for (int i = 0; i < SIZE; ++i) {
        try {
				std::cout << a->operator [](i);
        }
        catch (const std::out_of_range& e) {
				std::cout << "Error: " << e.what() << std::endl;
        }
    }

	std::cout << "\nTotal size: " << a->size() << std::endl << std::endl;

    /* Testing copy-constr */
    Block b(300);
    Block c(*a);

    /* Testing assignment operator */
    b = *a;

	std::cout << "Testing << operator\n" << *a << "\n";

    std::cout << "\nEND" << std::endl;

    delete a;
	getchar();
    return 0;
}
