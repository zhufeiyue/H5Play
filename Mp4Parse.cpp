#include "Mp4Parse.h"
#include <iostream>

Mp4Parse::Mp4Parse()
{
}

Mp4Parse::~Mp4Parse()
{
}
 
void Mp4Parse::InputData(const char* pdata, int len)
{
	m_res.boxHead.type[0] = 0;
	m_res.m_iNeedMoreData = 0;
	m_res.m_iGotBox = 0;
	m_res.m_iUsedDataLen = 0;
	m_res.m_iException = 0;

	int count (0), n= ParseBoxHead(pdata, len);
	if (n < 8)
	{
		return;
	}
	count += n;
	n = m_res.boxHead.size;

	if (strncmp(m_res.boxHead.type, "ftyp", 4) == 0)
	{
		memcpy(m_res.boxHead.majorBand, pdata + count, 4);
		count += 4;
		memcpy(m_res.boxHead.minVersion, pdata + count, 4);
		count += 4;
		//char a[100] = { 0 };
		//memcpy(a, pdata + count, m_res.boxHead.size - count);
		//a[99] += 1;
	}
	else if (strncmp(m_res.boxHead.type, "moov", 4) == 0)
	{
		Parse_moov(pdata + count, len - count);
		memcpy(m_res.boxHead.type, "moov", 4);
	}
	else if (strncmp(m_res.boxHead.type, "moof", 4) == 0)
	{
		Parse_moof(pdata + count, len - count);
		memcpy(m_res.boxHead.type, "moof", 4);
	}
	else if (strncmp(m_res.boxHead.type, "sidx", 4) == 0)
	{
		// unnecessary this moment
		Mp4Sidx & sidx = m_res.sidx;
		count += 4;
		sidx.referenceID = Char4ToLittleInt(pdata + count);
		count += 4;
		sidx.timescale = Char4ToLittleInt(pdata + count);
		count += 4;
		sidx.earliest_presentation_time = Char4ToLittleInt(pdata + count);
		count += 4;
		sidx.first_offset = Char4ToLittleInt(pdata + count);
		count += 4;
		count += 2;
		sidx.reference_count = Char2ToLittleInt(pdata + count);
		count += 2;
		uint32_t temp;
		Mp4Sidx::ReferenceItem ri;
		for (uint16_t i = 0; i < sidx.reference_count; ++i)
		{
			temp = Char4ToLittleInt(pdata + count);
			ri.referencetype = (temp & 0x80000000) >> 31;
			ri.referencesize = (temp & 0x7fffffff);
			count += 4;
			ri.subsegment_duration = Char4ToLittleInt(pdata + count);
			count += 4;
			temp = Char4ToLittleInt(pdata + count);
			count += 4;
			ri.starts_width_SAP = (temp & 0x80000000) >> 31;
			ri.SAP_type = (temp & 0x70000000) >> 28;
			ri.SAP_delta_time = temp & 0x0fffffff;
			sidx.ref_items.push_back(std::move(ri));
		}
	}

	m_res.m_iGotBox = 1;
	m_res.m_iUsedDataLen = n;
}

int Mp4Parse::ParseBoxHead(const char*pdata, int len)
{
	int count(00);

	if (len < 8)
	{
		m_res.m_iNeedMoreData = 1;
		return count;
	}

	m_res.boxHead.size = Char4ToLittleInt(pdata);
	if ((uint32_t)len < m_res.boxHead.size)
	{
		m_res.m_iNeedMoreData = 1;
		return count;
	}
	count += 4;

	memcpy(m_res.boxHead.type, pdata + count, 4);
	count += 4;

	if (m_res.boxHead.size == 1)
	{
		m_res.boxHead.sizeex = Char8ToLittleInt(pdata + count);
		m_res.boxHead.size = static_cast<uint32_t>(m_res.boxHead.sizeex);
		count += 8;

		// 认为无需扩展size，也就是uint32_t 可以保存读出的sizeex
		if ((uint32_t)m_res.boxHead.size != m_res.boxHead.sizeex)
		{
			m_res.m_iException = 1;
			return 0;
		}

		if ((uint32_t)len < m_res.boxHead.size)
		{
			m_res.m_iNeedMoreData = 1;
			return 0;
		}
	}
	m_res.boxHead.boxheadsize = count;

	return count;
}

void Mp4Parse::Parse_moof(const char* pdata, int len)
{
	uint32_t moofsize = m_res.boxHead.size - m_res.boxHead.boxheadsize;
	uint32_t count(00), boxsize, boxhsize;

ReadBox:
	boxhsize = ParseBoxHead(pdata + count, len - count);
	if (boxhsize < 8)
		return;
	count += boxhsize;
	boxsize = m_res.boxHead.size;

	if (strncmp(m_res.boxHead.type, "mfhd", 4) == 0)
	{
		// version 1
		// flags 3
		count += 4;
		m_res.moof.mfhd_seqno = Char4ToLittleInt(pdata + count);
		count += 4;
	}
	else if (strncmp(m_res.boxHead.type, "traf", 4) == 0)
	{
		Mp4TrackFragment traf;
		Parse_moof_traf(pdata + count, len - count, traf);
		count += boxsize - boxhsize;

		if (traf.tfhd_trackid == m_res.moov.track_video.trackid)
		{
			m_res.moof.traf_video = std::move(traf);
		}
		else if (traf.tfhd_trackid == m_res.moov.track_audio.trackid)
		{
			m_res.moof.traf_audio = std::move(traf);
		}
	}
	else
	{
		count += boxsize - boxhsize;
	}

	if (count < moofsize)
		goto ReadBox;
}

