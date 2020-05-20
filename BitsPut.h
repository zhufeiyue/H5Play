#pragma once

#include "PrintLog.h"

class BitsPut
{
public:
	BitsPut(int bitslen);
	~BitsPut();
	int PutBit(bool on);
	int PutBits(int len, int value);
	void ResetBits();
	unsigned char* GetBitsData();

protected:
	unsigned char* pBits;
	int allLens;
	int currentPos;
};

class BitsGet
{
public:
	BitsGet(unsigned char*);
	int GetBits(int len);
	int GetBit();
protected:
	const unsigned char* pBits;
	int currentPos;
};
