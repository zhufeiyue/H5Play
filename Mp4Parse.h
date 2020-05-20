#pragma once
#include "FlvParse.h"
#include <tuple>

struct Mp4SampleFlags
{
	// reserved UI6
	char sampleDependsOn;  // UI2 0 unknown, 1 does depend on , 2 does't depend on, 3 resrved
	char samplesDependedOn; // UI2 0 unknown, 1 does depend on , 2 does't depend on, 3 resrved
	char sampleHasRedundancy; // UI2 0 unknow, 1 there is, 2 there is no, 3 reserved
	// samplepadding value  UI3 reserved. set to 0
	char samplesDifferenceSample; // UI1 0 key, 1 non-key
	// smapleDegradationPriority UI16 reserved, set to 1
};

struct Mp4BoxHead
{
	uint32_t size;
	char type[4];
	char majorBand[4];
	char minVersion[4];
	uint64_t sizeex;
	uint32_t boxheadsize;
};

struct Mp4Track
{
	char tkhd_version;
	char thkd_flag[4];
	uint64_t tkhd_createtime;
	uint64_t tkhd_moditime;
	uint32_t trackid;
	uint64_t tkhd_duration;
	float tkhd_width;
	float tkhd_height;

	//mdia
	char mdhd_version;
	uint64_t mdhd_createtime;
	uint64_t mdhd_moditime;
	uint32_t mdhd_timescale;
	uint64_t mdhd_duration;
	char mdhd_language[3];
	char hdlr_type[4]; //vide or soun
	//mdia-minf
	uint16_t vmhd_graphicsMode;
	uint16_t vmhd_opcolor[3];
	float smhd_balance; // 0 center
	//mdia-minf-stbl
	uint16_t stsd_v_width;
	uint16_t stsd_v_height;
	float    stsd_v_resolution_h;
	float    stsd_v_resolution_w;
	uint16_t stsd_v_framecount;
	uint16_t stsd_v_depth;
	char     stsd_v_codectype[4];
	char     stsd_v_compressorname[32];
	char     stsd_a_codectype[4];
	uint16_t stsd_a_channelcount;
	uint16_t stsd_a_samplesize;
	float    stsd_a_samplerate;
	char     stsd_a_esds_objectTypeID; // 0x40 aac 0x69 0x6b mp3

	AudioSpecificConfig stsd_a_aac_head;
	AVCDecoderConfigurationRecord stsd_v_avc_head;

	std::vector<uint32_t> stts_samplecount;
	std::vector<uint32_t> stts_sampledelta;
	std::vector<uint32_t> stsc_firstchunk;
	std::vector<uint32_t> stsc_sampleperchunk;
	std::vector<uint32_t> stsc_sampledescindex;
	std::vector<uint32_t> ctts_samplecount;
	std::vector<uint32_t> ctts_sampleoffset;
	std::vector<uint32_t> stsz_sizetable;
	std::vector<uint32_t> stsz_offsettable;
	uint32_t              stsz_constantsize;
	uint32_t              stco_offsetcount;
	std::vector<uint32_t> stco_offsets;
	std::vector<uint32_t> stss_synctable;
};

struct Mp4Mvex
{
	char mehd_version;
	uint64_t mehd_fragmentduration;
	std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, Mp4SampleFlags> > trexs;;
};

struct Mp4Moov
{
	char     mvhd_version;
	uint64_t mvhd_createtime;
	uint64_t mvhd_moditime;
	uint32_t mvhd_timescale;
	uint64_t mvhd_duration;
	Mp4Mvex  mvex;
	Mp4Track track_video;
	Mp4Track track_audio;
};

struct Mp4TrackFragment
{
	uint32_t tfhd_flags;
	uint32_t tfhd_trackid;
	uint64_t tfhd_basedataoffset;
	uint64_t tfdt_basemediadecodetime;
	uint32_t trun_flags;
	uint32_t trun_samplecount;
	int32_t  trun_dataoffset;
	Mp4SampleFlags trun_firstsampleflags;
	std::vector<uint32_t>       trun_sampleduration;
	std::vector<uint32_t>       trun_samplesize;
	std::vector<Mp4SampleFlags> trun_sampleflags;
	std::vector<uint32_t>       trun_sampleflags1;
	std::vector<uint32_t>       trun_samplecompositiontimeoffset;
};

struct Mp4Moof
{
	uint32_t mfhd_seqno;
	Mp4TrackFragment traf_video;
	Mp4TrackFragment traf_audio;
};

struct Mp4Sidx
{
	uint32_t referenceID;
	uint32_t timescale;
	uint64_t earliest_presentation_time;
	uint64_t first_offset;
	uint16_t reference_count;

	struct ReferenceItem
	{
		char referencetype;
		char starts_width_SAP;
		char SAP_type;
		uint32_t referencesize;
		uint32_t subsegment_duration;
		uint32_t SAP_delta_time;
	};

	std::vector<ReferenceItem> ref_items;
};

struct Mp4ParseRes
{
	int m_iGotBox;
	Mp4BoxHead boxHead;
	int m_iNeedMoreData;
	int m_iException;
	int m_iUsedDataLen;
	Mp4Moov moov;
	Mp4Sidx sidx;
	Mp4Moof moof;
};

class Mp4Parse
{
public:
	Mp4Parse();
	~Mp4Parse();

	Mp4ParseRes& GetRes() { return m_res; }
	void InputData(const char*, int);
protected:
	int ParseBoxHead(const char*, int);
	void Parse_moof(const char*, int);
	void Parse_moof_traf(const char*, int, Mp4TrackFragment&);
	void Parse_moov(const char*, int);
	void Parse_moov_mvex(const char*, int);
	void Parse_moov_track(const char*, int, Mp4Track&);
	void Parse_moov_track_mdia(const char*, int, Mp4Track&);
	void Parse_moov_track_mdia_minf(const char*, int, Mp4Track&);
	void Parse_moov_track_mdia_minf_stbl(const char*, int, Mp4Track&);
	void Cal_stbl(Mp4Track&);
	void Cal_SampleFlags(const char*, Mp4SampleFlags&);
protected:
	Mp4ParseRes m_res;
};

// dash only
class Mp4Muxer
{
public:
	Mp4ParseRes muxerParams;
	char avcHeadData[256];
	char aacHeadData[8];
	int avcHeadLen;
	int aacHeadLen;

	int Create_BoxHead(char* , int , const char* );
	int Create_ftyp(char * , int );
	int Create_moov(char*, int);
	int Create_moov_mvhd(char*, int);
	int Create_moov_mvex(char*, int);
	int Create_moov_track(char*, int, const Mp4Track&);
	int Create_moov_track_mdia(char*, int, const Mp4Track&);
	int Create_moov_track_mdia_minf(char*, int, const Mp4Track&);
	int Create_moov_track_mdia_minf_stbl(char*, int, const Mp4Track&);
	int Create_moof(char*, int, int);
	int Create_moof_traf(char*, int, Mp4TrackFragment&);
};