void Mp4Parse::Parse_moof_traf(const char* pdata, int len, Mp4TrackFragment& traf)
{
	uint32_t trafsize = m_res.boxHead.size - m_res.boxHead.boxheadsize;
	uint32_t count(0), boxsize, boxhsize;

ReadBox:
	boxhsize = ParseBoxHead(pdata + count, len - count);
	if (boxhsize < 8)
		return;
	count += boxhsize;
	boxsize = m_res.boxHead.size;

	if (strncmp(m_res.boxHead.type, "tfhd", 4) == 0)
	{
		// version 1
		// flags 3
		traf.tfhd_flags = Char4ToLittleInt(pdata + count );
		count += 4;
		traf.tfhd_trackid = Char4ToLittleInt(pdata + count);
		count += 4;

		if (traf.tfhd_flags & 0x000001)
		{
			traf.tfhd_basedataoffset = Char8ToLittleInt(pdata + count);
		}
		else
		{
			traf.tfhd_basedataoffset = 0;
		}

		count += boxsize - boxhsize - 8;
	}
	else if (strncmp(m_res.boxHead.type, "tfdt", 4) == 0)
	{
		char ver = *(pdata + count);
		count += 4;

		if (ver == 0)
		{
			traf.tfdt_basemediadecodetime = (uint32_t)Char4ToLittleInt(pdata + count);
			count += 4;
		}
		else
		{
			traf.tfdt_basemediadecodetime = Char8ToLittleInt(pdata + count);
			count += 8;
		}
	}
	else if (strncmp(m_res.boxHead.type, "trun", 4) == 0)
	{
		// version 1
		// flags 3
		traf.trun_flags = Char4ToLittleInt(pdata + count);
		count += 4;
		// video samplecount
		// for audio it should be sample block count
		traf.trun_samplecount = Char4ToLittleInt(pdata + count );
		count += 4;

		if (traf.trun_flags & 0x000001)
		{
			traf.trun_dataoffset = Char4ToLittleInt(pdata + count);
			count += 4;
		}
		if (traf.trun_flags & 0x000004)
		{
			Cal_SampleFlags(pdata + count, traf.trun_firstsampleflags);
			count += 4;
		}

		Mp4SampleFlags msf;
		for (uint32_t i = 0; i < traf.trun_samplecount; ++i)
		{
			if (traf.trun_flags & 0x000100)
			{
				traf.trun_sampleduration.push_back(Char4ToLittleInt(pdata + count ));
				count += 4;
			}

			if (traf.trun_flags & 0x000200)
			{
				traf.trun_samplesize.push_back(Char4ToLittleInt(pdata + count));
				count += 4;
			}

			if (traf.trun_flags & 0x000400)
			{
				Cal_SampleFlags(pdata + count, msf);
				traf.trun_sampleflags1.push_back(Char4ToLittleInt(pdata + count));
				count += 4;
				traf.trun_sampleflags.push_back(msf);
			}

			if (traf.trun_flags & 0x000800)
			{
				traf.trun_samplecompositiontimeoffset.push_back(Char4ToLittleInt(pdata + count));
				count += 4;
			}
		}
	}
	else
	{
		count += boxsize - boxhsize;
	}

	if (count < trafsize)
		goto ReadBox;
}

void Mp4Parse::Parse_moov(const char* pdata, int len)
{
	int count(0), boxsize, boxhsize;
	int moovsize = m_res.boxHead.size - m_res.boxHead.boxheadsize;

ReadBox:
	boxhsize = ParseBoxHead(pdata+count, len-count);
	if (boxhsize < 8)
		return;
	count += boxhsize;

	if (strncmp(m_res.boxHead.type, "mvhd", 4) == 0)
	{
		m_res.moov.mvhd_version = *(pdata + count);
		count += 1;
		count += 3;

		if (m_res.moov.mvhd_version == 0)
		{
			m_res.moov.mvhd_createtime = (uint32_t)Char4ToLittleInt(pdata + count);
			count += 4;
			m_res.moov.mvhd_moditime = (uint32_t)Char4ToLittleInt(pdata + count);
			count += 4;
		}
		else
		{
			m_res.moov.mvhd_createtime = Char8ToLittleInt(pdata + count);
			count += 8;
			m_res.moov.mvhd_moditime = Char8ToLittleInt(pdata + count);
			count += 8;
		}

		m_res.moov.mvhd_timescale = (uint32_t)Char4ToLittleInt(pdata + count);
		count += 4;

		if (m_res.moov.mvhd_version == 0)
		{
			m_res.moov.mvhd_duration = (uint32_t)Char4ToLittleInt(pdata + count);
			count += 4;
		}
		else
		{
			m_res.moov.mvhd_duration = Char8ToLittleInt(pdata + count);
			count += 8;
		}

		//rate
		count += 4;
		//volume
		count += 2;
		//reserved
		count += 2;
		count += 8;
		//matrix
		count += 36;
		//reserved
		count += 24;
		//nexttrackid 3
		count += 4;
	}
	else if (strncmp(m_res.boxHead.type, "trak", 4) == 0)
	{
		Mp4Track track;

		boxsize = m_res.boxHead.size;
		Parse_moov_track(pdata + count, len - count, track);
		count += boxsize - boxhsize;

		if (strncmp(track.hdlr_type, "vide", 4) == 0)
		{
			Cal_stbl(track);
			m_res.moov.track_video = std::move(track);
		}
		else if (strncmp(track.hdlr_type, "soun", 4) == 0)
		{
			Cal_stbl(track);
			m_res.moov.track_audio = std::move(track);
		}
	}
	else if (strncmp(m_res.boxHead.type, "mvex", 4) == 0)
	{
		boxsize = m_res.boxHead.size;
		Parse_moov_mvex(pdata + count, len - count);
		count += boxsize - boxhsize;
	}
	else
	{
		TRACE0("un handled box \n");
		count += (m_res.boxHead.size - boxhsize);
	}

	if (count < moovsize)
		goto ReadBox;
}

void  Mp4Parse::Parse_moov_mvex(const char* pdata, int len)
{
	uint32_t mvexsize = m_res.boxHead.size - m_res.boxHead.boxheadsize;
	uint32_t count(0), boxhsize, boxsize;

ReadSubBox:

	boxhsize = ParseBoxHead(pdata + count, len - count);
	boxsize = m_res.boxHead.size;
	count += boxhsize;

	if (strncmp(m_res.boxHead.type, "mehd", 4) == 0)
	{
		m_res.moov.mvex.mehd_version = *(pdata + count);
		count += 1;
		// flags 3
		count += 3;
		if (m_res.moov.mvex.mehd_version == 0)
		{
			m_res.moov.mvex.mehd_fragmentduration = (uint32_t)Char4ToLittleInt(pdata + count);
			count += 4;
		}
		else
		{
			m_res.moov.mvex.mehd_fragmentduration = Char8ToLittleInt(pdata + count);
			count += 8;
		}
	}
	else if (strncmp(m_res.boxHead.type, "trex", 4) == 0)
	{
		// version
		// flag
		count += 4;
		std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, Mp4SampleFlags> trex;

		std::get<0>(trex) = Char4ToLittleInt(pdata + count);
		count += 4;
		std::get<1>(trex) = Char4ToLittleInt(pdata + count);
		count += 4;
		std::get<2>(trex) = Char4ToLittleInt(pdata + count);
		count += 4;
		std::get<3>(trex) = Char4ToLittleInt(pdata + count);
		count += 4;
		Cal_SampleFlags(pdata + count, std::get<4>(trex));
		count += 4;

		m_res.moov.mvex.trexs.push_back(std::move(trex));
	}
	else
	{
		count += boxsize - boxhsize;
	}

	if (count < mvexsize)
		goto ReadSubBox;
}

