#pragma once
#include <string>
#include <vector>
#include "BitsPut.h"

void WriteInt32(int32_t, char *a);
void WriteInt16(int16_t, char *a);
void WriteInt64(int64_t, char *a);

int64_t Char8ToLittleInt(const char*);
int32_t Char4ToLittleInt(const char*);
int32_t Char2ToLittleInt(const char*);
double Char8ToDouble(const char*);
double FixedPointNumber88(const char*);
double FixedPointNumber1616(const char*);
void ToFixedPointNumber1616(double d, char a[]);

struct H264SPSInfo
{
	char forbidden_zero_bit;
	char nal_ref_idc;
	char nal_unit_type; // 7 sps, 8 sps, 5 I frame

	unsigned char profile_idc; // 1byte
	//char constraint_set_flag[5];
	//char reserved_zero_3bits[3]; // 1byte
	char level_idc;  // 1byte
	int seq_parameter_set_id; // ue(v)
	int chroma_format_idc;// ue(v)
	char separate_colour_plane_flag; // u(1)
	int bit_depth_luma_minus8;
	int bit_depth_chroma_minus8;
	char qpprime_y_zero_transform_bypass_flag;
	char seq_scaling_matrix_present_flag;
	int log2_max_frame_num_minus4;
	int pic_order_cnt_type;
		int log2_max_pic_order_cnt_lsb_minus4;
	int max_num_ref_frames;
	char gaps_in_frame_num_value_allow_flag;
	int pic_width_in_mbs_minus1;
	int pic_height_in_map_units_minus1;
	char frame_mbs_only_flag;
		char mb_adaptive_frame_field_flag;
	char direct_8x8_interface_flag;
	char frame_cropping_flag;
		int frame_crop_left_offset;
		int frame_crop_right_offset;
		int frame_crop_top_offset;
		int frame_crop_bottom_offset;
	char vui_parameters_present_flag;
	int aspect_ratio_info_present_flag; //u(1)
	int aspect_ratio_idc;// u(8)
		int sar_width;// u(16)
		int sar_height;// u(16)
};

struct AudioSpecificConfig
{
	char aac_seq_head_audioObjectType;        // 5 bits; 1 aac main, 2 aac lc,  
	char aac_seq_head_samplingFrequencyIndex; // 4 bits; 0 96000, 1 88200, 2 64000, 3 48000, 4 44100, 5 32000, 16 取决于samplingFrequency
	int aac_seq_head_samplingFrequency;       // 24 bits;
	char aac_seq_head_channelConfiguration;   // 4 bits; 声道信息
};

struct AVCDecoderConfigurationRecord
{
	char avc_seq_head_version;
	char avc_seq_head_profile;
	char avc_seq_head_profilecompatibility;
	char avc_seq_head_level;

	char avc_seq_head_reserved; ; //6bits
	char avc_seq_head_NALUnitLength; //2bits

	char avc_seq_head_spsnum;
	char avc_seq_head_ppsnum;
	char avc_seq_head_data[256];
	int avc_seq_head_datalen;
};

void ParseAVCDecoderConfigurationRecord(AVCDecoderConfigurationRecord& avc_head, const char*, int);
void ParseAudioSpecificConfig(AudioSpecificConfig& aac_head, const unsigned  char*, int);
int ParseSampleRateByIndex(int iii);
void ParseSPS(H264SPSInfo& sps, const unsigned char*, int);

enum FlvPartType
{
	FlvPartNone,
	FlvPartFileHead,
	FlvPartAudioTag,
	FlvPartVideoTag,
	FlvPartScriptTag
};

struct FlvFileHead
{
	char flv[3]; //FLV; take 3 bytes;
	char ver;  //1; take 1byte
	char flag; // 1 has audio, 4 has video, 5 has both audio and video, 0 nothing ; take 1 byte
	int headsize; //9 or more than 9 ; take 4 bytes
}; // take 9+ bytes

