#include "DashConvert.h"
//#include <fstream>
#include <algorithm>
#include <cmath>

DashConvert::DashConvert():
	m_flvdatabuflen(0),
	m_flvdatabufused(0),
	m_datapersecond(0),
	m_gotVideoAVCHead(0),
	m_gotAudioAACHead(0),
	m_gotInitSeg(0),
	m_gotNormalSeg(0),
	m_keyframeNumber(0),

	m_initSegLen(0),
	m_normalSegNo(1),
	m_normalSegVideoSize(0),
	m_normalSegAudioSize(0),
	m_lastNormalSegVideoSize(0),
	m_lastNormalSegAudioSize(0),
	m_normalSegMoofLen(0),
	m_normalSegMdatLen(0),
	m_segDurationA(0),
	m_segDurationV(0),
	m_tfDTV(0),
	m_tfDTA(0)
{
}

DashConvert::~DashConvert()
{
}

int DashConvert::GetDataRate()
{
	return m_datapersecond;
}

double DashConvert::GetDuration()
{
	return m_flvParams.scriptTag.duration;
}

int DashConvert::GetInitSeg(char* buf, int buflen)
{
	if (!m_gotInitSeg)
	{
		return 0;
	}

	if (!buf || buflen < m_initSegLen + 24)
	{
		return -(m_initSegLen + 24);
	}

	memcpy(buf, m_pInitSeg.get(), m_initSegLen + 24);
	return m_initSegLen + 24;
}

int DashConvert::GetNormalSeg(char* buf, int buflen)
{
	if (m_gotNormalSeg < 1)
		return 0;
	if (!buf || (buflen < (m_normalSegMoofLen + m_normalSegMdatLen)))
		return -(m_normalSegMoofLen + m_normalSegMdatLen);

	int n(0), res(-1);
	//uint32_t last;

	n += m_muxer.Create_moof(buf, buflen, 0);
	n += m_muxer.Create_BoxHead(buf + n, m_normalSegMdatLen, "mdat");

	if (m_lastNormalSegVideoSize != m_pVideoBuf->ReadData(buf + n, m_lastNormalSegVideoSize))
	{
		throw std::logic_error("read cached video sample error(create mdat).");
	}
	n += m_lastNormalSegVideoSize;
	if (m_lastNormalSegAudioSize != m_pAudioBuf->ReadData(buf + n, m_lastNormalSegAudioSize))
	{
		throw std::logic_error("read cached audio sample error(create mdat).");
	}
	n += m_lastNormalSegAudioSize;

	if (n != m_normalSegMoofLen + m_normalSegMdatLen)
		goto End;

	res = n;
	m_tfDTA = 0;
	--m_keyframeNumber;

	//last = m_muxer.muxerParams.moof.traf_video.trun_samplesize.back();
	m_muxer.muxerParams.moof.traf_video.trun_samplesize.clear();
	//m_muxer.muxerParams.moof.traf_video.trun_samplesize.push_back(last);

	//last = m_muxer.muxerParams.moof.traf_video.trun_sampleflags1.back();
	m_muxer.muxerParams.moof.traf_video.trun_sampleflags1.clear();
	//m_muxer.muxerParams.moof.traf_video.trun_sampleflags1.push_back(last);

	//last = m_muxer.muxerParams.moof.traf_video.trun_samplecompositiontimeoffset.back();
	m_muxer.muxerParams.moof.traf_video.trun_samplecompositiontimeoffset.clear();
	//m_muxer.muxerParams.moof.traf_video.trun_samplecompositiontimeoffset.push_back(last);

	m_muxer.muxerParams.moof.traf_audio.trun_samplesize.clear();
	m_normalSegAudioSize -= m_lastNormalSegAudioSize;
	m_normalSegVideoSize -= m_lastNormalSegVideoSize;
	
	--m_gotNormalSeg;
End:
	return res;
}

double DashConvert::GetNormalSegDuration()
{
	return m_segDurationA;
}

