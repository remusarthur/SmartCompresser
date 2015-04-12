// SmartCompresser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LZW.cpp"

int _tmain(int argc, _TCHAR* argv[])
{
	LZWCompressor compressor;

	compressor.compressFile("test.pdb", "output.lzw");
	compressor.decompressFile("output.lzw", "new2.txt");

	return 0;
}

