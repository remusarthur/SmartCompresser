#include "stdafx.h"
///
/// @file
/// @author Julius Pettersson
/// @copyright MIT/Expat License.
/// @brief LZW file compressor
/// @version 1
///
/// This is the C++11 implementation of a Lempel-Ziv-Welch single-file command-line compressor.
/// It uses the simpler fixed-width code compression method.
/// It was written with Doxygen comments.
///
/// @see http://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Welch
/// @see http://marknelson.us/2011/11/08/lzw-revisited/
/// @see http://www.cs.duke.edu/csed/curious/compression/lzw.html
/// @see http://warp.povusers.org/EfficientLZW/index.html
/// @see http://en.cppreference.com/
/// @see http://www.doxygen.org/
///

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <limits>
#include <map>
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>


inline std::vector<char> operator + (std::vector<char> vc, char c)
{
	vc.push_back(c);
	return vc;
}

/// Type used to store and retrieve codes.
using CodeType = std::uint16_t;

/// Dictionary Maximum Size (when reached, the dictionary will be reset)
static CodeType dms = std::numeric_limits<CodeType>::max();



class LZWCompressor
{

	enum Mode{
		Compress,
		Decompress
	};

	
	class EncoderDictionary {

		///
		/// @brief Binary search tree node.
		///
		struct Node {

			///
			/// @brief Default constructor.
			/// @param c    byte that the Node will contain
			///
			explicit Node(char c) : first(dms), c(c), left(dms), right(dms)
			{
			}

			CodeType    first;  ///< Code of first child string.
			char        c;      ///< Byte.
			CodeType    left;   ///< Code of child node with byte < `c`.
			CodeType    right;  ///< Code of child node with byte > `c`.
		};

	public:

		///
		/// @brief Default constructor.
		/// @details It builds the `initials` cheat sheet.
		///
		EncoderDictionary()
		{
			const int minc = std::numeric_limits<char>::min();
			const int maxc = std::numeric_limits<char>::max();
			CodeType k{ 0 };

			for (int c = minc; c <= maxc; ++c)
				initials[static_cast<unsigned char> (c)] = k++;

			bTreeNodes.reserve(dms);
			resetValues();
		}

		void resetValues()
		{
			bTreeNodes.clear();

			const int minc = std::numeric_limits<char>::min();
			const int maxc = std::numeric_limits<char>::max();

			for (int c = minc; c <= maxc; ++c)
				bTreeNodes.push_back(Node(static_cast<char>(c)));
		}

		///
		/// @brief Searches for a pair (`i`, `c`) and inserts the pair if it wasn't found.
		/// @param i                code to search for
		/// @param c                attached byte to search for
		/// @returns The index of the pair, if it was found.
		/// @retval globals::dms    if the pair wasn't found
		///
		CodeType searchInsert(CodeType i, char c)
		{
			// dictionary's maximum size was reached
			if (bTreeNodes.size() == dms)
				resetValues();

			if (i == dms)
				return searchInitials(c);

			const CodeType vn_size = bTreeNodes.size();
			CodeType ci{ bTreeNodes[i].first }; // Current Index

			if (ci != dms)
			{
				while (true)
				if (c < bTreeNodes[ci].c)
				{
					if (bTreeNodes[ci].left == dms)
					{
						bTreeNodes[ci].left = vn_size;
						break;
					}
					else
						ci = bTreeNodes[ci].left;
				}
				else
				if (c > bTreeNodes[ci].c)
				{
					if (bTreeNodes[ci].right == dms)
					{
						bTreeNodes[ci].right = vn_size;
						break;
					}
					else
						ci = bTreeNodes[ci].right;
				}
				else // c == vn[ci].c
					return ci;
			}
			else
				bTreeNodes[i].first = vn_size;

			bTreeNodes.push_back(Node(c));
			return dms;
		}

		///
		/// @brief Fakes a search for byte `c` in the one-byte area of the dictionary.
		/// @param c    byte to search for
		/// @returns The code associated to the searched byte.
		///
		CodeType searchInitials(char c) const
		{
			return initials[static_cast<unsigned char> (c)];
		}