int DashConvert::GetLastSeg(char* buf, int buflen)
{
	if (m_keyframeNumber > 0)
	{
		TRACE0("process the last normal seg");
		if (m_gotNormalSeg < 1)
			CreateNormalSeg();
		return GetNormalSeg(buf, buflen);
	}

	TRACE0("no normal seg left");
	return 0;
}

void DashConvert::Clean()
{
	m_pInitSeg.reset(NULL);
	m_pFlvDataBuffer.reset(NULL);
	m_flvdatabuflen = 0;
	m_flvdatabufused = 0;
	m_gotNormalSeg = 0;
	m_gotInitSeg = 0;
}

int DashConvert::CachedFlvLen()
{
	return m_flvdatabufused;
}

void DashConvert::GetFrameSize(int&w, int& h)
{
	w = (int)m_flvParams.scriptTag.width;
	h = (int)m_flvParams.scriptTag.height;
}

void DashConvert::GetPixelAspectRatio(int&w, int&h)
{
	H264SPSInfo sps;
	ParseSPS(sps,
		(unsigned char*)m_flvParams.videoTag.avc_seq_head.avc_seq_head_data + 4, // sps
		m_flvParams.videoTag.avc_seq_head.avc_seq_head_datalen - 4); // spssize+4+ppssize, unnecessary to get spssize alone

	if (sps.aspect_ratio_info_present_flag == -1 || sps.aspect_ratio_info_present_flag == 0) // unknown or no sample aspect ratio info
	{
		w = 0;
		h = 0;
	}
	else if (sps.aspect_ratio_info_present_flag == 1)
	{
		w = sps.sar_width;
		h = sps.sar_height;
	}
}

int DashConvert::AppendData(const char* pdata, int datalen)
{
	DashConvertStatus s(DashConvertStatus::DCUnknow);

	int used(0), len(0);
	const char* pd;

	JoinPredata(pdata, datalen);
	pd = m_pFlvDataBuffer.get();
	len = m_flvdatabufused;

	if (m_gotNormalSeg > 0)
	{
		MoveLeftData(used);
		return DCGotNormalFragment;
	}

	m_fp.InputData(pd, len, m_flvParams);

CheckParseRes:

	if (m_flvParams.type > FlvPartNone)
	{
		switch (m_flvParams.type)
		{
		case FlvPartAudioTag:
			HandleAudioData();
			break;
		case FlvPartVideoTag:
			HandleVideoData();
			break;
		case FlvPartScriptTag:
			HandleMetaData();
			break;
		case FlvPartFileHead:
			TRACE0("got file head, continue");
			break;
		default:
			//never
			return s;
			break;
		}

		used += m_flvParams.m_iUseDataLen;
		pd += m_flvParams.m_iUseDataLen;
		len -= m_flvParams.m_iUseDataLen;

		if (m_gotNormalSeg)
		{
			MoveLeftData(used);
			return DCGotNormalFragment;
		}
		else if (m_gotInitSeg > 0)
		{
			m_gotInitSeg *= -1;
			MoveLeftData(used);
			return DCGotInitlFragment;
		}

		m_fp.InputData(pd, len, m_flvParams);
		goto CheckParseRes;
	}
	else
	{
		if (m_flvParams.m_bNeedMoreData)
		{
			//TRACE0("need more data");
			MoveLeftData(used);
			s = DashConvertStatus::DCNeedMoreData;
		}
		else if (m_flvParams.m_bNotFlv)
		{
			TRACE0("not a flv!!!");
			s = DashConvertStatus::DCNotFlv;
		}
		else if (m_flvParams.m_bException) 
		{
			TRACE0("exception happened");
			s = DashConvertStatus::DCErrorHappen;
		}
	}

	return s;
}