void Mp4Parse::Parse_moov_track(const char* pdata, int len, Mp4Track& track)
{
	int count(0), boxhsize, boxsize;
	int tracksize = m_res.boxHead.size - m_res.boxHead.boxheadsize;

	do
	{
		boxhsize = ParseBoxHead(pdata + count, len-count);
		if (boxhsize < 8)
			return;
		count += boxhsize;

		if (strncmp(m_res.boxHead.type, "tkhd", 4) == 0)
		{
			track.tkhd_version = *(pdata + count);
			++count;
			track.thkd_flag[0] = 0;
			memcpy(&(track.thkd_flag[1]), pdata + count, 3);
			count += 3;

			if (track.tkhd_version == 0)
			{
				track.tkhd_createtime = (uint32_t)Char4ToLittleInt(pdata + count);
				count += 4;
				track.tkhd_moditime = (uint32_t)Char4ToLittleInt(pdata + count);
				count += 4;
			}
			else
			{
				track.tkhd_createtime = Char8ToLittleInt(pdata + count);
				count += 8;
				track.tkhd_moditime = Char8ToLittleInt(pdata + count);
				count += 8;
			}

			track.trackid = Char4ToLittleInt(pdata + count);
			count += 4;

			//reserved
			count += 4;

			if (track.tkhd_version == 0)
			{
				track.tkhd_duration = Char4ToLittleInt(pdata + count);
				count += 4;
			}
			else
			{
				track.tkhd_duration = Char8ToLittleInt(pdata + count);
				count += 8;
			}

			//reserved
			count += 8;
			//layer
			count += 2;
			//alternateGroup
			count += 2;
			//volumn
			count += 2;
			//reserved
			count += 2;
			//matrix
			count += 36;
			track.tkhd_width = (float)FixedPointNumber1616(pdata + count);
			count += 4;
			track.tkhd_height = (float)FixedPointNumber1616(pdata + count);
			count += 4;
		}
		else if (strncmp(m_res.boxHead.type, "mdia", 4) == 0)
		{
			boxsize = m_res.boxHead.size;
			Parse_moov_track_mdia(pdata + count, len - count, track);
			count += boxsize - boxhsize;
		}
		else
		{
			count += (m_res.boxHead.size - boxhsize);
		}
	} while (count < tracksize);
}

void Mp4Parse::Parse_moov_track_mdia(const char* pdata, int len, Mp4Track& track)
{
	int count(0), boxhsize, boxsize;
	int mdiasize = m_res.boxHead.size - m_res.boxHead.boxheadsize;
	unsigned char temp[4];

ReadSubBox:
	boxhsize = ParseBoxHead(pdata + count, len-count);
	if (boxhsize < 8)
	{
		return;
	}
	count += boxhsize;

	if (strncmp(m_res.boxHead.type, "mdhd", 4) == 0)
	{
		track.mdhd_version = *(pdata + count);
		++count;
		// flags, reserved
		count += 3;

		if (track.mdhd_version == 0)
		{
			track.mdhd_createtime = (uint32_t)Char4ToLittleInt(pdata + count);
			count += 4;
			track.mdhd_moditime = (uint32_t)Char4ToLittleInt(pdata + count);
			count += 4;

			track.mdhd_timescale = Char4ToLittleInt(pdata + count);
			count += 4;

			track.mdhd_duration = (uint32_t)Char4ToLittleInt(pdata + count);
			count += 4;
		}
		else
		{
			track.mdhd_createtime = Char8ToLittleInt(pdata + count);
			count += 8;
			track.mdhd_moditime = Char8ToLittleInt(pdata + count);
			count += 8;

			track.mdhd_timescale = Char4ToLittleInt(pdata + count);
			count += 4;

			track.mdhd_duration = Char8ToLittleInt(pdata + count);
			count += 8;
		}

		memcpy(temp, pdata + count, 2);
		count += 2;

		//0x15c7 eng
		track.mdhd_language[0] = ((temp[0] & 0x7c) >> 2) + 0x60;
		track.mdhd_language[1] = ((temp[0] & 0x3) << 3) + ((temp[1] & 0xe0) >> 5) + 0x60;
		track.mdhd_language[2] = (temp[1] & 0x1f) + 0x60;

		//reserved
		count += 2;
	}
	else if (strncmp(m_res.boxHead.type, "hdlr", 4) == 0)
	{
		boxsize = count + m_res.boxHead.size - boxhsize;
		//version
		count += 1;
		//flags
		count += 3;
		//predefined
		count += 4;

		memcpy(track.hdlr_type, pdata + count, 4);
		count += 4;

		//reserved
		count += 12;
		// null terminated utf8 string
		// 第一个字符可能是 unit separator(31)，其不可显示
		std::string name = pdata + count;
		count += name.length() + 1;

		count = boxsize;
	}
	else if (strncmp(m_res.boxHead.type, "minf", 4) == 0)
	{
		boxsize = m_res.boxHead.size;
		Parse_moov_track_mdia_minf(pdata + count, len-count, track);
		count += boxsize - boxhsize;
	}
	else
	{
		count += m_res.boxHead.size - boxhsize;
	}

	if (count < mdiasize)
		goto ReadSubBox;
}

