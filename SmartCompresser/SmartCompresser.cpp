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
	size_t MIN_SIZE = 1024 * 160;
	std::string tempFileName = "file.tmp";
	std::string compressedtempFileName = "fileCompressed.tmp";

	const char HuffmanKey = static_cast<char>(1);
	const char RleKey = static_cast<char>(2);
	const char LzwKey = static_cast<char>(3);
	const char MuLawKey = static_cast<char>(4);
	
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
			return compressFile(input, output, LempelZivWelch);
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

		size_t minSize = (1<<31);
		Mode mode;

		auto isMin = [&]()
		{
			std::ifstream is(compressedtempFileName, std::ios_base::binary);
			std::streampos fsize = 0;

			fsize = is.tellg();
			is.seekg(0, std::ios::end);
			fsize = is.tellg() - fsize;

			if (fsize <= minSize)
			{
				minSize = fsize;
				return true;
			}

			return false;
		};

		compressFile(tempFileName, compressedtempFileName, HuffmanCoding);
		if (isMin())
			mode = HuffmanCoding;
		compressFile(tempFileName, compressedtempFileName, LempelZivWelch);
		if (isMin())
			mode = LempelZivWelch;
		compressFile(tempFileName, compressedtempFileName, RunLengthEncoding);
		if (isMin())
			mode = RunLengthEncoding;

		return compressFile(input, output, mode);
	}

public:
	
	enum Mode
	{
		RunLengthEncoding,
		LempelZivWelch,
		Mulaw,
		HuffmanCoding,
		Smart
	};
	int compressFile(const std::string& input, const std::string& output, Mode mode)
	{
		RLE rle(RleKey);
		AudioCompresser audioComp(MuLawKey);
		Huffman huffman(HuffmanKey);
		LZWCompressor lzw(LzwKey);

		switch (mode)
		{
		case RunLengthEncoding:
				return rle.compressFile(input, output);
		case LempelZivWelch:
				return lzw.compressFile(input, output);
		case Mulaw:
				return audioComp.compressFile(input, output);
		case HuffmanCoding:
				return huffman.compressFile(input, output);
		case Smart:
				return smartCompress(input, output);
		}
	}

	int decompressFile(const std::string& input, const std::string& output)
	{
		RLE rle(RleKey);
		AudioCompresser audioComp(MuLawKey);
		Huffman huffman(HuffmanKey);
		LZWCompressor lzw(LzwKey);
		Mode mode;

		std::ifstream is(input, std::ios_base::binary);
		std::ofstream os(tempFileName, std::ios_base::binary);

		if (!is.is_open() || !os.is_open())
			return EXIT_FAILURE;
		
		char data;
		is.get(data);
		is.close();

		if (data == RleKey)
			mode = RunLengthEncoding;
		if (data == MuLawKey)
			mode = Mulaw;
		if (data == HuffmanKey)
			mode = HuffmanCoding;
		if (data == LzwKey)
			mode = LempelZivWelch;

		switch (mode)
		{
		case RunLengthEncoding:
			return rle.decompressFile(input, output);
		case LempelZivWelch:
			return lzw.decompressFile(input, output);
		case Mulaw:
			return audioComp.decompressFile(input, output);
		case HuffmanCoding:
			return huffman.decompressFile(input, output);
		}
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
			smartCompresser.compressFile(input, output, SmartCompresser::RunLengthEncoding);
		else if (mode == "MULAW")
			smartCompresser.compressFile(input, output, SmartCompresser::Mulaw);
		else if (mode == "LZW")
			smartCompresser.compressFile(input, output, SmartCompresser::LempelZivWelch);
		else if (mode == "HUFFMAN")
			smartCompresser.compressFile(input, output, SmartCompresser::HuffmanCoding);
		else
			smartCompresser.compressFile(input, output, SmartCompresser::Smart);
	}
	std::cout << "Compression finished in ";
	time_t te; 
	time(&te);
	std::cout << te - ts << std::endl;

	return 0;
}