void DashConvert::HandleMetaData()
{
	if (m_flvParams.scriptTag.onlastsecond)
	{
		return;
	}
	if (m_flvParams.flvHead.flag != 5)
	{
		throw 1;
	}

	if (m_flvParams.scriptTag.height < 1 ||
		m_flvParams.scriptTag.width < 1 ||
		m_flvParams.scriptTag.duration < 1)
	{
		throw 1;
	}

	if (m_flvParams.scriptTag.filesize < 1)
	{
		if (m_flvParams.scriptTag.videosize < 1)
		{
			if (m_flvParams.scriptTag.videodatarate < 1)
			{
				throw 1;
			}
			else
			{
				m_flvParams.scriptTag.videosize = m_flvParams.scriptTag.videodatarate*m_flvParams.scriptTag.duration ;
			}
		}

		if (m_flvParams.scriptTag.audiosize < 1)
		{
			if (m_flvParams.scriptTag.audiodatarate < 1)
				throw 1;
			else
			{
				m_flvParams.scriptTag.audiosize = m_flvParams.scriptTag.audiodatarate*m_flvParams.scriptTag.duration ;
			}
		}

		m_flvParams.scriptTag.filesize = (m_flvParams.scriptTag.audiosize + m_flvParams.scriptTag.videosize) * 1000 / 8;
	}

	if ((int)m_flvParams.scriptTag.videocodec != 7 || (int)m_flvParams.scriptTag.audiocodec != 10)
	{
		throw std::logic_error("need h264+aac");
	}

	if (m_flvParams.scriptTag.m_vecPos.empty())
	{
		m_datapersecond = static_cast<int> (m_flvParams.scriptTag.filesize / m_flvParams.scriptTag.duration);
	}
	else
	{
		m_datapersecond = static_cast<int>(m_flvParams.scriptTag.filesize / (m_flvParams.scriptTag.m_vecPos.size() - 1));
	}
	m_pAudioBuf.reset(new RingBuffer(m_datapersecond / 4));
	m_pVideoBuf.reset(new RingBuffer(m_datapersecond / 4 * 6));
}

void DashConvert::HandleVideoData()
{
	if (m_flvParams.videoTag.avc_packet_type == 1)
	{
		if (m_flvParams.videoTag.videoFrameType != 1)
		{
			if (m_keyframeNumber < 1)
				return;
		}
		else
		{
			++m_keyframeNumber;
			if (m_keyframeNumber > 1)
			{
				CreateNormalSeg();
				--m_keyframeNumber;
				m_flvParams.m_iUseDataLen = 0;
				//TRACE0("dont handle this i frame");
				return;
			}
			m_tfDTV = static_cast<uint32_t>(std::round(m_flvParams.tagHead.timestamp*m_flvParams.scriptTag.framerate));
		}

		int videolen = m_flvParams.videoTag.datalen - 4;
		if (m_pVideoBuf->WriteableBufferLen() < videolen)
		{
			int newsize = m_pVideoBuf->TotalBufferLen() + std::max(videolen, m_datapersecond / 2);
			//TRACE4("resize video buf: old buf len is %d, writeable len is %d, incoming len is %d, new size is %d", m_pVideoBuf->TotalBufferLen(),
			//	m_pVideoBuf->WriteableBufferLen(),
			//	videolen,
			//	newsize);
			if (!m_pVideoBuf->Resize(newsize))
			{
				throw std::logic_error("cannt resize video sample buf");
			}
		}

		if (m_pVideoBuf->WriteData(m_flvParams.videoTag.data + 4, videolen) != videolen)
		{
			throw std::logic_error("failed to write video sample buf");
		}

		m_normalSegVideoSize += videolen;
		m_muxer.muxerParams.moof.traf_video.trun_samplesize.push_back(videolen);
		m_muxer.muxerParams.moof.traf_video.trun_samplecompositiontimeoffset.
			push_back(m_flvParams.videoTag.avc_nalu_compositiontime*static_cast<uint32_t>(m_flvParams.scriptTag.framerate));
		if (m_flvParams.videoTag.videoFrameType == 1)
		{
			m_muxer.muxerParams.moof.traf_video.trun_sampleflags1.push_back(0x00000000);
		}
		else
		{
			m_muxer.muxerParams.moof.traf_video.trun_sampleflags1.push_back(0x00010000);
		}

	}
	else if (m_flvParams.videoTag.avc_packet_type == 0)
	{
		m_gotVideoAVCHead = 1;
		memcpy(m_muxer.avcHeadData, m_flvParams.videoTag.avc_seq_head_org, m_flvParams.videoTag.avc_seq_head_org_len);
		m_muxer.avcHeadLen = m_flvParams.videoTag.avc_seq_head_org_len;
		m_muxer.muxerParams.moov.track_video.trackid = 1;
		m_muxer.muxerParams.moov.track_video.tkhd_duration = 0;
		m_muxer.muxerParams.moov.track_video.tkhd_height = static_cast<float>(m_flvParams.scriptTag.height);
		m_muxer.muxerParams.moov.track_video.tkhd_width = static_cast<float>(m_flvParams.scriptTag.width);
		m_flvParams.scriptTag.framerate = std::round(m_flvParams.scriptTag.framerate);
		m_muxer.muxerParams.moov.track_video.mdhd_timescale = static_cast<uint32_t>(m_flvParams.scriptTag.framerate * 1000);
		m_muxer.muxerParams.moov.track_video.mdhd_duration = 0;
		memcpy(m_muxer.muxerParams.moov.track_video.hdlr_type, "vide", 4);
		m_muxer.muxerParams.moov.track_video.stsd_v_width = static_cast<uint16_t>(m_flvParams.scriptTag.width);
		m_muxer.muxerParams.moov.track_video.stsd_v_height = static_cast<uint16_t>(m_flvParams.scriptTag.height);
		m_muxer.muxerParams.moof.traf_video.tfhd_trackid = 1;
		m_muxer.muxerParams.moof.traf_video.tfhd_flags = 0x020000;
		m_muxer.muxerParams.moof.traf_video.trun_flags = 0x0e01;

		if (m_gotAudioAACHead)
		{
			CreateInitSeg();
		}
	}
	else if (m_flvParams.videoTag.avc_packet_type == 2)
	{
		TRACE0("avc seq end");
	}
}