void Mp4Parse::Parse_moov_track_mdia_minf(const char*pdata, int len, Mp4Track& track)
{
	int count(0), boxhsize, boxsize;
	int minfsize = m_res.boxHead.size - m_res.boxHead.boxheadsize;

ReadSubBox:
	boxhsize = ParseBoxHead(pdata + count, len-count);
	if (boxhsize < 8)
		return;
	count += boxhsize;

	if (strncmp(m_res.boxHead.type, "vmhd", 4) == 0)
	{

		// version 0
		count += 1;
		// flags 1
		count += 3;

		track.vmhd_graphicsMode = Char2ToLittleInt(pdata + count );
		count += 2;

		track.vmhd_opcolor[0] = Char2ToLittleInt(pdata + count);
		count += 2;
		track.vmhd_opcolor[1] = Char2ToLittleInt(pdata + count);
		count += 2;
		track.vmhd_opcolor[2] = Char2ToLittleInt(pdata + count);
		count += 2;
	}
	else if (strncmp(m_res.boxHead.type, "smhd", 4) == 0)
	{

		// version 0
		count += 1;
		// flags 0
		count += 3;

		track.smhd_balance = (float)FixedPointNumber88(pdata + count);
		count += 2;

		//reserved
		count += 2;
	}
	else if (strncmp(m_res.boxHead.type, "dinf", 4) == 0)
	{
		//char temp[100] = { 0 };
		//memcpy(temp, pdata + count, m_res.boxHead.size - 8);
		boxsize = m_res.boxHead.size;

		// data reference
		// 暂时无用
		int n, c(0);
		while(1)
		{
			n = ParseBoxHead(pdata + count + c, len - count - c);
			if (n < 8)
			{
				return;
			}

			c += n;

			if (strncmp(m_res.boxHead.type, "dref", 4) == 0)
			{
				// version 1
				// flags 3
				//int refindex = Char4ToLittleInt(pdata + count + c + 4);
			}

			c += m_res.boxHead.size - n;
			if (c >= boxsize - boxhsize)
				break;
		}
		count += boxsize - boxhsize;
	}
	else if (strncmp(m_res.boxHead.type, "stbl", 4) == 0)
	{
		boxsize = m_res.boxHead.size;
		Parse_moov_track_mdia_minf_stbl(pdata+count, len - count, track);
		count += boxsize - boxhsize;
	}
	else
	{
		count += m_res.boxHead.size - boxhsize;
	}

	if (count < minfsize)
		goto ReadSubBox;
}

