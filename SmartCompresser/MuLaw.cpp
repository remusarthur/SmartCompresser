#pragma once

#include "stdafx.h"


class AudioCompresser
{
	typedef int16_t InitialType;
	typedef int8_t CompressType;

	const uint16_t MMAX = 0x1FFF;
	const uint16_t BIAS = 0x84;//132

	CompressType encode(InitialType data)
	{
		uint16_t bitMask = 0x1000;
		uint8_t sign = 0;
		uint8_t pos = 12;
		uint8_t mantissa = 0;
		if (data < 0)
		{
			data = -data;
			sign = 0x80;
		}

		data += BIAS;
		if (data > MMAX)
			data = MMAX;
		
		while((data & bitMask) != bitMask && pos >= 5)
		{
			bitMask >>= 1;
			pos--;
		}

		mantissa = (data >> (pos - 4)) & 0x0f;
		return (~(sign | ((pos - 5) << 4) | mantissa));
	}


	InitialType decode(CompressType data)
	{
		uint8_t sign = 0, pos = 0;
		InitialType result = 0;

		data = ~data;
		if (data & 0x80)
		{
			data &= ~(1 << 7);
			sign = -1;
		}

		pos = ((data & 0xF0) >> 4) + 5;
		result = ((1 << pos) | ((data & 0x0F) << (pos - 4))
			| (1 << (pos - 5))) - BIAS;

		return (sign == 0) ? (result) : (-(result));
	}

public:

	int compressFile(const std::string& inputPath, const std::string& outputPath)
	{
		std::ifstream input_file(inputPath, std::ios_base::binary);
		std::ofstream output_file(outputPath, std::ios_base::binary);

		if (!output_file.is_open() || !input_file.is_open())
			return EXIT_FAILURE;

		InitialType data;
		while (input_file.read(reinterpret_cast<char *>(&data), sizeof(data)))
		{
			CompressType result = encode(data);
			output_file.write(reinterpret_cast<const char *> (&result), sizeof(CompressType));
		}

		return EXIT_SUCCESS;
	}

	int decompressFile(const std::string& inputPath, const std::string& outputPath)
	{
		std::ifstream input_file(inputPath, std::ios_base::binary);
		std::ofstream output_file(outputPath, std::ios_base::binary);

		if (!output_file.is_open() || !input_file.is_open())
			return EXIT_FAILURE;

		CompressType data;
		while (input_file.read(reinterpret_cast<char*>(&data), sizeof(CompressType)))
		{
			InitialType result = decode(data);
			output_file.write(reinterpret_cast<char*>(&result), sizeof(InitialType));
		}

		return EXIT_SUCCESS;
	}
};