void DashConvert::HandleAudioData()
{
	if (*m_flvParams.audioTag.data == 1)
	{
		int audiolen = m_flvParams.audioTag.datalen - 1;
		if (m_pAudioBuf->WriteableBufferLen() < audiolen)
		{
			TRACE3("resize audio buf: old buf len is %d, writeable len is %d, incoming len is %d", m_pAudioBuf->TotalBufferLen(),
				m_pAudioBuf->WriteableBufferLen(),
				audiolen);
			if (!m_pAudioBuf->Resize(m_pAudioBuf->TotalBufferLen() + std::max(audiolen, m_datapersecond / 4)))
			{
				throw std::logic_error("cannt resize audio sample buf");
			}
		}

		if (m_pAudioBuf->WriteData(m_flvParams.audioTag.data + 1, audiolen) != audiolen)
		{
			throw std::logic_error("failed to write audio sample buf");
		}

		m_normalSegAudioSize += audiolen;
		m_muxer.muxerParams.moof.traf_audio.trun_samplesize.push_back(audiolen);

		if (m_tfDTA < 1)
		{
			m_tfDTA = static_cast<uint32_t>(m_flvParams.tagHead.timestamp*(m_flvParams.scriptTag.samplerate / 1000));
		}
	}
	else
	{
		m_gotAudioAACHead = 1;
		memcpy(m_muxer.aacHeadData, m_flvParams.audioTag.aac_seq_head_org, 2);
		m_muxer.aacHeadLen = 2;
		m_muxer.muxerParams.moov.track_audio.trackid = 2;
		m_muxer.muxerParams.moov.track_audio.tkhd_duration = 0;
		m_muxer.muxerParams.moov.track_audio.tkhd_height = 0;
		m_muxer.muxerParams.moov.track_audio.tkhd_width = 0;
		m_muxer.muxerParams.moov.track_audio.mdhd_timescale = m_flvParams.audioTag.aac_seq_head.aac_seq_head_samplingFrequency;
		m_muxer.muxerParams.moov.track_audio.mdhd_duration = 0;
		memcpy(m_muxer.muxerParams.moov.track_audio.hdlr_type, "soun", 4);
		m_muxer.muxerParams.moov.track_audio.stsd_a_channelcount = m_flvParams.scriptTag.stereo ? 2 : 1;
		m_muxer.muxerParams.moov.track_audio.stsd_a_samplesize = 16;
		m_muxer.muxerParams.moov.track_audio.stsd_a_samplerate = m_flvParams.audioTag.aac_seq_head.aac_seq_head_samplingFrequency;
		m_muxer.muxerParams.moof.traf_audio.tfhd_trackid = 2;
		m_muxer.muxerParams.moof.traf_audio.tfhd_flags = 0x020000;
		m_muxer.muxerParams.moof.traf_audio.trun_flags = 0x0201;

		if (m_gotVideoAVCHead)
		{
			CreateInitSeg();
		}
	}
}

