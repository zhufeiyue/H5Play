#include "BitsPut.h"
#include <string>
#include <iostream>

BitsPut::BitsPut(int bitslen):
	pBits(0),
	allLens(bitslen),
	currentPos(0)
{
	int n;

	if (bitslen % 8 == 0)
	{
		n = bitslen / 8;
	}
	else
	{
		n = bitslen / 8 + 1;
	}

	pBits = new unsigned char[n];
	memset(pBits, 0, n);
	//TRACE1("create mem for %d bits", n);
}

BitsPut::~BitsPut()
{
	if (pBits)
	{
		delete[]pBits;
	}
}

int BitsPut::PutBit(bool on)
{
	unsigned char*pdest, ch=1;
	int yu = currentPos % 8;

	pdest = pBits + currentPos / 8;

	ch = ch << (7 - yu);
	if (on)
	{
		*pdest = (*pdest) | ch;
	}
	else
	{
		*pdest = (*pdest) & (~ch);
	}
	++currentPos;

	return currentPos;
}

int BitsPut::PutBits(int len, int value)
{
	if (len > 32 || len <1)
		return currentPos;

	int n, yu, i(0);
	unsigned char* ptemp = (unsigned char*)&value, *psrc, ch;

	yu = len % 8;
	if (yu == 0)
	{
		n = len / 8;
		yu = 8;
	}
	else
	{
		n = len < 8 ? 1 : len / 8 + 1;
	}

	psrc = ptemp + n - 1;

	do
	{
		for (int j = yu-1; j >=0; --j)
		{
			ch = 1;
			ch = ch << j;
			ch = (*psrc)&ch;
			//std::cout << (ch ? '1' : '0');
			PutBit(ch ? true : false);
		}
		yu = 8;
		--psrc;
	} while (++i < n);
	//std::cout << std::endl;

	return 0;
}

void BitsPut::ResetBits()
{
	currentPos = 0;
}

unsigned char* BitsPut::GetBitsData()
{
	return pBits;
}

BitsGet::BitsGet(unsigned char* p):
	pBits(p),
	currentPos(0)
{
}

int BitsGet::GetBits(int bitslen)
{
	int n(0);
	for (int i = 0; i < bitslen; ++i)
	{
		n += GetBit()*(1 << (bitslen - 1 - i));
	}

	return n;
}

int BitsGet::GetBit()
{
	const unsigned char *pdest;
	unsigned char ch(1);
	int yu = currentPos % 8;

	pdest = pBits + currentPos / 8;
	ch = ch << (7 - yu);

	++currentPos;
	if ((*pdest) & ch)
	{
		return 1;
	}

	return 0;
}