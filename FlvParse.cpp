#include "FlvParse.h"
#include <string>
#include <cmath>

int get_ue(BitsGet& bg)
{
	int zerobits_num(0);
	int b;

	b = bg.GetBit();
	while (b == 0)
	{
		++zerobits_num;
		b = bg.GetBit();
	}

	return (1 << zerobits_num) + bg.GetBits(zerobits_num) - 1;
}

void WriteInt32(int32_t i, char *a)
{
	char*temp = (char*)&i;
	a[0] = temp[3];
	a[1] = temp[2];
	a[2] = temp[1];
	a[3] = temp[0];
}

void WriteInt16(int16_t i, char *a)
{
	char*temp = (char*)&i;
	a[0] = temp[1];
	a[1] = temp[0];
}

void WriteInt64(int64_t i, char *a)
{
	char*temp = (char*)&i;

	for (int j = 0; j < 8; ++j)
	{
		a[j] = temp[7 - j];
	}
}

int64_t Char8ToLittleInt(const char* p)
{
	uint64_t high32 = (uint32_t)Char4ToLittleInt(p);
	uint32_t low32 = (uint32_t)Char4ToLittleInt(p + 4);
	return (high32 << 32) + low32;
}

int32_t Char4ToLittleInt(const char*temp)
{
	const unsigned char* p = (const unsigned char*)temp;
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

int32_t Char2ToLittleInt(const char*temp)
{
	const unsigned char* p = (const unsigned char*)temp;
	return (p[0] << 8) | p[1];
}

double Char8ToDouble(const char* temp)
{
	char p[8], ch;
	memcpy(p, temp, 8);

	for (int i = 0; i < 4; ++i)
	{
		ch = p[i];
		p[i] = p[7 - i];
		p[7 - i] = ch;
	}
	double d = *((double*)p);
	return d;
}

double FixedPointNumber88(const char* temp)
{
	double d(0);

	int32_t i = (int32_t)*temp;
	uint8_t f = *(uint8_t*)(temp + 1);

	uint8_t m;
	for (int n = 0; n < 8; ++n)
	{
		m = 1;
		m = m << n;

		if (f & m)
		{
			d = d + 1.0 / pow(2, n + 1);
		}
	}

	return i + (i > 0 ? d : -d);
}

double FixedPointNumber1616(const char* temp)
{
	double d(0);

	int32_t i = Char2ToLittleInt(temp);
	uint16_t f = *(uint16_t*)(temp + 2);

	uint16_t m;
	for (int n = 0; n < 16; ++n)
	{
		m = 1;
		m = m << n;

		if (f & m)
		{
			d = d + 1.0 / pow(2, n + 1);
		}
	}
	
	return i + (i > 0 ? d : -d);
}

void ToFixedPointNumber1616(double d, char a[])
{
	int i = static_cast<int>(d);

	memset(a, 0, 4);

	BitsPut bp(16);
	bp.PutBits(16, i);

	memcpy(a, bp.GetBitsData(), 2);

}

void ParseAVCDecoderConfigurationRecord(AVCDecoderConfigurationRecord& avc_head, const char* ptemp, int)
{
	int count(0);
	avc_head.avc_seq_head_version = *(ptemp + (count++));
	avc_head.avc_seq_head_profile = *(ptemp + (count++));
	avc_head.avc_seq_head_profilecompatibility = *(ptemp + (count++));
	avc_head.avc_seq_head_level = *(ptemp + (count++));
	avc_head.avc_seq_head_NALUnitLength = *(ptemp + (count++)) & 3;
	avc_head.avc_seq_head_NALUnitLength += 1;
	avc_head.avc_seq_head_spsnum = *(ptemp + (count++)) & 0x1f; // 一般1， 不是1的话，就完蛋了

	int spssize, ppssize/*, i*/;
	char buf[] = { 0x0, 0x0, 0x0, 0x1 }; //h264 startcode

	memcpy(avc_head.avc_seq_head_data, buf, 4);
	//for (i = 0; i < res.videoTag.avc_seq_head_spsnum; ++i)
	//{
	spssize = Char2ToLittleInt(ptemp + count);
	count += 2;
	memcpy(avc_head.avc_seq_head_data + 4, ptemp + count, spssize);
	count += spssize;
	//}

	avc_head.avc_seq_head_ppsnum = *(ptemp + count);         // 一般1
	memcpy(avc_head.avc_seq_head_data + 4 + spssize, buf, 4);
	count += 1;
	//for (i = 0; i < res.videoTag.avc_seq_head_ppsnum; ++i)
	//{
	ppssize = Char2ToLittleInt(ptemp + count);
	count += 2;
	memcpy(avc_head.avc_seq_head_data + 4 + spssize + 4, ptemp + count, ppssize);
	//}

	avc_head.avc_seq_head_datalen = 4 + spssize + 4 + ppssize;
}

void ParseAudioSpecificConfig(AudioSpecificConfig& aac_head, const unsigned char*a, int)
{
	unsigned char temp1, temp2, temp3;
	aac_head.aac_seq_head_audioObjectType = (a[0] & 0xf8) >> 3;
	aac_head.aac_seq_head_audioObjectType -= 1;
	aac_head.aac_seq_head_samplingFrequencyIndex = ((a[0] & 0x07) << 1) + ((a[1] & 0x80) >> 7);
	if (aac_head.aac_seq_head_samplingFrequencyIndex == 0xf)
	{
		temp1 = ((a[1] & 0x7f) << 1) | ((a[2] & 0x80) >> 7);
		temp2 = ((a[2] & 0x7f) << 1) | ((a[3] & 0x80) >> 7);
		temp3 = ((a[3] & 0x7f) << 1) | ((a[4] & 0x80) >> 7);
		aac_head.aac_seq_head_samplingFrequency = (temp1 << 16) | (temp2 << 8) | temp3;
		aac_head.aac_seq_head_channelConfiguration = (a[4] & 0x78) >> 3;

		//throw 1;
	}
	else
	{
		aac_head.aac_seq_head_channelConfiguration = (a[1] & 0x78) >> 3;
		aac_head.aac_seq_head_samplingFrequency = ParseSampleRateByIndex(aac_head.aac_seq_head_samplingFrequencyIndex);
	}
}

int ParseSampleRateByIndex(int iii)
{
	int res;
	switch (iii)
	{
	case 0:
		res = 96000;
		break;
	case 1:
		res = 88200;
		break;
	case 2:
		res = 64000;
		break;
	case 3:
		res = 48000;
		break;
	case 4:
		res = 44100;
		break;
	case 5:
		res = 32000;
		break;
	case 6:
		res = 24000;
		break;
	case 7:
		res = 22025;
		break;
	case 0x8:
		res = 16000;
		break;
	case 0x9:
		res = 12000;
		break;
	case 0xa:
		res = 11025;
		break;
	case 0xb:
		res = 8000;
		break;
	case 0xc:
		res = 7350;
		break;
	default:
		std::logic_error("unsupport samplerate");
		break;
	}

	return res;
}

void ParseSPS(H264SPSInfo& sps, const unsigned char* p, int spssize)
{
	//NALU : Coded H.264 data is stored or transmitted as a series of packets known as NetworkAbstraction LayerUnits.
	//RBSP : A NALU contains a Raw  Byte Sequence  Payload, a sequence of bytes containingsyntax elements
	//SODB : String Of Data Bits

	// sps和pps各是一个nalu

	/*
		H.264 防竞争
		0X000000 - 0X00000300
		0X000001 - 0X00000301
		0X000002 - 0X00000302
		0X000003 - 0X00000303
	*/

	sps.forbidden_zero_bit = (p[0] & 0x80) >> 7;
	sps.nal_ref_idc = (p[0] & 0x60) >> 5;
	sps.nal_unit_type = p[0] & 0x1f;

	if (sps.nal_unit_type != 7)
	{
		throw std::logic_error("not sps");
	}

	int i(1), n(0);
	unsigned char* rbsp = new unsigned char[spssize];

	for (; i < spssize; ++i)
	{
		if (i + 2 < spssize && p[i] == 0x0 && p[i + 1] == 0x0 && p[i + 2] == 0x03)
		{
			rbsp[n++] = 0;
			rbsp[n++] = 0;
			i += 2;
		}
		else
		{
			rbsp[n++] = p[i];
		}
	}

	i = 0;
	sps.aspect_ratio_idc = -1; // -1 unknow, 0 no aspect ratio info, 1 yes
	sps.profile_idc = rbsp[i++];
	++i;//skip 1byte
	sps.level_idc = rbsp[i++];

	BitsGet bg(rbsp + i);
	sps.seq_parameter_set_id = get_ue(bg);

	if (sps.profile_idc == 100 ||  // High profile
		sps.profile_idc == 110 ||  // High10 profile
		sps.profile_idc == 122 ||  // High422 profile
		sps.profile_idc == 244 ||  // High444 Predictive profile
		sps.profile_idc == 44 ||  // Cavlc444 profile
		sps.profile_idc == 83 ||  // Scalable Constrained High profile (SVC)
		sps.profile_idc == 86 ||  // Scalable High Intra profile (SVC)
		sps.profile_idc == 118 ||  // Stereo High profile (MVC)
		sps.profile_idc == 128 ||  // Multiview High profile (MVC)
		sps.profile_idc == 138 ||  // Multiview Depth High profile (MVCD)
		sps.profile_idc == 144) {  // old High444 profile

		sps.chroma_format_idc = get_ue(bg);
		if (sps.chroma_format_idc == 3)
		{
			sps.separate_colour_plane_flag = bg.GetBit();
		}

		sps.bit_depth_luma_minus8 = get_ue(bg);
		sps.bit_depth_chroma_minus8 = get_ue(bg);
		sps.qpprime_y_zero_transform_bypass_flag = bg.GetBit();
		sps.seq_scaling_matrix_present_flag = bg.GetBit();
		if (sps.seq_scaling_matrix_present_flag)
		{
			// 处理这个分支太复杂，放弃
			TRACE0("seq_scaling_matrix_present_flag");
			goto End;
		}
	}

	sps.log2_max_frame_num_minus4 = get_ue(bg);
	sps.pic_order_cnt_type = get_ue(bg);
	if (sps.pic_order_cnt_type == 0)
	{
		sps.log2_max_pic_order_cnt_lsb_minus4 = get_ue(bg);
	}
	else if (sps.pic_order_cnt_type == 1)
	{
		TRACE0("sps.pic_order_cnt_type == 1");
		goto End;
	}

	sps.max_num_ref_frames = get_ue(bg);
	sps.gaps_in_frame_num_value_allow_flag = bg.GetBit();
	sps.pic_width_in_mbs_minus1 = get_ue(bg);
	sps.pic_height_in_map_units_minus1 = get_ue(bg);
	sps.frame_mbs_only_flag = bg.GetBit();
	if (!sps.frame_mbs_only_flag)
	{
		sps.mb_adaptive_frame_field_flag = bg.GetBit();
	}
	sps.direct_8x8_interface_flag = bg.GetBit();
	sps.frame_cropping_flag = bg.GetBit();
	if (sps.frame_cropping_flag)
	{
		sps.frame_crop_left_offset = get_ue(bg);
		sps.frame_crop_right_offset = get_ue(bg);
		sps.frame_crop_top_offset = get_ue(bg);
		sps.frame_crop_bottom_offset = get_ue(bg);
	}
	sps.vui_parameters_present_flag = bg.GetBit();

	// vui
	if (sps.vui_parameters_present_flag)
	{
		sps.aspect_ratio_info_present_flag = bg.GetBit();
		if (sps.aspect_ratio_info_present_flag)
		{
			// all we want is this
			sps.aspect_ratio_idc = bg.GetBits(8);
			int h264_pixel_aspect[34] = {
				 0,  1 ,
				 1,  1 ,
				 12, 11 ,
				 10, 11 ,
				 16, 11 ,
				 40, 33 ,
				 24, 11 ,
				 20, 11 ,
				 32, 11 ,
				 80, 33 ,
				 18, 11 ,
				 15, 11 ,
				 64, 33 ,
				 160, 99 ,
				 4,  3 ,
				 3,  2 ,
				 2,  1 ,
			};
			if (sps.aspect_ratio_idc == 255)
			{
				sps.sar_width = bg.GetBits(16);
				sps.sar_height = bg.GetBits(16);
			}
			else if (sps.aspect_ratio_idc < 17)
			{
				sps.sar_width = h264_pixel_aspect[sps.aspect_ratio_idc * 2];
				sps.sar_height = h264_pixel_aspect[sps.aspect_ratio_idc * 2 + 1];
			}
			else
			{
				TRACE0("illegal aspect ratio");
				sps.aspect_ratio_idc = -1;
				goto End;
			}
		}
	}
	// 余下的不管了
End:
	delete[] rbsp;
}

FlvParse::FlvParse():
	m_bWaitFlvHead(true), 
	m_bWaitNextTag(false)
{
}

FlvParse::~FlvParse()
{
}

BitsPut* FlvParse::CreateAACADTSHeader(const AudioSpecificConfig& res, int datalen)
{
	BitsPut* pBP = new BitsPut(7 * 8);

	// create aac adts
	/*
	syncword			12  ; all bits 1
	ID					1   ; 0 mpeg4,1 mpeg2
	layer				2   ; all bits 0
	protection_absent	1   ; 1
	// 2 bytes
	profile             2   ;
	sampling_freq_inex  4
	private_bit         1
	channel_configura   3
	original_copy       1
	home                1

	copyright_bit       1
	copyright_start     1
	aac_frame_length    13  ; size ,include ADTS head size
	adts_buffer_fullness 11
	number_of_data_blocks 2
	*/
	// all 7bytes

	pBP->PutBits(12, 0xfff);
	pBP->PutBits(1, 0);
	pBP->PutBits(2, 0);
	pBP->PutBits(1, 1);
	pBP->PutBits(2, res.aac_seq_head_audioObjectType);
	pBP->PutBits(4, res.aac_seq_head_samplingFrequencyIndex);
	pBP->PutBits(1, 0);
	pBP->PutBits(3, res.aac_seq_head_channelConfiguration);
	pBP->PutBits(1, 0);
	pBP->PutBits(1, 0);
	pBP->PutBits(1, 0);
	pBP->PutBits(1, 0);
	pBP->PutBits(13, 7+datalen);
	pBP->PutBits(11, 0x7ff);
	pBP->PutBits(2, 0);

	return pBP;
}

char* FlvParse::CreateH264Data(const AVCDecoderConfigurationRecord& avc_seq_head, const char* data, int videolen, int & datalen)
{
	char * pdata(0);
	char buf[] = { 0x0, 0x0, 0x0, 0x1 }; //h264 startcode
	char temp[4] = { 0 };
	int count(0), n;

	datalen = 0;

	while (count <videolen)
	{
		switch (avc_seq_head.avc_seq_head_NALUnitLength)
		{
		case 4:
			n = Char4ToLittleInt(data + count);
			count += 4;
			break;
		case 3:
			memcpy(temp + 1, data + count, 3);
			count += 3;
			n = Char4ToLittleInt(temp);
		case 2:
			n = Char2ToLittleInt(data + count);
			count += 2;
			break;
		default:
			n = *(data + count);
			count += 1;
			break;
		}

		count += n;
		datalen += (4 + n);
	}

	pdata = new char[datalen];
	count = 0;
	datalen = 0;
	while (count <videolen)
	{
		memcpy(pdata + datalen, buf, 4);
		datalen += 4;
		switch (avc_seq_head.avc_seq_head_NALUnitLength)
		{
		case 4:
			n = Char4ToLittleInt(data + count);
			count += 4;
			break;
		case 3:
			memcpy(temp + 1, data + count, 3);
			count += 3;
			n = Char4ToLittleInt(temp);
		case 2:
			n = Char2ToLittleInt(data + count);
			count += 2;
			break;
		default:
			n = *(data + count);
			count += 1;
			break;
		}

		memcpy(pdata + datalen, data + count, n);
		count += n;
		datalen += n;
	}

	return pdata;
}

void FlvParse::InputData(const char* pData, int len, FlvParseRes& res)
{
	res.type = FlvPartNone;

	if (m_bWaitNextTag)
	{
		// pretagsize(4) + taghead(11)
		if (len < 15)
		{
			res.m_bNeedMoreData = true;
			res.m_bNotFlv = false;
			res.tagHead.datasize = 4096*2;
			return;
		}

		res.m_iPreTagSize = Char4ToLittleInt(pData);
		ParseTag(pData + 4, len - 4, res);
		if (res.type != FlvPartNone)
			res.m_iUseDataLen += 4;
	}
	else if (m_bWaitFlvHead)
	{
		// file head size 9+
		if (len < 9)
		{
			res.m_bNeedMoreData = true;
			res.m_bNotFlv = false;
			res.tagHead.datasize = 4096*2;
			return;
		}

		ParseFileHead(pData, len, res);
	}
}

void FlvParse::ParseFileHead(const char* pData, int len, FlvParseRes& res)
{
	memcpy(res.flvHead.flv, pData, 3);
	if (strncmp(res.flvHead.flv, "FLV", 3) != 0)
	{
		goto NOFLV;
	}
	pData += 3;

	res.flvHead.ver = *pData;
	if (res.flvHead.ver != 1)
	{
		goto NOFLV;
	}
	pData += 1;

	res.flvHead.flag = *pData;
	pData += 1;

	char temp[4];
	memcpy(temp, pData, 4);
	res.flvHead.headsize = Char4ToLittleInt(temp);
	res.m_iUseDataLen = res.flvHead.headsize;
	res.type = FlvPartFileHead;
	m_bWaitFlvHead = false;
	m_bWaitNextTag = true;
	return;
NOFLV:
	res.m_bNeedMoreData = false;
	res.m_bNotFlv = true;
	return;
}

void FlvParse::ParseTag(const char* pData, int len, FlvParseRes& res)
{
	char temp[4] = { 0 };

	res.tagHead.tagType = *pData;
	if (res.tagHead.tagType != 8 && res.tagHead.tagType != 9 && res.tagHead.tagType != 18)
	{
		goto NOFLV;
	}
	pData += 1;

	memcpy(temp + 1, pData, 3);
	temp[0] = 0;
	res.tagHead.datasize = Char4ToLittleInt(temp);
	if (11 + res.tagHead.datasize > len)
	{
		res.m_bNeedMoreData = true;
		res.m_bNotFlv = false;
		return;
	}

	pData += 3;
	memcpy(temp + 1, pData, 3);
	temp[0] = *(pData + 3);
	res.tagHead.timestamp = Char4ToLittleInt(temp);
	pData += 4;

	memcpy(temp + 1, pData, 3);
	temp[0] = 0;
	res.tagHead.streamid = Char4ToLittleInt(temp);
	pData += 3;
	
	len -= 11;
	if (res.tagHead.tagType == 8)
	{
		//audio
		ParseAudioTag(pData, len, res);
	}
	else if (res.tagHead.tagType == 9)
	{
		//video
		ParseVideoTag(pData, len, res);
	}
	else if (res.tagHead.tagType == 18)
	{
		//script
		ParseScriptTag(pData, len, res);
	}
	if (res.type != FlvPartNone)
		res.m_iUseDataLen += 11;
	return;
NOFLV:
	res.m_bNotFlv = true;
	return;
}

void FlvParse::ParseAudioTag(const char* p, int len, FlvParseRes&res)
{
	char ch = *p;

	res.audioTag.soundFormat = (ch & 0xf0) >> 4;
	res.audioTag.soundRate = (ch & 0x0c) >> 2;
	res.audioTag.soundSize = (ch & 0x02) >> 1;
	res.audioTag.soundType = (ch & 0x01);

	res.audioTag.data = p + 1;
	res.audioTag.datalen = res.tagHead.datasize - 1;

	if (res.audioTag.soundFormat == 10)
	{
		// aac
		ch = *res.audioTag.data;
		if (ch == 0)
		{
			// aac sequence header
			unsigned char a[10] = { 0 };
			memcpy(a, res.audioTag.data+1, res.audioTag.datalen-1);

			ParseAudioSpecificConfig(res.audioTag.aac_seq_head, a, 0);
			memcpy(res.audioTag.aac_seq_head_org, a, 2);
			res.scriptTag.samplerate = res.audioTag.aac_seq_head.aac_seq_head_samplingFrequency;
		}
		else
		{
			// raw data
		}
	}

	res.type = FlvPartAudioTag;
	res.m_iUseDataLen = res.tagHead.datasize;
}

void FlvParse::ParseVideoTag(const char*p, int len, FlvParseRes&res)
{
	char ch = *p;

	res.videoTag.videoFrameType = (ch & 0xf0) >> 4;
	res.videoTag.videoCodecType = (ch & 0x0f);

	res.videoTag.data = p + 1;
	res.videoTag.datalen = res.tagHead.datasize - 1;

	if (res.videoTag.videoCodecType == 7)
	{
		int count(1);
		const char* ptemp = res.videoTag.data;;
		res.videoTag.avc_packet_type = ch = *ptemp;
		count += 3; //skip compositiontime


		if (ch == 1)
		{
			// NALU
			// composition time
			res.videoTag.avc_nalu_compositiontime = (((unsigned char)(*(ptemp + 1))) << 16) +
				(((unsigned char)(*(ptemp + 2))) << 8) +
				(unsigned char)*(ptemp + 3);
		}
		else if (ch == 0)
		{
			// sequence header
			// AVCDecoderConfigurationRecord
			res.videoTag.avc_nalu_compositiontime = 0;
			ParseAVCDecoderConfigurationRecord(res.videoTag.avc_seq_head, ptemp + count, 0);
			res.videoTag.avc_seq_head_org_len = res.videoTag.datalen - count;
			memcpy(res.videoTag.avc_seq_head_org, ptemp + count, res.videoTag.avc_seq_head_org_len);
		}
		else if (ch == 2)
		{
			// sequence end
			res.videoTag.avc_nalu_compositiontime = 0;
		}
	}

	res.type = FlvPartVideoTag;
	res.m_iUseDataLen = res.tagHead.datasize;
}

void FlvParse::ParseScriptTag(const char*p, int len, FlvParseRes&res)
{
	int count(0);
	std::string key;
	char ch;

	res.scriptTag.duration = 0;
	while (count<res.tagHead.datasize)
	{
		ch = *(p + count);
		count += 1;
		switch (ch)
		{
		case 2:
			// string
			count += AMFString(p + count, key);
			if (key == "onMetaData")
			{
				res.scriptTag.onlastsecond = false;
			}
			else if (key == "onLastSecond")
			{
				res.scriptTag.onlastsecond = true;
			}
			else 
			{
				throw std::logic_error("unknown flv sricpt tag " + key);
				goto End;
			}

			break;
		case 8:
			// array
			res.m_bException = false;
			count += AMFECMAArray(p + count, res);
			// array end 0,0,9
			count += 3;
			if (res.m_bException)
				goto End;
			break;
		default:
			goto End;
			break;
		}
	}

End:
	res.type = FlvPartScriptTag;
	res.m_iUseDataLen = res.tagHead.datasize;
}

int FlvParse::AMFString(const char* p, std::string& s)
{
	int count = Char2ToLittleInt(p);
	s = std::string(p + 2, count);
	return count + 2;
}

int FlvParse::AMFNumber(const char* p, double& d)
{
	d = Char8ToDouble(p);
	return 8;
}

int  FlvParse::AMFBoolean(const char* p, bool &b)
{
	if (*p)
		b = true;
	else
		b = false;
	return 1;
}

int FlvParse::AMFECMAArray(const char*p, FlvParseRes& res)
{
	std::string key, value;
	int count(0), arraylen, stringlen;
	char ch;
	double dValue;
	bool bValue;

	arraylen = Char4ToLittleInt(p + count);
	count += 4;
	key = "";

	for (int i = 0; i < arraylen; ++i)
	{
		// key长度
		stringlen = Char2ToLittleInt(p + count);
		count += 2;
		// key name
		key = std::string(p + count, stringlen);
		count += stringlen;

		// value type
		ch = *(p + count);
		count += 1;
		// value
		switch (ch)
		{
		case 0:
			count += AMFNumber(p + count, dValue);
			//printf("%s is %lf\n", key.c_str(), dValue);

			if (key == "duration")
			{
				res.scriptTag.duration = dValue;
			}
			else if (key == "width")
			{
				res.scriptTag.width = dValue;
			}
			else if (key == "height")
			{
				res.scriptTag.height = dValue;
			}
			else if (key == "filesize")
			{
				res.scriptTag.filesize = dValue;
			}
			else if (key == "framerate")
			{
				res.scriptTag.framerate = dValue;
			}
			else if (key == "videocodecid")
			{
				res.scriptTag.videocodec = dValue;
			}
			else if (key == "audiocodecid")
			{
				res.scriptTag.audiocodec = dValue;
			}
			else if (key == "audiosamplerate")
			{
				if (dValue > 16)
				{
					res.scriptTag.samplerate = dValue;
				}
				else
				{
					res.scriptTag.samplerate = ParseSampleRateByIndex(static_cast<int>(dValue));
				}
			}
			else if (key == "audiosize")
			{
				res.scriptTag.audiosize = dValue;
			}
			else if (key == "videosize")
			{
				res.scriptTag.videosize = dValue;
			}
			else if (key == "videodatarate")
			{
				res.scriptTag.videodatarate = dValue;
			}
			else if (key == "audiodatarate")
			{
				res.scriptTag.audiodatarate = dValue;
			}
			
			break;
		case 1:
			count += AMFBoolean(p + count, bValue);
			//printf("%s is %s\n", key.c_str(), bValue ? "true" : "false");
			if (key == "stereo")
			{
				res.scriptTag.stereo = bValue;
			}
			break;
		case 2:
			count += AMFString(p + count, value);
			//printf("%s is %s\n", key.c_str(), value.c_str());
			break;
		case 3:
			if (key != "keyframes")
			{
				res.m_bException = true;
				return count;
			}

			count += AMFObject(p + count, res);
			if (res.m_bException)
				return count;
			break;
		default:
			res.m_bException = true;
			return count;
			break;
		}

	}

	return count;
}

int FlvParse::AMFObject(const char*p, FlvParseRes& res)
{
	int n, count(0);
	std::string key, value;
	char end[] = { 0,0,9 }, ch;

	while (1)
	{
		// check end
		if (memcmp(p + count, end, 3) == 0)
		{
			count += 3;
			break;
		}

		// key
		n = Char2ToLittleInt(p+count);
		count += 2;
		key = std::string(p + count, n);
		count += n;

		//value
		ch = *(p + count);
		count += 1;
		switch (ch)
		{
		case 10:
			if (key == "filepositions")
			{
				std::vector<double> vTempPos;
				count += AMFStrictArray(p + count, res, vTempPos);
				if (!vTempPos.empty())
				{
					res.scriptTag.m_vecPos.resize(vTempPos.size());
					for (size_t i = 0; i < vTempPos.size(); ++i)
					{
						res.scriptTag.m_vecPos[i] = static_cast<int>(vTempPos[i]);
					}
				}
			}
			else if (key == "times")
			{
				res.scriptTag.m_vecTime.clear();
				count += AMFStrictArray(p + count, res, res.scriptTag.m_vecTime);
			}
			else
			{
				res.m_bException = true;
			}
			if (res.m_bException)
				return count;
			break;
		default:
			res.m_bException = true;
			return count;
			break;
		}
	}

	return count;
}

int FlvParse::AMFStrictArray(const char*p, FlvParseRes&res, std::vector<double>&v)
{
	double d;
	int n, count(0);
	char ch;

	// array len
	n = Char4ToLittleInt(p);
	count += 4;

	for (int i = 0; i < n; ++i)
	{
		ch = *(p + count);
		count += 1;

		switch (ch)
		{
		case 0:
			count += AMFNumber(p + count, d);
			v.push_back(d);
			break;
		default:
			res.m_bException = true;
			return count;
			break;
		}
	}


	return count;
}