void DashConvert::JoinPredata(const char* p, int len)
{
	if (m_flvdatabufused + len > m_flvdatabuflen)
	{
		int newsize = (m_flvdatabuflen * 1.5) > (m_flvdatabufused + len) ? (m_flvdatabuflen * 1.5) : (m_flvdatabufused + len);
		//TRACE2("resize flv raw data buf size(old %d, new %d)", m_flvdatabuflen, newsize);

		char * temp = new char[newsize];
		memcpy(temp, m_pFlvDataBuffer.get(), m_flvdatabufused);
		memcpy(temp + m_flvdatabufused, p, len);
		m_pFlvDataBuffer.reset(temp);
		m_flvdatabuflen = newsize;
		m_flvdatabufused = m_flvdatabufused + len;
	}
	else
	{
		memcpy(m_pFlvDataBuffer.get() + m_flvdatabufused, p, len);
		m_flvdatabufused += len;
	}
}

void DashConvert::MoveLeftData(int used)
{
	m_flvdatabufused -= used;
	if (used > 0 && m_flvdatabufused)
	{
		memmove(m_pFlvDataBuffer.get(), m_pFlvDataBuffer.get() + used, m_flvdatabufused);
	}
}

void DashConvert::CreateInitSeg()
{
	if (m_gotInitSeg)
	{
		throw std::logic_error("no init seg");
	}

	m_muxer.muxerParams.moov.mvhd_timescale = 90000;
	m_muxer.muxerParams.moov.mvhd_duration = 0;
	m_muxer.muxerParams.moov.mvex.mehd_fragmentduration = static_cast<uint64_t>(90000 * m_flvParams.scriptTag.duration);

	m_initSegLen = m_muxer.Create_moov(0, 0);
	m_pInitSeg.reset(new char[m_initSegLen + 32]);

	m_muxer.Create_ftyp(m_pInitSeg.get(), 24);
	m_muxer.Create_moov(m_pInitSeg.get() + 24, m_initSegLen);

	//std::ofstream fileOut;
	//fileOut.open("d:\\test.mp4", std::ofstream::binary | std::ofstream::out);
	//fileOut.write(m_pInitSeg.get(), m_initSegLen+24);
	//fileOut.close();

	m_gotInitSeg = 1;
}

void DashConvert::CreateNormalSeg()
{
	++m_gotNormalSeg;

	m_lastNormalSegAudioSize = m_normalSegAudioSize;
	m_lastNormalSegVideoSize = m_normalSegVideoSize;
	m_muxer.muxerParams.moof.traf_video.trun_samplecount = int(m_muxer.muxerParams.moof.traf_video.trun_samplesize.size());
	m_muxer.muxerParams.moof.traf_audio.trun_samplecount = int(m_muxer.muxerParams.moof.traf_audio.trun_samplesize.size());
	m_normalSegMoofLen = m_muxer.Create_moof(0, 0, 0);
	m_normalSegMdatLen =  8 + m_lastNormalSegVideoSize + m_lastNormalSegAudioSize;
	m_muxer.muxerParams.moof.traf_video.trun_dataoffset = m_normalSegMoofLen + 8;
	m_muxer.muxerParams.moof.traf_audio.trun_dataoffset = m_normalSegMoofLen + 8 + m_lastNormalSegVideoSize;
	m_muxer.muxerParams.moof.mfhd_seqno = m_normalSegNo++;

	m_segDurationV = m_muxer.muxerParams.moof.traf_video.trun_samplecount*1.0 / m_flvParams.scriptTag.framerate;
	m_segDurationA = 1024.0* m_muxer.muxerParams.moof.traf_audio.trun_samplecount / m_flvParams.scriptTag.samplerate;

	m_muxer.muxerParams.moof.traf_video.tfdt_basemediadecodetime = m_tfDTV;
	m_muxer.muxerParams.moof.traf_audio.tfdt_basemediadecodetime = m_tfDTA;
}

