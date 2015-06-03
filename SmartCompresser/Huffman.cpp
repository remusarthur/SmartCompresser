#include "stdafx.h"
#include "BitFileManager.cpp"
#include "BaseCompression.h"
#include <iostream>
#include <queue>
#include <map>
#include <climits> // for CHAR_BIT
#include <iterator>
#include <algorithm>

class Huffman: public BaseCompression
{
	static const int UniqueSymbols = 1 << CHAR_BIT;
	
	typedef std::vector<bool> HuffmanCode;
	typedef std::map<char, HuffmanCode> HuffCodeMapping;
	typedef std::map<HuffmanCode, char> HuffDecodeMapping;


	class INode
	{
	public:
		const int frequency;

		virtual ~INode() {}

	protected:
		INode(int frecv) : frequency(frecv) {}
	};

	class InternalNode : public INode
	{
	public:
		INode *const left;
		INode *const right;

		InternalNode(INode* c0, INode* c1) : INode(c0->frequency + c1->frequency), left(c0), right(c1) {}
		~InternalNode()
		{
			delete left;
			delete right;
		}
	};

	class LeafNode : public INode
	{
	public:
		const char c;

		LeafNode(int f, char c) : INode(f), c(c) {}
	};

	struct NodeComparator
	{
		bool operator()(const INode* lhs, const INode* rhs) const { return lhs->frequency > rhs->frequency; }
	};

	INode* BuildHuffmanTree(const int(&frequencies)[UniqueSymbols])
	{
		std::priority_queue<INode*, std::vector<INode*>, NodeComparator> tree;

		for (int i = 0; i < UniqueSymbols; ++i)
		if (frequencies[i] > 0)
			tree.push(new LeafNode(frequencies[i], (char)i));

		while (tree.size() > 1)
		{
			INode* childR = tree.top();
			tree.pop();

			INode* childL = tree.top();
			tree.pop();

			INode* parent = new InternalNode(childR, childL);
			tree.push(parent);
		}
		return tree.top();
	}

	void GenerateHuffmanCodes(const INode* node, const HuffmanCode& prefix, HuffCodeMapping& outCodes)
	{
		if (const LeafNode* lf = dynamic_cast<const LeafNode*>(node))
		{
			outCodes[lf->c] = prefix;
		}
		else if (const InternalNode* in = dynamic_cast<const InternalNode*>(node))
		{
			HuffmanCode leftPrefix = prefix;
			leftPrefix.push_back(false);
			GenerateHuffmanCodes(in->left, leftPrefix, outCodes);

			HuffmanCode rightPrefix = prefix;
			rightPrefix.push_back(true);
			GenerateHuffmanCodes(in->right, rightPrefix, outCodes);
		}
	}
public:

	int compressFile(const std::string& inputPath, const std::string& outputPath)
	{
		// Build frequency table
		int frequencies[UniqueSymbols] = { 0 };
		std::ifstream is(inputPath, std::ios_base::binary);

		if (!is.is_open())
			return EXIT_FAILURE;

		unsigned char data;
		while (is.get(reinterpret_cast<char&>(data)))
			++frequencies[data];

		INode* treeRoot = BuildHuffmanTree(frequencies);

		HuffCodeMapping codes;
		GenerateHuffmanCodes(treeRoot, HuffmanCode(), codes);
		delete treeRoot;
		is.clear();
		is.seekg(0, std::ios::beg);

		BitFileManager os(BitFileManager::Mode::Write, outputPath);
		addHeader(os.getOStream());
		for (HuffCodeMapping::const_iterator it = codes.begin(); it != codes.end(); ++it)
		{
			unsigned char data = static_cast<unsigned char>(it->first);
			os.write(data);

			std::vector<bool> coded = codes[data];
			for (const auto& it : coded)
				it == true ? os.write('1') : os.write('0');
		}
		os.write((char)0);

		while (is.get(reinterpret_cast<char&>(data)))
		{
			std::vector<bool> coded = codes[data];
			for (const auto& it : coded)
				os.write(it);
		}

		is.close();

		return EXIT_SUCCESS;
	}

	int decompressFile(const std::string& inputPath, const std::string& outputPath)
	{
		// Build frequency table
		std::ofstream os(outputPath, std::ios_base::binary);
		if (!os.is_open())
			return EXIT_FAILURE;

		BitFileManager is(BitFileManager::Mode::Read, inputPath);
		if(!checkHeader(is.getIStream()))
			return EXIT_FAILURE;

		HuffDecodeMapping decodes;
		unsigned char data;

		for (is.read(data); data != 0;)
		{
			unsigned char code;
			std::vector<bool> enc;
			for (is.read(code); code == '0' || code == '1'; is.read(code))
				enc.push_back(code == '0' ? false : true);

			decodes[enc] = data;
			data = code;
		}

		bool bit;
		std::vector<bool> enc;
		while (is.read(bit) == EXIT_SUCCESS)
		{
			enc.push_back(bit);

			auto it = decodes.find(enc);
			if (it != decodes.end())
			{
				enc.clear();
				os.write(reinterpret_cast<const char *> (&it->second), sizeof (char));
			}
		}

		os.close();
		
		return EXIT_SUCCESS;
	}

	Huffman(char key): BaseCompression(key)
	{};
};