#include "ReadFlv.h"
#include "FlvParse.h"
#include "Mp4Parse.h"
#include "DashConvert.h"
#include "Vs4DashConvert.h"
#include <fstream>
#include <iostream>
#include <cassert>

void TestReadFlv()
{
	int buflen = 4096;
	char *buf = new char[buflen];

	std::ifstream fileIn;

	fileIn.open("D:\\Downloads\\nginx-1.12.0\\html\\video\\1.flv", std::ifstream::binary | std::ifstream::in);
	fileIn.read(buf, buflen);

	FlvParse p;
	FlvParseRes res;
	int len = fileIn.gcount();
	char* pData = buf, *ptemp, ch;

	std::ofstream fileOut;
	fileOut.open("d:\\1.aac", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	std::ofstream fileOut1;
	fileOut1.open("d:\\1.h264", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);


	while (len > 0)
	{
		p.InputData(pData, len, res);

		if (res.type == FlvPartNone)
		{
			if (res.m_bNotFlv)
			{
				TRACE0("not a flv!!!\n");
				break;
			}
			else if (res.m_bNeedMoreData)
			{
				TRACE0("need more data\n");
				goto ReadMoreData;
			}
		}
		else
		{
			TRACE1("pre tag size %d\n", res.m_iPreTagSize);
			switch (res.type)
			{
			case FlvPartFileHead:
				TRACE1("file head about audio and video info %d\n", (int)res.flvHead.flag);
				break;
			case FlvPartAudioTag:
				std::cout << "audio type is " << (int)res.audioTag.soundFormat << " audio data len is " << (int)res.audioTag.datalen << std::endl;
				if (res.audioTag.soundFormat == 10)
				{
					ch = *res.audioTag.data;
					if (ch == 0)
					{
					}
					else
					{
						//std::cout << res.tagHead.timestamp << std::endl;
						BitsPut* pBP = FlvParse::CreateAACADTSHeader(res.audioTag.aac_seq_head, res.audioTag.datalen - 1);
						fileOut.write((char*)pBP->GetBitsData(), 7);
						fileOut.write(res.audioTag.data + 1, res.audioTag.datalen - 1);
						delete pBP;
					}
				}
				break;
			case FlvPartVideoTag:
				std::cout << "video " << (int)res.videoTag.videoCodecType << " ts is "<<res.tagHead.timestamp<<std::endl;
				std::cout << "pts is " << res.tagHead.timestamp + res.videoTag.avc_nalu_compositiontime << " cts is " << res.videoTag.avc_nalu_compositiontime << " dts is "
					<< res.tagHead.timestamp << std::endl;
				if (res.videoTag.videoCodecType == 7)
				{
					if (res.videoTag.avc_packet_type == 1)
					{
						int ss;
						char * pp1 = FlvParse::CreateH264Data(res.videoTag.avc_seq_head,
							res.videoTag.data + 4,
							res.videoTag.datalen - 4,
							ss);
						fileOut1.write(pp1, ss);
						delete[]pp1;
					}
					else if (res.videoTag.avc_packet_type == 0)
					{
						fileOut1.write(res.videoTag.avc_seq_head.avc_seq_head_data, res.videoTag.avc_seq_head.avc_seq_head_datalen);
					}
				}

				break;
			case FlvPartScriptTag:
				break;
			default:
				std::cout << "end" << std::endl;
				break;
			}
		}

		pData += res.m_iUseDataLen;
		len -= res.m_iUseDataLen;
		if (len > 0)
			continue;
		else
			std::cout << "need more" << std::endl;

	ReadMoreData:
		if (fileIn.eof())
		{
			break;
		}

		res.tagHead.datasize += 4096;
		if (res.tagHead.datasize > buflen)
		{
			ptemp = new char[res.tagHead.datasize];
			if (len > 0)
			{
				memcpy(ptemp, pData, len);
			}
			delete[]buf;
			buf = ptemp;
			buflen = res.tagHead.datasize;
		}
		else
		{
			if (len > 0)
			{
				memmove(buf, buf + buflen - len, len);
			}
		}

		fileIn.read(buf + len, buflen - len);
		len += fileIn.gcount();
		pData = buf;
	}

	fileIn.close();
	fileOut.close();
	fileOut1.close();
	delete[]buf;
}

void TestReadMp4()
{
	unsigned int buflen = 4096, len;
	char *buf = new char[buflen];
	char *pdata = buf;

	std::ifstream fileIn;
	fileIn.open("D:\\Downloads\\nginx-1.12.0\\html\\video\\1.mp4", std::ifstream::binary | std::ifstream::in);

	fileIn.read(buf, buflen);
	len = fileIn.gcount();

	Mp4Parse mp;
	Mp4ParseRes &res = mp.GetRes();

	while (1)
	{
		mp.InputData(pdata, len);
		pdata += res.m_iUsedDataLen;
		len -= res.m_iUsedDataLen;

		if (res.m_iGotBox)
		{
			if (strncmp(res.boxHead.type, "moov", 4) == 0)
			{
				std::ofstream fileOut;
				fileOut.open("d:\\1.aac", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
				std::ofstream fileOut1;
				fileOut1.open("d:\\1.h264", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

				for (int i = 0; i < res.moov.track_audio.stsz_offsettable.size(); ++i)
				{
					BitsPut* pBP = FlvParse::CreateAACADTSHeader(res.moov.track_audio.stsd_a_aac_head,
						res.moov.track_audio.stsz_sizetable[i]);
					fileOut.write((char*)pBP->GetBitsData(), 7);

					if (buflen < res.moov.track_audio.stsz_sizetable[i])
					{
						delete[]buf;
						buf = new char[res.moov.track_audio.stsz_sizetable[i]];
						buflen = res.moov.track_audio.stsz_sizetable[i];
					}

					fileIn.seekg(res.moov.track_audio.stsz_offsettable[i], std::ifstream::beg);
					fileIn.read(buf, res.moov.track_audio.stsz_sizetable[i]);
					fileOut.write(buf, res.moov.track_audio.stsz_sizetable[i]);
					delete pBP;
				}


				fileOut1.write(res.moov.track_video.stsd_v_avc_head.avc_seq_head_data, 
					res.moov.track_video.stsd_v_avc_head.avc_seq_head_datalen);
				int ss;
				for (int i = 0; i < res.moov.track_video.stsz_offsettable.size(); ++i)
				{
					if (buflen < res.moov.track_video.stsz_sizetable[i])
					{
						delete[]buf;
						buf = new char[res.moov.track_video.stsz_sizetable[i]];
						buflen = res.moov.track_video.stsz_sizetable[i];
					}

					fileIn.seekg(res.moov.track_video.stsz_offsettable[i], std::ifstream::beg);
					fileIn.read(buf, res.moov.track_video.stsz_sizetable[i]);
					auto pp=FlvParse::CreateH264Data(res.moov.track_video.stsd_v_avc_head, buf, res.moov.track_video.stsz_sizetable[i], ss);
					fileOut1.write(pp, ss);
					delete[]pp;
				}

				break;
			}
		}
		else
		{
			if (res.m_iException)
				break;
			else if (res.m_iNeedMoreData)
				goto ReadMore;
		}

		continue;

	ReadMore:

		if (fileIn.eof())
		{
			break;
		}

		if (res.boxHead.size > buflen)
		{
			char* ptemp = new char[res.boxHead.size];

			if (len > 0)
			{
				memcpy(ptemp, pdata, len);
			}

			delete[]buf;
			buf = ptemp;
			pdata = buf;
			buflen = res.boxHead.size;
		}
		else
		{
			if (len > 0)
				memcpy(buf, pdata, len);
			pdata = buf;
		}

		fileIn.read(pdata + len, buflen - len);
		len += fileIn.gcount();
	}

	delete[] buf;
}

void TestReadMp4_dash()
{
	unsigned int buflen = 4096, len;
	char *buf = new char[buflen];
	char *pdata = buf;

	std::ifstream fileIn;
	//fileIn.open("D:\\Downloads\\nginx-1.12.0\\html\\videoplayer\\2_dashinit.mp4", std::ifstream::binary | std::ifstream::in);
	fileIn.open("d:\\1_dash.mp4", std::ifstream::binary | std::ifstream::in);


	fileIn.read(buf, buflen);
	len = fileIn.gcount();

	Mp4Parse mp;
	Mp4ParseRes &res = mp.GetRes();

	std::ofstream fileOut;
	fileOut.open("d:\\1.aac", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	std::ofstream fileOut1;
	fileOut1.open("d:\\1.h264", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

	while (1)
	{
		mp.InputData(pdata, len);
		pdata += res.m_iUsedDataLen;
		len -= res.m_iUsedDataLen;

		if (res.m_iGotBox)
		{
			if (strncmp(res.boxHead.type, "mdat", 4) == 0)
			{
				int ss;
				auto pmdat = pdata - res.m_iUsedDataLen;
				pmdat += 8;
				for (int i = 0; i < res.moof.traf_video.trun_samplesize.size(); ++i)
				{
					auto ppp=FlvParse::CreateH264Data(res.moov.track_video.stsd_v_avc_head, pmdat, res.moof.traf_video.trun_samplesize[i], ss);
					fileOut1.write(ppp, ss);
					delete[]ppp;
					pmdat += res.moof.traf_video.trun_samplesize[i];
				}

				for (int i = 0; i < res.moof.traf_audio.trun_samplesize.size(); ++i)
				{
					auto ppp = FlvParse::CreateAACADTSHeader(res.moov.track_audio.stsd_a_aac_head, res.moof.traf_audio.trun_samplesize[i]);
					fileOut.write((char*)ppp->GetBitsData(), 7);
					fileOut.write(pmdat, res.moof.traf_audio.trun_samplesize[i]);
					delete ppp;
					pmdat += res.moof.traf_audio.trun_samplesize[i];
				}
			}
			else if (strncmp(res.boxHead.type ,"moov", 4) == 0)
			{
				fileOut1.write(res.moov.track_video.stsd_v_avc_head.avc_seq_head_data, res.moov.track_video.stsd_v_avc_head.avc_seq_head_datalen);
			}
		}
		else
		{
			if (res.m_iException)
				break;
			else if (res.m_iNeedMoreData)
				goto ReadMore;
		}

		continue;

	ReadMore:

		if (fileIn.eof())
		{
			break;
		}

		if (res.boxHead.size > buflen)
		{
			char* ptemp = new char[res.boxHead.size];

			if (len > 0)
			{
				memcpy(ptemp, pdata, len);
			}

			delete[]buf;
			buf = ptemp;
			pdata = buf;
			buflen = res.boxHead.size;
		}
		else
		{
			if (len > 0)
				memcpy(buf, pdata, len);
			pdata = buf;
		}

		fileIn.read(pdata + len, buflen - len);
		len += fileIn.gcount();
	}

	delete[] buf;
}

void TestDashConvert()
{
	DashConvert dc;

	int buflen = 409600;
	int readlen = buflen;
	char *buf = new char[buflen];


	std::ifstream fileIn;
	//fileIn.open("D:\\Downloads\\nginx-1.12.0\\html\\videoplayer\\FE0350486C8505E5324D052B634DB692.flv", std::ifstream::binary | std::ifstream::in);
	fileIn.open("D:\\Downloads\\nginx-1.12.0\\html\\videoplayer\\22.flv", std::ifstream::binary | std::ifstream::in);

	std::ofstream fileOut;
	fileOut.open("d:\\2_dash.mp4", std::ofstream::binary | std::ofstream::out);

	int n, len;
	while (1)
	{
		if (fileIn.eof())
		{

			if (dc.CachedFlvLen() > 4 && n!=3)
			{
				TRACE0("has flv data");
			}
			else
			{
				n = dc.GetLastSeg(buf, readlen);
				if (n > 0)
				{
					fileOut.write(buf, n);
				}
				break;
			}
			
		}

		fileIn.read(buf, buflen);
		n = dc.AppendData(buf, static_cast<int>(fileIn.gcount()));
		//TRACE1("res is %d", n);
		switch (n)
		{
		case DashConvert::DCNeedMoreData:
			continue;
			break;
		case DashConvert::DCGotNormalFragment:
			len = dc.GetNormalSeg(0, 0);
			len *= -1;
			if (len > readlen)
			{
				readlen = len;
				delete[]buf;
				buf = new char[len];
			}
			if (len != dc.GetNormalSeg(buf, len))
				throw 2;

			{
				int pos = fileOut.tellp(), pos1;
				int done(0);
				do
				{
					fileOut.write(buf + done, len - done);
					pos1 = fileOut.tellp();
					done = pos1 - pos;
				} while (done != len);

			}
			break;
		case DashConvert::DCGotInitlFragment:
			len = dc.GetInitSeg(0, 0);
			len *= -1;
			dc.GetInitSeg(buf, len);
			fileOut.write(buf, len);
			break;
		default:
			TRACE0("something happened");
			break;
		}
	}

	delete[]buf;
	fileIn.close();
	fileOut.close();
}

void TestDashConvertGOP()
{
	DashConvertGOP dcgop;

	int buflen = 409600;
	int readlen = buflen;
	char *buf = new char[buflen];

	int index(1), indexnum;
	int pos, len;

	std::ifstream fileIn;
	fileIn.open("C:\\Users\\z\\Downloads\\nginx-1.12.1\\html\\video\\gg.flv", std::ifstream::binary | std::ifstream::in);

	std::ofstream fileOut;
	fileOut.open("d:\\1_dash.mp4", std::ofstream::binary | std::ofstream::out);

	int n;

	fileIn.read(buf, buflen);
	n = dcgop.AppendData(buf, static_cast<int>(fileIn.gcount()));

	while (1)
	{
		switch (n)
		{
		case DashConvert::DCNeedMoreData:

			fileIn.read(buf, buflen);
			n = dcgop.AppendData(buf, static_cast<int>(fileIn.gcount()));
			continue;
			break;
		case DashConvert::DCGotNormalFragment:
			len = dcgop.GetNormalSeg(0, 0);
			len *= -1;
			if (len > readlen)
			{
				readlen = len;
				delete[]buf;
				buf = new char[len];
			}
			if (len != dcgop.GetNormalSeg(buf, len))
				throw 2;

			{
				int pos = fileOut.tellp(), pos1;
				int done(0);
				do
				{
					fileOut.write(buf + done, len - done);
					pos1 = fileOut.tellp();
					done = pos1 - pos;
				} while (done != len);

			}

			// 最后一个seg, 只包含avc seq end tag，跳过
			if (index < indexnum)
			{

				dcgop.GetSegReqPosReqBufLen(index++, pos, len);

				if (len > 0)
				{
					if (len > readlen)
					{
						readlen = len;
						delete[]buf;
						buf = new char[len];
					}

					n = fileIn.tellg();
					if (n != pos) {
						fileIn.seekg(pos, std::ifstream::beg);
					}
					fileIn.read(buf, len);
					n = dcgop.AppendData(buf, static_cast<int>(fileIn.gcount()));
					continue;
				}
			}
			else
			{
				goto End;
			}
			break;
		case DashConvert::DCGotInitlFragment:
			len = dcgop.GetInitSeg(0, 0);
			len *= -1;
			dcgop.GetInitSeg(buf, len);
			fileOut.write(buf, len);

			{
				indexnum = dcgop.GetSegNumber();
				index = 1;
				dcgop.GetSegReqPosReqBufLen(index++, pos, len);

				if (len > 0)
				{
					if (len > readlen)
					{
						readlen = len;
						delete[]buf;
						buf = new char[len];
					}

					fileIn.seekg(pos, std::ifstream::beg);
					fileIn.read(buf, len);
					n = dcgop.AppendData(buf, static_cast<int>(fileIn.gcount()));
					continue;
				}
			}
			break;
		default:
			TRACE0("something happened");
			break;
		}
	}

End:

	delete[]buf;
	fileIn.close();
	fileOut.close();
}

void TestVs4DashConvert()
{
	Vs4DashConvert dcgop;

	int buflen = 409600;
	int readlen = buflen;
	char *buf = new char[buflen];

	int index(1), indexnum;
	int pos, len;

	std::ifstream fileIn;
	fileIn.open("D:\\Downloads\\nginx-1.12.0\\html\\video\\氢气在氯气中的燃烧.vs4", std::ifstream::binary | std::ifstream::in);

	std::ofstream fileOut;
	fileOut.open("d:\\vs4_dash.mp4", std::ofstream::binary | std::ofstream::out);

	int n;

	fileIn.read(buf, buflen);
	n = dcgop.AppendData(buf, static_cast<int>(fileIn.gcount()));

	while (1)
	{
		switch (n)
		{
		case DashConvert::DCNeedMoreData:

			fileIn.read(buf, buflen);
			n = static_cast<int>(fileIn.gcount());
			n = dcgop.AppendData(buf, n);
			continue;
			break;
		case DashConvert::DCGotNormalFragment:
			len = dcgop.GetNormalSeg(0, 0);
			len *= -1;
			if (len > readlen)
			{
				readlen = len;
				delete[]buf;
				buf = new char[len];
			}
			if (len != dcgop.GetNormalSeg(buf, len))
				throw 2;

			{
				int pos = fileOut.tellp(), pos1;
				int done(0);
				do
				{
					fileOut.write(buf + done, len - done);
					pos1 = fileOut.tellp();
					done = pos1 - pos;
				} while (done != len);

			}

			// 最后一个seg, 只包含avc seq end tag，跳过
			if (index < indexnum)
			{

				dcgop.GetSegReqPosReqBufLen(index++, pos, len);

				if (len > 0)
				{
					if (len > readlen)
					{
						readlen = len;
						delete[]buf;
						buf = new char[len];
					}

					n = fileIn.tellg();
					if (n != pos) {
						fileIn.seekg(pos, std::ifstream::beg);
					}
					fileIn.read(buf, len);
					n = dcgop.AppendData(buf, static_cast<int>(fileIn.gcount()));
					continue;
				}
			}
			else
			{
				goto End;
			}
			break;
		case DashConvert::DCGotInitlFragment:
			len = dcgop.GetInitSeg(0, 0);
			len *= -1;
			dcgop.GetInitSeg(buf, len);
			fileOut.write(buf, len);

			{
				indexnum = dcgop.GetSegNumber();
				index = 1;
				dcgop.GetSegReqPosReqBufLen(index++, pos, len);

				if (len > 0)
				{
					if (len > readlen)
					{
						readlen = len;
						delete[]buf;
						buf = new char[len];
					}

					fileIn.seekg(pos, std::ifstream::beg);
					fileIn.read(buf, len);
					n = dcgop.AppendData(buf, static_cast<int>(fileIn.gcount()));
					continue;
				}
				else
				{
					n= dcgop.AppendData(buf, 0);
					continue;
				}
			}
			break;
		default:
			TRACE0("something happened");
			goto End;
			break;
		}
	}

End:

	delete[]buf;
	fileIn.close();
	fileOut.close();
}