DashConvertGOP::DashConvertGOP():
	m_iSegNumber(0),
	m_iCurrentProcessSegIndex(1),
	m_iCurrentProcessSegStartPos(0),
	m_iCurrentProcessSegSize(-1)
{
}

int DashConvertGOP::AppendData(const char*pdata, int datalen)
{
	DashConvertStatus s(DashConvertStatus::DCUnknow);

	int used(0), len(0);
	const char* pd;

	JoinPredata(pdata, datalen);
	pd = m_pFlvDataBuffer.get();
	len = m_flvdatabufused;

	if (m_gotNormalSeg > 0)
	{
		MoveLeftData(used);
		return DCGotNormalFragment;
	}

	m_fp.InputData(pd, len, m_flvParams);

CheckParseRes:

	if (m_flvParams.type > FlvPartNone)
	{
		switch (m_flvParams.type)
		{
		case FlvPartAudioTag:
			HandleAudioData();
			m_iCurrentProcessSegSize -= m_flvParams.m_iUseDataLen;
			break;
		case FlvPartVideoTag:
			HandleVideoData();
			m_iCurrentProcessSegSize -= m_flvParams.m_iUseDataLen;
			break;
		case FlvPartScriptTag:
			HandleMetaData();
			// first one is avc seq head
			// last one is avc seq end(maybe)
			m_iSegNumber = static_cast<int>(m_flvParams.scriptTag.m_vecPos.size() - 1);
			m_iCurrentProcessSegSize -= m_flvParams.m_iUseDataLen;
			break;
		case FlvPartFileHead:
			//TRACE0("got file head, continue");
			break;
		default:
			// never
			return s;
			break;
		}

		used += m_flvParams.m_iUseDataLen;
		pd += m_flvParams.m_iUseDataLen;
		len -= m_flvParams.m_iUseDataLen;

		if (m_gotInitSeg > 0)
		{
			m_iCurrentProcessSegIndex = 1;
			CalSegPosSize(1, m_iCurrentProcessSegStartPos, m_iCurrentProcessSegSize);
			if (used != m_iCurrentProcessSegStartPos)
			{
				/*
				在第一个GOP的I帧之前存在若干audio tag；这些audio tag导致flv metadata中第一个关键帧的
				位置与计算init seg时得来的长度不一致（计算init seg 不需要这些一般的audio tag参与），因为
				默认的情况是计算完init seg后紧接着就是视频I帧（以I帧的开始计算 normal seg）
				*/

				// 1 skip those audio tag
				used = m_iCurrentProcessSegStartPos;

				// 2 handle those before return init seg
				//m_fp.InputData(pd, len, m_flvParams);
				//goto CheckParseRes;
			}
			m_gotInitSeg *= -1;
			MoveLeftData(used);
			return DCGotInitlFragment;
		}
		else if (m_iCurrentProcessSegSize == 0)
		{
			CreateNormalSeg();
			MoveLeftData(used);

			++m_iCurrentProcessSegIndex;
			CalSegPosSize(m_iCurrentProcessSegIndex, m_iCurrentProcessSegStartPos, m_iCurrentProcessSegSize);

			return DCGotNormalFragment;
		}


		m_fp.InputData(pd, len, m_flvParams);
		goto CheckParseRes;
	}
	else
	{
		if (m_flvParams.m_bNeedMoreData)
		{
			//TRACE0("need more data");
			MoveLeftData(used);
			s = DashConvertStatus::DCNeedMoreData;
		}
		else if (m_flvParams.m_bNotFlv)
		{
			TRACE0("not a flv!!!");
			s = DashConvertStatus::DCNotFlv;
		}
		else if (m_flvParams.m_bException)
		{
			TRACE0("exception happened");
			s = DashConvertStatus::DCErrorHappen;
		}
	}

	return s;
}