void Mp4Parse::Parse_moov_track_mdia_minf_stbl(const char*pdata, int len, Mp4Track& track)
{
	int stblsize = m_res.boxHead.size - m_res.boxHead.boxheadsize;
	int boxsize, boxhsize, count(0), i, n, c;

ReadSubBox:
	boxhsize = ParseBoxHead(pdata + count, len - count);
	if (boxhsize < 8)
	{
		return;
	}

	count += boxhsize;
	boxsize = m_res.boxHead.size;

	if (strncmp(m_res.boxHead.type, "stsd", 4) == 0)
	{
		//version
		count += 1;
		//flags
		count += 3;
		// number of entry
		n = Char4ToLittleInt(pdata + count);
		count += 4;

		for (i = 0; i < n; ++i)
		{
			c = 0;
			if (strncmp(track.hdlr_type, "vide", 4) == 0)
			{
				c += ParseBoxHead(pdata + count + c, len - count - c);
				memcpy(track.stsd_v_codectype, m_res.boxHead.type, 4);
				boxsize = m_res.boxHead.size;

				// reserved
				c += 6;
				// datareferenceindex 1
				c += 2;
				// predefined
				c += 2;
				// reserved
				c += 2;
				// predefined
				c += 12;
				track.stsd_v_width = Char2ToLittleInt(pdata + count + c);
				c += 2;
				track.stsd_v_height = Char2ToLittleInt(pdata + count + c);
				c += 2;
				track.stsd_v_resolution_h = (float)FixedPointNumber1616(pdata + count + c);
				c += 4;
				track.stsd_v_resolution_w = (float)FixedPointNumber1616(pdata + count + c);
				c += 4;
				//reserved
				c += 4;
				track.stsd_v_framecount = Char2ToLittleInt(pdata + count + c);
				c += 2;
				memset(track.stsd_v_compressorname, 0, 32);
				memcpy(track.stsd_v_compressorname, pdata + count + c, 32);
				memmove(track.stsd_v_compressorname, track.stsd_v_compressorname + 1, track.stsd_v_compressorname[0]);
				track.stsd_v_compressorname[strlen(track.stsd_v_compressorname) > 0 ? (strlen(track.stsd_v_compressorname) - 1) : 0] = 0;
				c += 32;
				track.stsd_v_depth = Char2ToLittleInt(pdata + count + c);
				c += 2;
				//predefined
				c += 2;

				do
				{
					ParseBoxHead(pdata + count + c, len - count - c);
					if (strncmp(m_res.boxHead.type, "avcC", 4) == 0)
					{
						ParseAVCDecoderConfigurationRecord(track.stsd_v_avc_head, pdata + count + c + m_res.boxHead.boxheadsize, 0);
					}
					c += m_res.boxHead.size;
				} while (c < boxsize);
			}
			else if (strncmp(track.hdlr_type, "soun", 4) == 0)
			{
				c += ParseBoxHead(pdata + count + c, len - count - c);
				memcpy(track.stsd_a_codectype, m_res.boxHead.type, 4);
				boxsize = m_res.boxHead.size;

				// reserved
				c += 6;
				// datareferenceindex 1
				c += 2;
				// reserved
				c += 8;
				track.stsd_a_channelcount = Char2ToLittleInt(pdata + count + c);
				c += 2;
				track.stsd_a_samplesize = Char2ToLittleInt(pdata + count + c);
				c += 2;
				// predefined
				c += 2;
				// reserved
				c += 2;
				track.stsd_a_samplerate = (float)FixedPointNumber1616(pdata + count + c);
				c += 4;

				do
				{
					ParseBoxHead(pdata + count + c, len - count - c);
					// ESDs分为三层，MP4ESDescr(0x03开始),MP4DecConfigDescr(0x04开始),MP3DecSpecificDescr(0x05)
					track.stsd_a_esds_objectTypeID = 0;
					if (strncmp(m_res.boxHead.type, "esds", 4) == 0)
					{
						//char a[200] = { 0 };
						//memcpy(a, pdata + count + c + m_res.boxHead.boxheadsize, m_res.boxHead.size - m_res.boxHead.boxheadsize);
						
						int cc(c + m_res.boxHead.boxheadsize), len;
						
						// version 1
						cc += 1;
						// flags 3
						cc += 3;
						if (*(pdata + count + cc) == 0x03)
						{
							// tag
							++cc;
							// len 不确定长度占几个字节 (@*@)
							len = *(pdata + count + cc);
							++cc;
							if (len < 0)
								cc += 3;

							// ES_ID (0)
							cc += 2;
							// shoule 0
							cc += 1;

							if (*(pdata + count + cc) == 0x04)
							{
								// tag
								cc += 1;
								// len
								len = *(pdata + count + cc);
								cc += 1;
								if (len <0)
									cc += 3;

								track.stsd_a_esds_objectTypeID = *(pdata + count + cc);
								cc += 1;
								// 0x15
								cc += 1;
								cc += 11;
								if (*(pdata + count + cc) == 0x05)
								{
									// tag
									cc += 1;
									// len
									len = *(pdata + count + cc);
									if (len <0)
										cc += 3;
									cc += 1;
									ParseAudioSpecificConfig(track.stsd_a_aac_head, 
										(const unsigned char*)pdata + count + cc, 0);
								}
							}
						}
					}
					c += m_res.boxHead.size;
				} while (c < boxsize);
			}

			count += c;
		}
	}
	else if (strncmp(m_res.boxHead.type, "stts", 4) == 0)
	{
		//version 
		count += 1;
		//flags
		count += 3;
		n = Char4ToLittleInt(pdata + count);
		count += 4;

		// samplecount 个sample的 delta
		for (i = 0; i < n; ++i)
		{
			track.stts_samplecount.push_back(Char4ToLittleInt(pdata + count));
			count += 4;
			track.stts_sampledelta.push_back(Char4ToLittleInt(pdata + count));
			count += 4;
		}
	}
	else if (strncmp(m_res.boxHead.type, "stsc", 4) == 0)
	{
		// version 
		count += 1;
		// flags
		count += 3;
		n = Char4ToLittleInt(pdata + count);
		count += 4;

		// 从第 firstchunk 个chunk开始，每个chunk有 sampleperchunk 个sample
		// sampledescindex 1
		for (i = 0; i < n; ++i)
		{
			track.stsc_firstchunk.push_back(Char4ToLittleInt(pdata + count));
			count += 4;
			track.stsc_sampleperchunk.push_back(Char4ToLittleInt(pdata + count));
			count += 4;
			track.stsc_sampledescindex.push_back(Char4ToLittleInt(pdata + count));
			count += 4;
		}
	
	}
	else if (strncmp(m_res.boxHead.type, "stsz", 4) == 0)
	{
		// version 
		count += 1;
		// flags
		count += 3;
		// fixed sample size(usually 0, all samples have same size is impossible)
		track.stsz_constantsize = Char4ToLittleInt(pdata + count);
		count += 4;
		n = Char4ToLittleInt(pdata + count);
		count += 4;

		if (track.stsz_constantsize == 0)
		{
			for (i = 0; i < n; ++i)
			{
				// sample size
				track.stsz_sizetable.push_back(Char4ToLittleInt(pdata + count));
				count += 4;
			}
		}
	}
	else if (strncmp(m_res.boxHead.type, "stco", 4) == 0 || 
		strncmp(m_res.boxHead.type, "co64", 4) == 0)
	{
		// version 
		count += 1;
		// flags
		count += 3;
		track.stco_offsetcount = Char4ToLittleInt(pdata + count);
		count += 4;

		// chunk offset (from file begin)
		if (m_res.boxHead.type[0] == 's')
		{
			for (i = 0; i < track.stco_offsetcount; ++i)
			{
				track.stco_offsets.push_back(Char4ToLittleInt(pdata + count));
				count += 4;
			}
		}
		else
		{
			for (i = 0; i < track.stco_offsetcount; ++i)
			{
				track.stco_offsets.push_back((uint32_t)Char8ToLittleInt(pdata + count));
				count += 8;
			}
		}

	}
	else if (strncmp(m_res.boxHead.type, "ctts", 4) == 0)
	{
		// version 
		count += 1;
		// flags
		count += 3;
		n = Char4ToLittleInt(pdata + count);
		count += 4;

		for (i = 0; i < n; ++i)
		{
			track.ctts_samplecount.push_back(Char4ToLittleInt(pdata + count));
			count += 4;
			track.ctts_sampleoffset.push_back(Char4ToLittleInt(pdata + count));
			count += 4;
		}
	}
	else if (strncmp(m_res.boxHead.type, "stss", 4) == 0)
	{
		// version 
		count += 1;
		// flags
		count += 3;
		n = Char4ToLittleInt(pdata + count);
		count += 4;

		for (i = 0; i < n; ++i)
		{
			// syns sample table
			track.stss_synctable.push_back(Char4ToLittleInt(pdata + count));
			count += 4;
		}
	}
	//else if (strncmp(m_res.boxHead.type, "sdtp", 4) == 0)
	//{}
	else
	{
		count += boxsize - boxhsize;
	}

	if (count < stblsize)
		goto ReadSubBox;
}

void Mp4Parse::Cal_stbl(Mp4Track& track)
{
	uint32_t samplecount(00), n(0);
	std::size_t i, j;
	// vector< pair<firstsampleofthischunk, lastsampleofthischunk> >
	std::vector<std::pair<uint32_t, uint32_t> > chunkSamples;

	for (i = 0; i < track.stts_samplecount.size(); ++i)
	{
		samplecount += track.stts_samplecount[i];
	}

	chunkSamples.resize(track.stco_offsetcount);
	for (i = 0; i < track.stco_offsetcount; ++i)
	{
		chunkSamples[i].first = n+1;
		for (int j = track.stsc_firstchunk.size() - 1; j >= 0; --j)
		{
			if ((i + 1) >= track.stsc_firstchunk[j])
			{
				n += track.stsc_sampleperchunk[j];
				break;
			}
		}
		chunkSamples[i].second = n;
	}

	track.stsz_offsettable.resize(samplecount);
	for (i = 0; i < chunkSamples.size(); ++i)
	{
		n = track.stco_offsets[i];
		j = chunkSamples[i].first;
		do
		{
			track.stsz_offsettable[j-1] = n;
			n += track.stsz_sizetable[j - 1];
			++j;
		} while (j <= chunkSamples[i].second);
	}
}

void Mp4Parse::Cal_SampleFlags(const char* pdata, Mp4SampleFlags& msf)
{
	unsigned char ch1 = static_cast<unsigned char>(*pdata);
	unsigned char ch2 = static_cast<unsigned char>(*(pdata + 1));

	msf.sampleDependsOn = ch1 & 0x03;
	msf.samplesDependedOn = (ch2 & 0xc0) >> 6;
	msf.sampleHasRedundancy = (ch2 & 0x30) >> 4;
	msf.samplesDifferenceSample = ch2 & 0x01;
}