	private:

		/// Vector of nodes on top of which the binary search tree is implemented.
		std::vector<Node> bTreeNodes;

		/// Cheat sheet for mapping one-byte strings to their codes.
		std::array<CodeType, 1u << CHAR_BIT> initials;
	};

	void compress(std::istream &is, std::ostream &os)
	{
		EncoderDictionary ed;
		CodeType i{ dms }; // Index
		char c;

		while (is.get(c))
		{
			const CodeType temp{ i };

			if ((i = ed.searchInsert(temp, c)) == dms)
			{	
				os.write(reinterpret_cast<const char *> (&temp), sizeof (CodeType));
				i = ed.searchInitials(c);
			}
		}

		if (i != dms)
			os.write(reinterpret_cast<const char *> (&i), sizeof (CodeType));
	}

	///
	/// @brief Decompresses the contents of `is` and writes the result to `os`.
	/// @param [in] is      input stream
	/// @param [out] os     output stream
	///
	void decompress(std::istream &is, std::ostream &os)
	{
		std::vector<std::pair<CodeType, char>> dictionary;

		// "named" lambda function, used to reset the dictionary to its initial contents
		const auto reset_dictionary = [&dictionary] {
			dictionary.clear();
			dictionary.reserve(dms);

			const int minc = std::numeric_limits<char>::min();
			const int maxc = std::numeric_limits<char>::max();

			for (int c = minc; c <= maxc; ++c)
				dictionary.push_back({ dms, static_cast<char> (c) });
		};

		const auto rebuild_string = [&dictionary](CodeType k) -> const std::vector<char> * {
			static std::vector<char> s; // String

			s.clear();

			// the length of a string cannot exceed the dictionary's number of entries
			s.reserve(dms);

			while (k != dms)
			{
				s.push_back(dictionary[k].second);
				k = dictionary[k].first;
			}

			std::reverse(s.begin(), s.end());
			return &s;
		};

		reset_dictionary();

		CodeType i{ dms }; // Index
		CodeType k; // Key

		while (is.read(reinterpret_cast<char *> (&k), sizeof (CodeType)))
		{
			// dictionary's maximum size was reached
			if (dictionary.size() == dms)
				reset_dictionary();

			if (k > dictionary.size())
				throw std::runtime_error("invalid compressed code");

			const std::vector<char> *s; // String

			if (k == dictionary.size())
			{
				dictionary.push_back({ i, rebuild_string(i)->front() });
				s = rebuild_string(k);
			}
			else
			{
				s = rebuild_string(k);

				if (i != dms)
					dictionary.push_back({ i, s->front() });
			}

			os.write(&s->front(), s->size());
			i = k;
		}

		if (!is.eof() || is.gcount() != 0)
			throw std::runtime_error("corrupted compressed file");
	}


	int doFileAction(Mode mode, const std::string& inputPath, const std::string& outputPath)
	{
		std::ifstream input_file(inputPath, std::ios_base::binary);
		std::ofstream output_file(outputPath, std::ios_base::binary);

		if (!output_file.is_open() || !input_file.is_open())
			return EXIT_FAILURE;

		try
		{
			input_file.exceptions(std::ios_base::badbit);
			output_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);

			if (mode == Compress)
				compress(input_file, output_file);
			else if (mode == Decompress)
				decompress(input_file, output_file);
		}
		catch (const std::ios_base::failure &f)
		{
			std::cout << (std::string("File input/output failure: ") + f.what() + '.', false);
			return EXIT_FAILURE;
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	};

public:
	
	static int compressFile(const std::string& inputPath, const std::string& outputPath)
	{
		LZWCompressor obj;
		return obj.doFileAction(Mode::Compress, inputPath, outputPath);
	}

	static int decompressFile(const std::string& inputPath, const std::string& outputPath)
	{
		LZWCompressor obj;
		return obj.doFileAction(Mode::Decompress, inputPath, outputPath);
	}
};