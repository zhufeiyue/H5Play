#pragma once

class  RingBuffer
{
public:
	RingBuffer(int len, char* pOutsidebuf = 0, int UseableLen = 0, int UseableLenOffset = 0);
	~RingBuffer();
	bool IsEmpty() { return m_iRead == m_iWrite && !m_bFull; }
	bool IsFull() { return m_bFull; }
	int TotalBufferLen() { return m_iLen; }
	int ReadableBufferLen() { return m_iUsableLen; }
	int WriteableBufferLen() { return m_iLen - m_iUsableLen; }
	char* Data() { return m_pBuf; }
	void Reset() { m_iWrite = m_iRead = m_iUsableLen = 0; m_bFull = false; }
	int WriteData(const char* pData, int len);
	int ReadData(char* rcvbuf, int want);

	void SaveIndexState();
	void RestoreIndexState();
	int TransferData(RingBuffer* pDest, int want);
	bool Resize(int newsize);
	RingBuffer& operator=(RingBuffer&& rr);
	RingBuffer(RingBuffer&) = delete;
	RingBuffer& operator=(RingBuffer&) = delete;
protected:
	char* m_pBuf;
	int m_iLen;
	int m_iUsableLen;
	int m_iWrite;
	int m_iRead;
	bool m_bFull;
	bool m_bUseOutsideBuf;
	int* m_pSavedIndex;
};