int Mp4Muxer::Create_BoxHead(char* outbuf, int size, const char* type)
{
	WriteInt32(size, outbuf);
	memcpy(outbuf + 4, type, 4);
	return 8;
}

int Mp4Muxer::Create_ftyp(char * outbuf, int buflen)
{
	if (!outbuf)
		return 24;
	if (buflen < 24)
		return 24;

	// boxhead
	int n = Create_BoxHead(outbuf, 24, "ftyp");
	// major band
	memcpy(outbuf + n, "iso5", 4);
	n += 4;
	// min band
	//memset(outbuf + n, 0, 4);
	WriteInt32(1, outbuf + n);
	n += 4;
	memcpy(outbuf + n, "iso5dash", 8);
	n += 8;

	return n;//24 fixed
}

int Mp4Muxer::Create_moov(char* outbuf, int buflen)
{
	int moovsize = 8 + Create_moov_mvhd(0, 0) +
		Create_moov_mvex(0, 0) +
		Create_moov_track(0, 0, muxerParams.moov.track_video) +
		Create_moov_track(0, 0, muxerParams.moov.track_audio);

	if (!outbuf || buflen < moovsize)
	{
		return moovsize;
	}

	int n(0);
	n += Create_BoxHead(outbuf, moovsize, "moov");
	n += Create_moov_mvhd(outbuf + n, buflen - n);
	n += Create_moov_mvex(outbuf + n, buflen - n);
	n += Create_moov_track(outbuf + n, buflen - n, muxerParams.moov.track_video);
	n += Create_moov_track(outbuf + n, buflen - n, muxerParams.moov.track_audio);

	if (moovsize != n)
		throw 1;

	return n;
}

int Mp4Muxer::Create_moov_mvhd(char* p, int len)
{
	if (!p || len <108)
		return 108;

	int n(0);
	n += Create_BoxHead(p, 108, "mvhd");
	memset(p + n, 0, 108 - n);

	// version 1
	n += 1;
	// flags
	n += 3;
	// createtime
	WriteInt32((int32_t)3552395809, p + n);
	n += 4;
	// modificationtime
	WriteInt32((int32_t)3552395810, p + n);
	n += 4;
	// timescale
	WriteInt32(muxerParams.moov.mvhd_timescale, p + n);
	n += 4;
	// duration
	WriteInt32(static_cast<int32_t>( muxerParams.moov.mvhd_duration), p + n);
	n += 4;
	// rate 4
	WriteInt32(0x00010000, p + n);
	n += 4;
	// volume
	WriteInt16(0x0100, p + n);
	n += 2;
	// reserved 2
	n += 2;
	// reserved 4*2
	n += 8;
	// matrix 4*9
	{
		int32_t m[9] = { 0x00010000, 0,0,
		0,0x00010000,0,
		0,0,0x40000000 };
		for (int i = 0; i < 9; ++i)
		{
			WriteInt32(m[i], p + n);
			n += 4;
		}
	}
	// reserved 4*6
	n += 24;
	// nexttrackid 4
	WriteInt32(3, p + n);
	n += 4;

	if (n != 108)
		throw 1;

	return n;
}

int Mp4Muxer::Create_moov_mvex(char* p, int len)
{
	if (!p || len < 88)
	{
		// head +  mehd + trex*2
		//return 8 + (8 + 8) + (8 + 24) * 2;
		return 88;
	}

	int n(0);
	n += Create_BoxHead(p, 88, "mvex");

	// mehd
	n += Create_BoxHead(p + n, 16, "mehd");
	// verison 1 flags 3
	WriteInt32(0, p + n);
	n += 4;
	WriteInt32(static_cast<uint32_t>(muxerParams.moov.mvex.mehd_fragmentduration), p + n);
	n += 4;

	// trex video
	n += Create_BoxHead(p + n, 32, "trex");
	// verison 1 flags 3
	WriteInt32(0, p + n);
	n += 4;
	// trackid
	WriteInt32(muxerParams.moov.track_video.trackid, p + n);
	n += 4;
	// default sample descrtption index
	WriteInt32(1, p + n);
	n += 4;
	// default sample duration
	WriteInt32(1000, p + n);
	n += 4;
	// default sample size
	WriteInt32(0, p + n);
	n += 4;
	// default sample flags
	WriteInt32(0x00010000, p + n);
	n += 4;
	 //trex audio
	n += Create_BoxHead(p + n, 32, "trex");
	WriteInt32(0, p + n);
	n += 4;
	WriteInt32(muxerParams.moov.track_audio.trackid, p + n);
	n += 4;
	WriteInt32(1, p + n);
	n += 4;
	WriteInt32(1024, p + n);
	n += 4;
	WriteInt32(0, p + n);
	n += 4;
	WriteInt32(0x02000000, p + n);
	n += 4;

	if (n != 88)
		throw 1;

	return n;
}

int Mp4Muxer::Create_moov_track(char*p, int len, const Mp4Track& track)
{
	// headsize + tkhd size + mdia 
	int tracksize = 8 + 92 +Create_moov_track_mdia(NULL, 0, track);
	if (!p)
		return tracksize;
	if (len < tracksize)
		return tracksize;

	int n(0);
	n += Create_BoxHead(p, tracksize, "trak");
	{
		n += Create_BoxHead(p + n, 92, "tkhd");

		// version 1 flags 3
		WriteInt32(0, p + n);
		n += 4;
		// createtime
		WriteInt32((int32_t)3552395809, p + n);
		n += 4;
		// modificationtime
		WriteInt32((int32_t)3552395810, p + n);
		n += 4;
		// trackid
		WriteInt32(track.trackid, p + n);
		n += 4;
		// reserved
		WriteInt32(0, p + n);
		n += 4;
		// duration
		WriteInt32(static_cast<int32_t>(track.tkhd_duration), p + n);
		n += 4;
		// reserved
		memset(p + n, 0, 8);
		n += 8;
		// layer & alternategroup
		WriteInt32(0, p + n);
		n += 4;
		// volume
		WriteInt16(0x0100, p + n);
		n += 2;
		// reserved
		WriteInt16(0, p + n);
		n += 2;
		// matrix 4*9
		{
			int32_t m[9] = { 0x00010000, 0,0,
				0,0x00010000,0,
				0,0,0x40000000 };
			for (int i = 0; i < 9; ++i)
			{
				WriteInt32(m[i], p + n);
				n += 4;
			}
		}
		// width	
		ToFixedPointNumber1616(static_cast<int32_t>(track.tkhd_width), p + n);
		n += 4;
		// height
		ToFixedPointNumber1616(static_cast<int32_t>(track.tkhd_height), p + n);
		n += 4;
	}

	n += Create_moov_track_mdia(p + n, len - n, track);

	if (n != tracksize)
		throw 1;

	return n;
}

