#include "RingBuffer.h"
#include <cassert>
#include <iostream>
#ifdef __EMSCRIPTEN__
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#else
#include <Windows.h>
#endif

RingBuffer::RingBuffer(int len, char* pOutsidebuf, int iUseableLen, int iUseableOff)
	:m_iLen(len),
	m_iUsableLen(0),
	m_iWrite(0),
	m_iRead(0),
	m_bFull(false),
	m_bUseOutsideBuf(false),
	m_pSavedIndex(NULL)
{
	assert(len > 0);
	if (pOutsidebuf)
	{
		m_pBuf = pOutsidebuf;
		m_bUseOutsideBuf = true;
		if (iUseableLen > 0 && iUseableLen <=len)
		{
			m_iUsableLen = iUseableLen;
			if (iUseableOff > 0 && iUseableOff < len)
			{
				m_iRead = iUseableOff;
			}

			int n = m_iRead + m_iUsableLen;
			if (n < len)
			{
				m_iWrite = n;
			}
			else
			{
				m_iWrite = n%len;
				m_bFull = (m_iWrite == m_iRead);
			}
		}
	}
	else
	{
#ifdef __EMSCRIPTEN__
		m_pBuf = new char[len];
#else
		m_pBuf = (char*)VirtualAlloc(NULL, (SIZE_T)len, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if(!m_pBuf) throw std::exception("VirtualAlloc failed");
#endif
	}
}

RingBuffer::~RingBuffer()
{
	if (m_pSavedIndex)
	{
		delete[]m_pSavedIndex;
	}

	if (m_bUseOutsideBuf)
	{
		return;
	}

	if (m_pBuf)
	{
#ifdef __EMSCRIPTEN__
		delete[]m_pBuf;
#else
		VirtualFree(m_pBuf, 0, MEM_RELEASE);
#endif
	}
}

int RingBuffer::WriteData(const char* pData, int len)
{
	int n, done(0), want(len);

	if (IsFull())
		return 0;

	while (want > 0)
	{
		if (m_iWrite >= m_iRead)
		{
			n = m_iLen - m_iWrite;
			n = min(n, want);
			memcpy(m_pBuf + m_iWrite, pData + done, n);
			done += n;
			want -= n;
			m_iWrite += n;
			if (m_iWrite == m_iLen)
			{
				m_iWrite = 0;
			}
		}
		else
		{
			n = m_iRead - m_iWrite;
			n = min(n, want);
			memcpy(m_pBuf + m_iWrite, pData + done, n);
			done += n;
			want -= n;
			m_iWrite += n;
		}

		if (m_iWrite == m_iRead)
		{
			m_bFull = true;
			break;
		}
	}

	m_iUsableLen += done;
	return done;
}

int RingBuffer::ReadData(char* rcvbuf, int len)
{
	int n(0), done(0), want(len);

	if (IsEmpty())
		return 0;

	while (want > 0)
	{
		if (m_iRead >= m_iWrite)
		{
			n = m_iLen - m_iRead;
			n = min(n, want);
			memcpy(rcvbuf + done, m_pBuf + m_iRead, n);
			done += n;
			want -= n;
			m_iRead += n;
			if (m_iRead == m_iLen)
			{
				m_iRead = 0;
			}
		}
		else
		{
			n = m_iWrite - m_iRead;
			n = min(n, want);
			memcpy(rcvbuf + done, m_pBuf + m_iRead, n);
			done += n;
			want -= n;
			m_iRead += n;
		}
		m_bFull = false;
		if (m_iRead == m_iWrite)
			break;
	}

	m_iUsableLen -= done;
	return done;
}

void RingBuffer::SaveIndexState()
{
	if (!m_pSavedIndex)
	{
		m_pSavedIndex = new int[4];
	}
	m_pSavedIndex[0] = this->m_iWrite;
	m_pSavedIndex[1] = this->m_iRead;
	m_pSavedIndex[2] = this->m_iUsableLen;
	m_pSavedIndex[3] = this->m_bFull ? 1 : 0;
}

void RingBuffer::RestoreIndexState()
{
	if (!m_pSavedIndex)
		return;
	this->m_iWrite = m_pSavedIndex[0];
	this->m_iRead = m_pSavedIndex[1];
	this->m_iUsableLen = m_pSavedIndex[2];
	this->m_bFull = m_pSavedIndex[3] != 0 ? true : false;
}

int RingBuffer::TransferData(RingBuffer* pDest, int want)
{
	const int kBufLen = 8192;
	char buf[kBufLen];
	int n, m, done(0);

	n = pDest->WriteableBufferLen();
	m = this->ReadableBufferLen();
	want = min(want, n);
	want = min(want, m);

	while (want > 0)
	{
		n = min(want, kBufLen);
		m = ReadData(buf, n);
		assert(m == n);
		m = pDest->WriteData(buf, n);
		assert(m == n);
		want -= m;
		done += m;
	}

	return done;
}

bool RingBuffer::Resize(int newsize)
{
	assert(newsize > 0);
	int oldsize = TotalBufferLen();
	int transfer;

	if (m_bUseOutsideBuf)
		return false;
	if (newsize == oldsize)
		return true;
	else if (newsize < oldsize)
	{
		if (ReadableBufferLen() > newsize)
			return false;
	}

	this->SaveIndexState();

	try
	{
		RingBuffer temp(newsize);
		int u = ReadableBufferLen();
		transfer = TransferData(&temp, u);
		if (transfer == u)
		{
			operator=(std::move(temp));
			return true;
		}
		else
		{
			this->RestoreIndexState();
			return false;
		}
	}
	catch (...)
	{
		this->RestoreIndexState();
	}
	return false;
}

RingBuffer& RingBuffer::operator=(RingBuffer&& rr)
{
	if (this == &rr)
		return *this;
	if (!m_bUseOutsideBuf)
	{
		if (m_pBuf)
		{
#ifdef __EMSCRIPTEN__
			delete[]m_pBuf;
#else
			VirtualFree(m_pBuf, 0, MEM_RELEASE);
#endif
			m_pBuf = NULL;
		}
	}

	if (m_pSavedIndex)
	{
		delete[]m_pSavedIndex;
	}
	this->m_pSavedIndex = rr.m_pSavedIndex;
	this->m_pBuf = rr.m_pBuf;
	rr.m_pSavedIndex = NULL;
	rr.m_pBuf = NULL;
	this->m_iLen = rr.m_iLen;
	this->m_iRead = rr.m_iRead;
	this->m_iWrite = rr.m_iWrite;
	this->m_iUsableLen = rr.m_iUsableLen;
	this->m_bFull = rr.m_bFull;
	this->m_bUseOutsideBuf = rr.m_bUseOutsideBuf;

	return *this;
}