int DashConvertGOP::UpdateTimeRange(float* tr, int n)
{
	m_timerangeplaying.clear();
	for (int i = 0; i < n; ++i)
	{
		m_timerangeplaying.push_back(*(tr + i));
	}
	return 0;
}

int DashConvertGOP::IsSegInTimeRange(int segIndex)
{
	int res(0);
	size_t ss(0), es(0);
	size_t i, j;
	double f, temp, temp1;;

	for (i = 0; i < m_timerangeplaying.size() / 2; ++i)
	{
		f = m_timerangeplaying[i * 2];
		temp = 100.0;

		for (j = 1; j < m_flvParams.scriptTag.m_vecTime.size() - 1; ++j)
		{
			temp1 = std::abs(m_flvParams.scriptTag.m_vecTime[j] - f);
			if (temp1 < temp)
			{
				ss = j;
				temp = temp1;
			}
		}
		//TRACE2("s in is %d, wc %lf", ss, temp);
		if (segIndex < ss)
		{
			break;
		}

		f = m_timerangeplaying[i * 2 + 1];
		temp = 100.0;
		for (j = 2; j < m_flvParams.scriptTag.m_vecTime.size(); ++j)
		{
			temp1 = std::abs(m_flvParams.scriptTag.m_vecTime[j] - f);
			if (temp1 < temp)
			{
				es = j;
				temp = temp1;
			}
		}
		//TRACE2("e in is %d, wc %lf", es, temp);


		//if (!(ss < es)) 
		//{
		//	throw std::logic_error("cal seg index  by time range error");
		//}

		
		if (segIndex < es) 
		{
			res = es;
			break;
		}

		else if (segIndex == es)
		{
			break;
		}
	}


	return res;
}

int DashConvertGOP::GetSegIndexByTime(double t)
{
	size_t i;

	for (i = 1; i < m_flvParams.scriptTag.m_vecTime.size() - 1; ++i)
	{
		if (t >= m_flvParams.scriptTag.m_vecTime[i] 
			&& t < m_flvParams.scriptTag.m_vecTime[i + 1])
		{
			return (int)i;
		}
	}

	return 0;
}

void DashConvertGOP::SeekBySegIndex(int seg)
{
	if (seg != m_iCurrentProcessSegIndex)
	{
		TRACE0("seek reset");
		m_flvdatabufused = 0;
		m_normalSegNo = seg;
		m_iCurrentProcessSegIndex = seg;
		CalSegPosSize(seg, m_iCurrentProcessSegStartPos, m_iCurrentProcessSegSize);

		m_pAudioBuf->Reset();
		m_pVideoBuf->Reset();
		m_normalSegVideoSize = 0;
		m_normalSegAudioSize = 0;

		return;
	}
	
	//TRACE0("no need to seek reset");
}

void DashConvertGOP::GetSegReqPosReqBufLen(int i, int& pos, int& datalen)
{
	if (i < 1 || (size_t)i >= m_flvParams.scriptTag.m_vecPos.size())
	{
		throw std::logic_error("seg index error");
		return;
	}

	if (i !=  m_iCurrentProcessSegIndex)
	{
		CalSegPosSize(i, pos, datalen);
	}
	else
	{
		CalSegPosSize(i, pos, datalen);
		if (m_flvdatabufused > 0)
		{
			//TRACE1("left seg data is %d", m_flvdatabufused);
			pos += m_flvdatabufused;
			datalen -= m_flvdatabufused;
		}
	}
}

int DashConvertGOP::GetSegNumber()
{
	return m_iSegNumber;
}

void DashConvertGOP::CalSegPosSize(int i, int& pos, int& len)
{
	pos = m_flvParams.scriptTag.m_vecPos[i];
	if (i + 1 < m_flvParams.scriptTag.m_vecPos.size())
	{
		len = m_flvParams.scriptTag.m_vecPos[i + 1] - pos;
	}
	else
	{
		len = 0;
	}
	pos -= 4;
}