int Mp4Muxer::Create_moov_track_mdia(char*p, int len, const Mp4Track& track)
{
	// head + mdhd + hdlr + minf
	int mdiasize = 8 + 32 + (track.trackid == 1 ? 64 : 68) + Create_moov_track_mdia_minf(NULL, 0, track);
	if (!p || len < mdiasize)
		return mdiasize;

	int n(0);
	n += Create_BoxHead(p, mdiasize, "mdia");
	n += Create_BoxHead(p + n, 32, "mdhd");
	// version 1 flags 3
	WriteInt32(0, p + n);
	n += 4;
	// createtime
	WriteInt32((int32_t)3552395809, p + n);
	n += 4;
	// modificationtime
	WriteInt32((int32_t)3552395810, p + n);
	n += 4;
	// timescale
	WriteInt32(track.mdhd_timescale, p + n);
	n += 4;
	// duration
	WriteInt32((int32_t)track.mdhd_duration, p + n);
	n += 4;
	// eng
	WriteInt16(0x15c7, p + n);
	n += 2;
	// reserved
	WriteInt16(0, p + n);
	n += 2;

	n += (track.trackid == 1 ? Create_BoxHead(p + n, 64, "hdlr") : Create_BoxHead(p + n, 68, "hdlr"));
	// version flags
	WriteInt32(0, p + n);
	n += 4;
	// predefined
	WriteInt32(0, p + n);
	n += 4;
	// handlertype
	memcpy(p + n, track.hdlr_type, 4);
	n += 4;
	// reserved
	memset(p + n, 0, 12);
	n += 12;
	if (track.trackid == 1)
	{
		memset(p + n, 0, 32);
		memcpy(p + n, "Mainconcept Video Media Handler", 31);
		n += 32;
	}
	else
	{
		memset(p + n, 0, 36);
		memcpy(p + n, "Mainconcept MP4 Sound Media Handler", 35);
		n += 36;
	}

	n += Create_moov_track_mdia_minf(p + n, len - n, track);
	if (n != mdiasize)
		throw 1;

	return mdiasize;
}

int Mp4Muxer::Create_moov_track_mdia_minf(char* p, int len, const Mp4Track& track)
{
	int minfsize(00);

	if (track.trackid == 1)
	{
		// boxhead + vmhd + dinf + stbl
		minfsize = 8 + 20 + 36 + Create_moov_track_mdia_minf_stbl(0, 0, track);
	}
	else
	{
		// boxhead + smhd + dinf + stbl
		minfsize = 8 + 16 + 36 + Create_moov_track_mdia_minf_stbl(0, 0, track);
	}

	if (!p || len < minfsize)
	{
		return minfsize;
	}

	int n(0);

	n += Create_BoxHead(p, minfsize, "minf");
	if (track.trackid == 1)
	{
		n += Create_BoxHead(p + n, 20, "vmhd");
		char vmhddata[] = {
			0, 0, 0, 1,
			0, 0,
			0, 0, 0, 0, 0, 0
		};
		memcpy(p + n, vmhddata, 12);
		n += 12;
	}
	else
	{
		n += Create_BoxHead(p + n, 16, "smhd");
		char smhddata[] = {
			0,0,0,0,
			0,0,
			0,0,
		};
		memcpy(p + n, smhddata, 8);
		n += 8;
	}
	n += Create_BoxHead(p + n, 36, "dinf");
	char dinfdata[] = {
		0x00, 0x00, 0x00, 0x1c,
		0x64, 0x72, 0x65, 0x66,
		0, 0, 0, 0,
		0, 0, 0, 0x01,
		0, 0, 0, 0x0c,
		0x75, 0x72, 0x6c, 0x20, 
		0,0,0,1
	};
	memcpy(p + n, dinfdata, 28);
	n += 28;
	n += Create_moov_track_mdia_minf_stbl(p + n, len - n, track);
	if (n != minfsize)
		throw 1;

	return minfsize;
}