struct FlvTagHead
{
	char tagType;
	/*
	2bits alway 0
	1bit  encrypted or not , should 0
	5bits 8 audio tag, 9 video tag, 18 script tag
	*/
	// 8 audio , 9 video , 18 script; take 1 byte
	int datasize;  // data size follow; take 3 bytes
	int timestamp; // dts relate to tag 0; take 4 bytes
	int streamid;  // 0; take 3 bytes
};// take 11 bytes

struct FlvAudioTagData
{
	char soundFormat; // 2bits; 2 mp3, 10 aac
	char soundRate;   // 2bits; 0--5.5khz 1--11khz 2--22khz 3--44khz
	char soundSize;   // 1bit ; 0--8 bits sample  1--16 bits sample
	char soundType;   // 1bit ; stereo or mono
	// take 1byte

	// sound data
	const char *data;
	int datalen;

	// aac audio AudioSpecificConfig
	AudioSpecificConfig aac_seq_head;
	char aac_seq_head_org[2]; // 2 bytes
};

struct FlvVideoTagData
{
	char videoFrameType; // 4bits; 1 avc key frame, 2 avc inter frame
	char videoCodecType; // 4bits; 7 avc
						 // take 1byte
						 // video data
	const char *data;
	int datalen;

	char avc_packet_type; ; // 0 seq head, 1 nalu, 2 seq end
	int avc_nalu_compositiontime; // take 3 bytes, =0 in seq head and seq end
								  // avc sequence head
	AVCDecoderConfigurationRecord avc_seq_head;
	char avc_seq_head_org[256]; // 不定字节长
	int avc_seq_head_org_len;
};

struct FlvScriptTagData
{
	double filesize;
	double duration;
	double width;
	double height;
	double framerate;
	double samplerate;
	double videocodec;
	double videosize;
	double videodatarate;
	double audiocodec;
	double audiosize;
	double audiodatarate;
	bool stereo;
	bool onlastsecond;
	std::vector<int> m_vecPos;
	std::vector<double> m_vecTime;

	FlvScriptTagData():
		filesize(0),
		duration(0),
		width(0),
		height(0),
		framerate(0),
		samplerate(0),
		videocodec(0),
		videosize(0),
		audiocodec(0),
		audiosize(0),
		stereo(true),
		onlastsecond(false)
	{
	}
};

struct FlvParseRes
{
	FlvPartType type;
	FlvFileHead flvHead;
	FlvTagHead tagHead;
	FlvAudioTagData audioTag;
	FlvVideoTagData videoTag;
	FlvScriptTagData scriptTag;
	bool m_bNeedMoreData;
	bool m_bNotFlv;
	bool m_bException;
	int m_iUseDataLen;
	int m_iPreTagSize;
};

class FlvParse
{
public:
	FlvParse();
	~FlvParse();
	static BitsPut* CreateAACADTSHeader(const AudioSpecificConfig& res, int datalen);
	static char* CreateH264Data(const AVCDecoderConfigurationRecord&, const char* pdata, int datalen, int & len);

	void InputData(const char* pData, int len, FlvParseRes& res);
protected:
	void ParseFileHead(const char* pData, int len, FlvParseRes& res);
	void ParseTag(const char* pData, int len, FlvParseRes& res);
	void ParseAudioTag(const char*, int, FlvParseRes&);
	void ParseVideoTag(const char*, int, FlvParseRes&);
	void ParseScriptTag(const char*, int, FlvParseRes&);
	int AMFString(const char* p, std::string& s);
	int AMFNumber(const char* p, double& d);
	int AMFBoolean(const char* p, bool &b);
	int AMFECMAArray(const char*p, FlvParseRes&);
	int AMFObject(const char*, FlvParseRes&);
	int AMFStrictArray(const char*p, FlvParseRes&, std::vector<double>& v);
public:
	bool m_bWaitFlvHead;
	bool m_bWaitNextTag;
};
