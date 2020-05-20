#pragma once

#include <memory>
#include "FlvParse.h"
#include "Mp4Parse.h"
#include "RingBuffer.h"

class DashConvert
{
public:
	enum DashConvertStatus
	{
		DCUnknow,
		DCErrorHappen,
		DCNotFlv,
		DCNeedMoreData,
		DCGotInitlFragment,
		DCGotNormalFragment
	};

public:
	DashConvert();
	virtual ~DashConvert();
	void Clean();
	int CachedFlvLen();
	virtual int AppendData(const char*, int);
	int GetDataRate();
	double GetDuration();
	int GetInitSeg(char*, int);
	int GetNormalSeg(char*, int);
	double GetNormalSegDuration();
	int GetLastSeg(char*, int);
	void GetFrameSize(int&w, int& h);
	void GetPixelAspectRatio(int&w, int&h);
protected:
	void HandleMetaData();
	void HandleVideoData();
	void HandleAudioData();

	virtual void JoinPredata (const char*, int);
	void MoveLeftData(int);

	void CreateInitSeg();
	void CreateNormalSeg();
protected:
	std::unique_ptr<char[]> m_pFlvDataBuffer;
	int m_flvdatabuflen;
	int m_flvdatabufused;
	int m_datapersecond;
	FlvParseRes m_flvParams;
	FlvParse m_fp;
	int m_gotVideoAVCHead;
	int m_gotAudioAACHead;
	int m_gotInitSeg;
	int m_gotNormalSeg;
	int m_keyframeNumber;

	std::unique_ptr<RingBuffer> m_pVideoBuf;
	std::unique_ptr<RingBuffer> m_pAudioBuf;
	Mp4Muxer m_muxer;
	std::unique_ptr<char[]> m_pInitSeg;
	int32_t m_initSegLen;
	int32_t m_normalSegNo;
	int32_t m_normalSegVideoSize;
	int32_t m_normalSegAudioSize;
	int32_t m_lastNormalSegVideoSize;
	int32_t m_lastNormalSegAudioSize;
	int32_t m_normalSegMoofLen;
	int32_t m_normalSegMdatLen;
	double m_segDurationV;
	double m_segDurationA;
	uint32_t m_tfDTV;
	uint32_t m_tfDTA;
};

class DashConvertGOP : public DashConvert
{
public:
	DashConvertGOP();

public:
	int AppendData(const char*, int);
	int UpdateTimeRange(float*, int);
	int IsSegInTimeRange(int);
	int GetSegIndexByTime(double t);
	void SeekBySegIndex(int);
	virtual void GetSegReqPosReqBufLen(int, int&, int&);
	int GetSegNumber();
protected:
	// new function
	void CalSegPosSize(int, int&, int&);

protected:
	int m_iSegNumber;
	int m_iCurrentProcessSegIndex;
	int m_iCurrentProcessSegStartPos;
	int m_iCurrentProcessSegSize;
	std::vector<float> m_timerangeplaying;
};
