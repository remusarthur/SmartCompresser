// SmartCompresser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "BitFileManager.cpp"
#include "LZW.cpp"
#include "RLE.cpp"
#include "MuLaw.cpp"
#include <ctime>
#include <windows.h>
#include <string>
#include "Huffman.cpp"

class SmartCompresser
{
	size_t MIN_SIZE = 1024 * 16;
	std::string tempFileName = "file.tmp";
	std::string compressedtempFileName = "fileCompressed.tmp";

	
	int smartCompress(const std::string& input, const std::string& output)
	{
		std::ifstream is(input, std::ios_base::binary);
		std::ofstream os(tempFileName, std::ios_base::binary);

		if (!is.is_open() || !os.is_open())
			return EXIT_FAILURE;
		std::streampos fsize = 0;

		fsize = is.tellg();
		is.seekg(0, std::ios::end);
		fsize = is.tellg() - fsize;

		if (fsize < MIN_SIZE)
		{
			is.close();
			return compressFile(input, output, LzwMode);
		}

		is.seekg(MIN_SIZE / 2, std::ios::beg);
		size_t nbytes = 0;
		char data;
		while (is.get(data) && nbytes < MIN_SIZE / 2)
		{
			os << data;
			nbytes++;
		}
		os.close();

		size_t minSize = nbytes;
		Mode mode;

		auto isMin = [&]()
		{
			std::ifstream is(compressedtempFileName, std::ios_base::binary);
			std::streampos fsize = 0;

			fsize = is.tellg();
			is.seekg(0, std::ios::end);
			fsize = is.tellg() - fsize;

			if (fsize < minSize)
			{
				minSize = fsize;
				return true;
			}

			return false;
		};

		compressFile(tempFileName, compressedtempFileName, HuffmanMode);
		if (isMin())
			mode = HuffmanMode;
		compressFile(tempFileName, compressedtempFileName, LzwMode);
		if (isMin())
			mode = LzwMode;
		compressFile(tempFileName, compressedtempFileName, RleMode);
		if (isMin())
			mode = RleMode;

		return compressFile(input, output, mode);
	}

public:
	
	enum Mode
	{
		RleMode,
		LzwMode,
		MulawMode,
		HuffmanMode,
		SmartMode
	};
	int compressFile(const std::string& input, const std::string& output, Mode mode)
	{
		RLE rle(static_cast<char>(1));
		AudioCompresser audioComp(static_cast<char>(2));
		Huffman huffman(static_cast<char>(3));
		LZWCompressor lzw(static_cast<char>(4));

		if (mode == RleMode)
			return rle.compressFile(input, output);
		if (mode == LzwMode)
			return lzw.compressFile(input, output);
		if (mode == MulawMode)
			return audioComp.compressFile(input, output);
		if (mode == HuffmanMode)
			return huffman.compressFile(input, output);
		if (mode == SmartMode)
			return smartCompress(input, output);
	}

	int decompressFile(const std::string& input, const std::string& output)
	{
		RLE rle(static_cast<char>(1));
		AudioCompresser audioComp(static_cast<char>(2));
		Huffman huffman(static_cast<char>(3));
		LZWCompressor lzw(static_cast<char>(4));
		Mode mode;

		std::ifstream is(input, std::ios_base::binary);
		std::ofstream os(tempFileName, std::ios_base::binary);

		if (!is.is_open() || !os.is_open())
			return EXIT_FAILURE;
		
		char data;
		is.get(data);
		is.close();
		if (data == static_cast<char>(1))
			mode = RleMode;
		if (data == static_cast<char>(2))
			mode = MulawMode;
		if (data == static_cast<char>(3))
			mode = HuffmanMode;
		if (data == static_cast<char>(4))
			mode = LzwMode;

		if (mode == RleMode)
			return rle.decompressFile(input, output);
		if (mode == LzwMode)
			return lzw.decompressFile(input, output);
		if (mode == MulawMode)
			return audioComp.decompressFile(input, output);
		if (mode == HuffmanMode)
			return huffman.decompressFile(input, output);
	}
};

std::string ws2s(const std::wstring& wideString)
{
	return std::string(wideString.begin(), wideString.end());
}

int _tmain(int argc, _TCHAR* argv[])
{
	time_t ts;
	time(&ts);
	if (argc != 5) //input output mode
		return ERROR_BAD_ARGUMENTS;

	std::string input = ws2s(argv[1]);
	std::string output = ws2s(argv[2]);
	std::string mode = ws2s(argv[3]);
	std::string cmp = ws2s(argv[4]);

	
	std::cout << "Starting compression" << std::endl;
	SmartCompresser smartCompresser;

	if (cmp == "DECOMPRESS")
	{
		smartCompresser.decompressFile(input, output);
	}
	else
	{
		if (mode == "RLE")
			smartCompresser.compressFile(input, output, SmartCompresser::RleMode);
		else if (mode == "MULAW")
			smartCompresser.compressFile(input, output, SmartCompresser::MulawMode);
		else if (mode == "LZW")
			smartCompresser.compressFile(input, output, SmartCompresser::LzwMode);
		else if (mode == "HUFFMAN")
			smartCompresser.compressFile(input, output, SmartCompresser::HuffmanMode);
		else
			smartCompresser.compressFile(input, output, SmartCompresser::SmartMode);
	}
	std::cout << "Compression finished in ";
	time_t te; 
	time(&te);
	std::cout << te - ts << std::endl;

	return 0;
}