int Mp4Muxer::Create_moov_track_mdia_minf_stbl(char* p, int len, const Mp4Track& track)
{
	int stblsize(0), stsdsize(0);
	int n(0);

	if (track.trackid == 1)
	{
		stsdsize = 110 + avcHeadLen;
	}
	else
	{
		stsdsize = 91;
	}
	// stbl head + stsdsize + other
	stblsize = 8 + stsdsize + 68;

	if (!p || len < stblsize)
	{
		return stblsize;
	}

	n += Create_BoxHead(p, stblsize, "stbl");

	// stsd
	n += Create_BoxHead(p + n, stsdsize, "stsd");
	WriteInt32(0, p + n);
	n += 4;
	WriteInt32(1, p + n);
	n += 4;
	if (track.trackid == 1)
	{
		int avc1size = 94 + avcHeadLen;
		n += Create_BoxHead(p + n, avc1size, "avc1");
		memset(p + n, 0, 6);
		n += 6;
		WriteInt16(1, p + n);
		n += 2;
		memset(p + n, 0, 18);
		n += 16;
		WriteInt16(track.stsd_v_width, p + n);
		n += 2;
		WriteInt16(track.stsd_v_height, p + n);
		n += 2;
		ToFixedPointNumber1616(72.0, p + n);
		n += 4;
		ToFixedPointNumber1616(72.0, p + n);
		n += 4;
		// reserved
		WriteInt32(0, p + n);
		n += 4;
		// framecount
		WriteInt16(1, p + n);
		n += 2;
		// compressorname
		memset(p + n, 0, 32);
		*(p + n) = 10;
		memcpy(p + n + 1, "AVC Coding", 10);
		n += 32;
		// depth
		WriteInt16(24, p + n);
		n += 2;
		WriteInt16(-1, p + n);
		n += 2;

		int avccsize = 8 + avcHeadLen;
		n += Create_BoxHead(p + n, avccsize, "avcC");
		memcpy(p + n, avcHeadData, avcHeadLen);
		n += avcHeadLen;
	}
	else
	{
		int esdssize(39);
		int mp4asize = 36 + esdssize;

		n += Create_BoxHead(p + n, mp4asize, "mp4a");
		memset(p + n, 0, 6);
		n += 6;
		WriteInt16(1, p + n);
		n += 2;
		memset(p + n, 0, 8);
		n += 8;
		WriteInt16(muxerParams.moov.track_audio.stsd_a_channelcount, p + n);
		n += 2;
		WriteInt16(muxerParams.moov.track_audio.stsd_a_samplesize, p + n);
		n += 2;
		WriteInt32(0, p + n);
		n += 4;
		ToFixedPointNumber1616(muxerParams.moov.track_audio.stsd_a_samplerate, p + n);
		n += 4;

		n += Create_BoxHead(p + n, esdssize, "esds");
		WriteInt32(0, p + n);
		n += 4;
		*(p + n) = 0x03;
		++n;
		*(p + n) = 25;
		++n;
		WriteInt16(0, p + n);
		n += 2;
		*(p + n) = 0;
		++n;
		*(p + n) = 0x04;
		++n;
		*(p + n) = 17;
		++n;
		*(p + n) = 64;
		++n;
		*(p + n) = 0x15;
		++n;
		memset(p + n, 0, 11);
		n += 11;
		*(p + n) = 0x05;
		++n;
		*(p + n) = 2;
		++n;
		memcpy(p + n, this->aacHeadData, 2);
		n += 2;
		*(p + n) = 0x06;
		*(p + n + 1) = 0x01;
		*(p + n + 2) = 0x02;
		n += 3;
	}
	
	// stts
	n += Create_BoxHead(p + n, 16, "stts");
	memset(p + n, 0, 8);
	n += 8;
	// stsc
	n += Create_BoxHead(p + n, 16, "stsc");
	memset(p + n, 0, 8);
	n += 8;
	// stsz
	n += Create_BoxHead(p + n, 20, "stsz");
	memset(p + n, 0, 12);
	n += 12;
	// stco
	n += Create_BoxHead(p + n, 16, "stco");
	memset(p + n, 0, 8);
	n += 8;

	if (n != stblsize)
	{
		throw 1;
	}

	return stblsize;
}

int Mp4Muxer::Create_moof(char* pout, int len, int segno)
{
	int moofsize(0);

	// head mfhd traf(v) traf(a)
	moofsize = 8 + 16 + 
		Create_moof_traf(0, 0, muxerParams.moof.traf_video) + 
		Create_moof_traf(0, 0, muxerParams.moof.traf_audio);

	if (!pout || len < moofsize)
	{
		return moofsize;
	}

	int n(0);
	memset(pout, 0, moofsize);
	n += Create_BoxHead(pout, moofsize, "moof");
	n += Create_BoxHead(pout + n, 16, "mfhd");
	n += 4;
	WriteInt32(muxerParams.moof.mfhd_seqno, pout + n);
	n += 4;
	n += Create_moof_traf(pout + n, len - n, muxerParams.moof.traf_video);
	n += Create_moof_traf(pout + n, len - n, muxerParams.moof.traf_audio);

	if (n != moofsize)
		throw 1;

	return moofsize;
}

int Mp4Muxer::Create_moof_traf(char* pout, int len, Mp4TrackFragment& traf)
{
	int trafsize(0), trunsize(0);
/*
	// head 8 version 1 flags 3 samplecount 4
	trunsize = 16;
	if (traf.trun_flags & 0x01)
	{ 
		// dataoffset
		trunsize += 4;
	}
	if (traf.trun_flags & 0x04)
	{
		// firstsampleflags
		trunsize += 4;
	}
	if (traf.trun_flags & 0x0100)
	{
		// sampleduration
		trunsize += 4 * traf.trun_samplecount;
	}
	if (traf.trun_flags & 0x0200)
	{
		// samplesize
		trunsize += 4 * traf.trun_samplecount;
	}
	if (traf.trun_flags & 0x0400)
	{
		// sampleflags
		trunsize += 4 * traf.trun_samplecount;
	}
	if (traf.trun_flags & 0x0800)
	{
		// samplecompositiontimeoffset
		trunsize += 4 * traf.trun_samplecount;
	}
*/
	// 简化一下 
	// 固定video traf的trun flag为0x0e01 dataoffset+(samplesize+sampleflag+samplects)*samplecount
	// 固定audio traf的trun flag为0x0201 dataoffset+(samplesize)*samplecount
	if (traf.tfhd_trackid == 1)
	{
		trunsize = 20 + traf.trun_samplecount * 4 * 3;
	}
	else
	{
		trunsize = 20 + traf.trun_samplecount * 4 * 1;
	}

	// head tfhd(no optional) tfdt trun
	trafsize = 8 + 16 + 16 + trunsize;
	if (!pout || len < trafsize)
		return trafsize;

	int count(0);

	count += Create_BoxHead(pout, trafsize, "traf");
	count += Create_BoxHead(pout + count, 16, "tfhd");
	WriteInt32(traf.tfhd_flags, pout + count);
	count += 4;
	WriteInt32(traf.tfhd_trackid, pout + count);
	count += 4;
	count += Create_BoxHead(pout + count, 16, "tfdt");
	count += 4;
	WriteInt32((uint32_t)traf.tfdt_basemediadecodetime, pout + count);
	count += 4;
	count += Create_BoxHead(pout + count, trunsize, "trun");
	WriteInt32(traf.trun_flags, pout + count);
	count += 4;
	WriteInt32(traf.trun_samplecount, pout + count);
	count += 4;
	WriteInt32(traf.trun_dataoffset, pout + count);
	count += 4;

	if (traf.tfhd_trackid == 1)
	{
		for (uint32_t i = 0; i < traf.trun_samplecount; ++i)
		{
			WriteInt32(traf.trun_samplesize[i], pout + count);
			count += 4;
			WriteInt32(traf.trun_sampleflags1[i], pout + count);
			count += 4;
			WriteInt32(traf.trun_samplecompositiontimeoffset[i], pout + count);
			count += 4;
		}
	}
	else
	{
		for (uint32_t i = 0; i < traf.trun_samplecount; ++i)
		{
			WriteInt32(traf.trun_samplesize[i], pout + count);
			count += 4;
		}
	}

	if (count != trafsize)
		throw 1;
	return trafsize;
}