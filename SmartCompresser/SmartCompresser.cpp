// SmartCompresser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LZW.cpp"
#include <ctime>

int _tmain(int argc, _TCHAR* argv[])
{
	time_t ts;
	time(&ts);
	std::cout << "Starting compression";
	LZWCompressor::compressFile("test.pdb", "output.lzw");
	std::cout << "Compression finished in ";
	time_t te;
	time(&te);
	std::cout << te - ts << std::endl;

	std::cout << "Starting decompression";
	LZWCompressor::decompressFile("output.lzw", "new2.txt");
	std::cout << "Decompression finished in";
	time(&ts);
	std::cout << ts - te << std::endl;
	return 0;
}

