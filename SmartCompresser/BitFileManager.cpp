#pragma once

#include "stdafx.h"
#include <bitset>
#define MAX_BUFFER_SIZE 1024*1024


class BitFileManager
{
public:

	enum Mode{
		Read,
		Write
	};


	void write(unsigned char value)
	{
		if (mode != Write)
			return;

		std::bitset<8> bits(value);

		for (i = 7; i >= 0; i--)
		{
			buffer[bufferSize++] = bits[i];
			flush();
		}
	}

	void write(bool value)
	{
		if (mode != Write)
			return;

		std::bitset<1> bits(value);
		
		buffer[bufferSize++] = value ? true : false;
		flush();
	}

	int read(unsigned char& value)
	{
		if (mode != Read)
			return EXIT_FAILURE;

		value = 0;
		for (j = 0; j < 8; j++)
		{
			if (bufferSize == 0)
			{
				int err = flush(true);

				if (err != EXIT_SUCCESS)
					return err;
			}

			value |= (buffer[--bufferSize] << ((7 - j) & 7));
		}

		return EXIT_SUCCESS;
	}

	int read(bool& value)
	{
		if (mode != Read)
			return EXIT_FAILURE;

		if (bufferSize == 0)
		{
			int err = flush(true);

			if (err != EXIT_SUCCESS)
				return err;
		}

		value = buffer[--bufferSize];

		return EXIT_SUCCESS;
	}

	BitFileManager(Mode mode_, const std::string& filePath)
	{
		mode = mode_;
		if (mode == Mode::Read)
			is = std::ifstream(filePath, std::ifstream::eofbit | std::ios::binary);
		
		if (mode == Mode::Write)
			os = std::ofstream(filePath, std::ios::binary);
	}

	~BitFileManager()
	{
		flush(true);

		if (mode == Mode::Read)
			is.close();

		if (mode == Mode::Write)
			os.close();
	}

	
private:
	int i, j;
	Mode mode;
	std::ifstream is;
	std::ofstream os;

	std::bitset<MAX_BUFFER_SIZE> buffer;
	int bufferSize = 0;

	inline int flush(bool force = false)
	{
		if (bufferSize == MAX_BUFFER_SIZE || force)
		{
			if (mode == Read)
			{
				const int bfrSize = ((MAX_BUFFER_SIZE + 7) >> 3);
				char bfr[bfrSize];

				is.read(bfr, bfrSize);
				std::streamsize bytes = is.gcount();
				std::bitset<MAX_BUFFER_SIZE> revBuffer;

				bufferSize = bytes << 3;
				for (int j = 0; j < bufferSize; j++)
					revBuffer[j] = ((bfr[(j >> 3)] >> ((7 - j) & 7)) & 1);

				buffer.reset();
				for (int j = 0; j < bufferSize; j++)
					buffer[j] = revBuffer[(bytes << 3) - j - 1];


				if (bytes == 0)
					return EXIT_FAILURE;
			}
			if (mode == Write)
			{
				const int dim = (bufferSize + 7) >> 3;
				std::vector<unsigned char> result(dim);
				for (int j = 0; j < bufferSize; j++)
					result[j >> 3] |= (buffer[j] << ((7 - j) & 7));

				for (int i = 0; i < dim; i++)
					os << result[i];

				buffer.reset();
				bufferSize = 0;
			}

			return EXIT_SUCCESS;
		}
	}


};