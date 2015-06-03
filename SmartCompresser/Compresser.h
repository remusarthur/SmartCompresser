#include "stdafx.h"

class Compresser
{

public:
	virtual int compress(const std::string& inputFile, const std::string& outputFle) = 0;
};