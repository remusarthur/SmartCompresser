#include "stdafx.h"



inline std::vector<char> operator + (std::vector<char> vc, char c)
{
	vc.push_back(c);
	return vc;
}

/// Type used to store and retrieve codes.
using KeyType = std::uint16_t;

/// Dictionary Maximum Size (when reached, the dictionary will be reset)
static KeyType dictMaxSize = std::numeric_limits<KeyType>::max();



class LZWCompressor
{

	enum Mode{
		Compress,
		Decompress
	};

	
	class Dictionary
	{
		struct Node
		{

			explicit Node(char c) : first(dictMaxSize), value(c), leftChild(dictMaxSize), rightChild(dictMaxSize) {}
			KeyType    leftChild;
			KeyType    rightChild;

			KeyType    first;// first child (like linked lists)
			char       value;

		};

	public:
		Dictionary()
		{
			const int minCharValue = std::numeric_limits<char>::min();
			const int maxCharValue = std::numeric_limits<char>::max();
			KeyType k = 0;

			for (int c = minCharValue; c <= maxCharValue; ++c)
				initials[static_cast<unsigned char> (c)] = k++;

			bTreeNodes.reserve(dictMaxSize);
			resetValues();
		}

		void resetValues()
		{
			const int minCharValue = std::numeric_limits<char>::min();
			const int maxCharValue = std::numeric_limits<char>::max();
			
			bTreeNodes.clear();
			for (int c = minCharValue; c <= maxCharValue; ++c)
				bTreeNodes.push_back(Node(static_cast<char>(c)));
		}

		KeyType searchInsert(KeyType i, char data)
		{
			if (bTreeNodes.size() == dictMaxSize)
				resetValues();

			if (i == dictMaxSize)
				return searchInitials(data);

			const KeyType treeSize = bTreeNodes.size();
			KeyType currentIndex = bTreeNodes[i].first;

			if (currentIndex != dictMaxSize)
			{
				bool needsAdded = false;
				while (!needsAdded)
				{
					if (data < bTreeNodes[currentIndex].value)
						if (bTreeNodes[currentIndex].leftChild == dictMaxSize)
						{
							bTreeNodes[currentIndex].leftChild = treeSize;
							needsAdded = true;
						}
						else
							currentIndex = bTreeNodes[currentIndex].leftChild;
					else
					if (data > bTreeNodes[currentIndex].value)
						if (bTreeNodes[currentIndex].rightChild == dictMaxSize)
						{
							bTreeNodes[currentIndex].rightChild = treeSize;
							needsAdded = true;
						}
						else
							currentIndex = bTreeNodes[currentIndex].rightChild;
					else
						return currentIndex;
				}
			}
			else
				bTreeNodes[i].first = treeSize;

			bTreeNodes.push_back(Node(data));
			return dictMaxSize;
		}

		KeyType searchInitials(char c) const
		{
			return initials[static_cast<unsigned char> (c)];
		}

	private:

		std::vector<Node> bTreeNodes;
		std::array<KeyType, 1u << CHAR_BIT> initials;
	};

	void compress(std::istream &is, std::ostream &os)
	{
		Dictionary dict;
		KeyType index = dictMaxSize;
		char data;

		while (is.get(data))
		{
			const KeyType temp = index;

			if ((index = dict.searchInsert(temp, data)) == dictMaxSize)//string doesn't exist in the dictionary
			{	
				os.write(reinterpret_cast<const char *> (&temp), sizeof (KeyType));
				index = dict.searchInitials(data);
			}
		}

		if (index != dictMaxSize)
			os.write(reinterpret_cast<const char *> (&index), sizeof (KeyType));
	}

	void decompress(std::istream &is, std::ostream &os)
	{
		std::vector<std::pair<KeyType, char>> dictionary;

		const auto resetDictionary = [&dictionary] {
			dictionary.clear();
			dictionary.reserve(dictMaxSize);

			const int minc = std::numeric_limits<char>::min();
			const int maxc = std::numeric_limits<char>::max();

			for (int c = minc; c <= maxc; ++c)
				dictionary.push_back({ dictMaxSize, static_cast<char> (c) });
		};

		const auto rebuildString = [&dictionary](KeyType k) -> const std::vector<char> * {
			static std::vector<char> s; // String

			s.clear();
			s.reserve(dictMaxSize);

			while (k != dictMaxSize)
			{
				s.push_back(dictionary[k].second);
				k = dictionary[k].first;
			}

			std::reverse(s.begin(), s.end());
			return &s;
		};

		resetDictionary();

		KeyType i = dictMaxSize; // Index
		KeyType k; // Key

		while (is.read(reinterpret_cast<char *> (&k), sizeof (KeyType)))
		{
			if (dictionary.size() == dictMaxSize)
				resetDictionary();

			if (k > dictionary.size())
				throw std::runtime_error("invalid compressed code");

			const std::vector<char> *s; // String

			if (k == dictionary.size())
			{
				dictionary.push_back({ i, rebuildString(i)->front() });
				s = rebuildString(k);
			}
			else
			{
				s = rebuildString(k);

				if (i != dictMaxSize)
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