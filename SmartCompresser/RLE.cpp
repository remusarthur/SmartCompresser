#include "stdafx.h"
#include "BitFileManager.cpp"
#include "BaseCompression.h"
#include <bitset>
#include <cmath>

class RLE: public BaseCompression
{

public:

	int compressFile(const std::string& inputPath, const std::string& outputPath)
	{
		char currentChar;
		char lastChar;
		unsigned char frequency = 1;
		bool first = true;

		std::ifstream file;
		BitFileManager fileManager(BitFileManager::Mode::Write, outputPath);
		file.open(inputPath, std::ios_base::binary);
		addHeader(fileManager.getOStream());

		file.get(lastChar);
		auto writeData = [&]()
		{
			fileManager.write(bool(frequency > 1));

			if (frequency > 1)
				fileManager.write(frequency);
			
			fileManager.write(static_cast<unsigned char>(lastChar));
		};

		while (file.get(currentChar))
		{
			if (lastChar != currentChar || frequency >= 255)
			{
				writeData();
				lastChar = currentChar;
				frequency = 1;
			}
			else
				frequency++;
		}

		writeData();

		file.close();

		return EXIT_SUCCESS;
	}

	int decompressFile(const std::string& inputPath, const std::string& outputPath)
	{
		std::ofstream os;
		char currentChar;
		unsigned char frequency;

		BitFileManager fileManager(BitFileManager::Mode::Read, inputPath);

		if (!checkHeader(fileManager.getIStream()))
			return EXIT_FAILURE;

		os.open(outputPath, std::ios_base::binary);
		
		bool moreData = true;
		
		while (moreData)
		{
			bool hasFrecv = false;
			if (fileManager.read(hasFrecv) != EXIT_SUCCESS)
			{
				moreData = false;
				continue;
			}

			if (hasFrecv == true)
				fileManager.read(frequency);
			else frequency = 1;

			if (fileManager.read(reinterpret_cast<unsigned char&>(currentChar)) == EXIT_SUCCESS)
			{
				for (unsigned char j = 0; j < frequency; ++j)
					os.write((char*)&currentChar, sizeof(char));
			}
		}

		os.close();

		return EXIT_SUCCESS;
	}

	RLE(BaseCompression::PrivateKeyType key) : BaseCompression(key)
	{};
};