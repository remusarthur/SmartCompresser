// SmartCompresser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LZW.cpp"
#include "RLE.cpp"
#include "BitFileManager.cpp"
#include "Audio.cpp"
#include <ctime>

RLE rle;

int _tmain(int argc, _TCHAR* argv[])
{
	time_t ts;
	time(&ts);
	std::cout << "Starting compression" << std::endl;
	//LZWCompressor::compressFile("untitled.bmp", "output.lzw");
	//rle.compressFile("untitled.bmp", "output.rle");
	//rle.compressFile("new1.txt", "out.rle");
	AudioCompresser audioComp;
	audioComp.compressFile("Louder16BitSigned.raw", "out.raw");

	std::cout << "Compression finished in ";
	time_t te; 
	time(&te);
	std::cout << te - ts << std::endl;

	std::cout << "Starting decompression" << std::endl;
	//LZWCompressor::decompressFile("output.lzw", "untitledDecomp.bmp");
	//rle.decompressFile("output.rle", "untitledDecomp2.bmp");
	//rle.decompressFile("out.rle", "new1Dec.txt");
	//audioComp.decompressFile("Louder16BitSignedALAW.raw", "Louder16BitSignedDecod.raw");

	audioComp.decompressFile("out.raw", "Louder16BitSignedDecod.raw");
	std::cout << "Decompression finished in";
	time(&ts);
	std::cout << ts - te << std::endl;

	/*BitFileManager bitManager;
	bitManager.mode = BitFileManager::Mode::Write;
	bitManager.os = std::ofstream("out.rle");
	char ch;
	bool b;
	bitManager.write(unsigned char('a'));
	bitManager.write(unsigned char('z'));
	b = true;
	bitManager.write(b);
	bitManager.write(b);
	b = false;
	bitManager.write(b);
	b = true;
	bitManager.write(b);

	//BitFileManager bitManager;
	bitManager.mode = BitFileManager::Mode::Read;
	bitManager.is = std::ifstream("out.rle", std::ifstream::eofbit);

	unsigned char a;
	bitManager.read(a);
	bitManager.read(a);
	bitManager.read(a);*/
		return 0;
}