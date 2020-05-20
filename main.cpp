#include <iostream>
#include <stdio.h>

#include "ReadFlv.h"
#include "FlvParse.h"
#include "Mp4Parse.h"
#include "DashConvert.h"
#include "Vs4DashConvert.h"

std::unique_ptr<Vs4DashConvert> pIns;

// clean
void bb()
{
	TRACE0("reset");
	pIns.reset(new Vs4DashConvert());
}

// appenddata
int aa(char* data, int len)
{
	//if (!pIns)
	//	return DashConvert::DCUnknow;

	int res(DashConvert::DCErrorHappen);
	try
	{
		res = pIns->AppendData(data, len);
	}
	catch (std::logic_error& e) 
	{
		TRACE1("logical error --- %s", e.what());
	}
	catch (...)
	{
		TRACE0("v unknown error");
		return DashConvert::DCErrorHappen;
	}
	return res;
}

// GetInitSeg
int ab(char* p, int len)
{
	return pIns->GetInitSeg(p, len);
}

// GetNormalSeg
int ac(char* p, int l)
{
	int res(0);

	try
	{
		res = pIns->GetNormalSeg(p, l);
	}
	catch (std::logic_error& e)
	{
		TRACE1("%s", e.what());
	}
	catch (...)
	{
		return res;
	}

	return res;
}

// GetSegNumber
int a_()
{
	return pIns->GetSegNumber();
}

static char stackbuf[128];
// GetSegPosLen
char* ad(int i)
{
	int pos, len;

	pIns->GetSegReqPosReqBufLen(i, pos, len);
#ifdef _MSC_VER
	sprintf_s(stackbuf, 128, "%d/%d", pos, len);

#else
	sprintf(stackbuf, "%d/%d", pos, len);
#endif
	return stackbuf;
}

std::string projectIdGlobal = "bbcyouerketang"; /*"videolab";*/
// GetVs4Url
char* ae(char* p)
{
	//if (strlen(p) > 30)
	//{
	//	throw std::logic_error("uri too long");
	//}

#ifdef _MSC_VER
	sprintf_s(stackbuf, 128, "http://192.168.0.17%s", p);
#else
	sprintf(stackbuf, "http://192.168.1.106%s", p);
	//sprintf(stackbuf, "http://192.168.56.101%s", p);
	//sprintf(stackbuf, "http://test-aliyun.viewshare.cn:80%s", p);
	//sprintf(stackbuf, "https://videolib.viewshare.cn%s", p);
	//sprintf(stackbuf, "https://videolib.viewshare.net%s", p);
	//sprintf(stackbuf, "http://videolib-test.viewshare.net%s", p);
	//sprintf(stackbuf, "https://smartbox.viewshare.net%s", p);
	//sprintf(stackbuf, "https://smartbox.viewshare.cn%s", p);
	//sprintf(stackbuf, "http://smartboxtest.viewshare.net%s", p);
	//sprintf(stackbuf, "http://kinder-demo.viewshare.cn%s", p);
	//sprintf(stackbuf, "http://teacher.viewshare.net%s", p);
	//sprintf(stackbuf, "http://teacher.tangshan.bbc.viewshare.cn%s", p);
	//sprintf(stackbuf, "https://dev-mvr.viewshare.net%s", p);
	//sprintf(stackbuf, "http://bbc-api.viewshare.net%s", p);

#endif
	return stackbuf;
}

// GetTotalDuration
double af()
{
	return pIns->GetDuration();
}

// UpdateTimeRange
int ag(float* tr, int n)
{
	return pIns->UpdateTimeRange(tr, n);
}

// IsSegInTimeRange
int ah(int i)
{
	return pIns->IsSegInTimeRange(i);
}

// GetSegIndexByTime
int ai(double t)
{
	return pIns->GetSegIndexByTime(t);
}

// SeekBySegIndex
void aj(int seg)
{
	return pIns->SeekBySegIndex(seg);
}

// GetDataRatePerSecond
int a__()
{
	return pIns->GetDataRate();
}

int GetLastNormalSeg(char*p, int l)
{
	return pIns->GetLastSeg(p, l);
}

double aK() 
{
	return pIns->GetNormalSegDuration();
}

int GetCachedFlvLen()
{
	return pIns->CachedFlvLen();
}

char* ak()
{
	int framew, frameh, sarw, sarh;
	pIns->GetFrameSize(framew, frameh);
	pIns->GetPixelAspectRatio(sarw, sarh);

	memset(stackbuf, 0, sizeof(stackbuf));
#ifdef __EMSCRIPTEN__
	sprintf(stackbuf, "%d-%d-%d-%d", framew, frameh, sarw, sarh);
#else
	sprintf_s(stackbuf, 128, "%d-%d-%d-%d", framew, frameh, sarw, sarh);
#endif
	return stackbuf;
}

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#endif

int main(int argc, char* argv[])
{
	pIns.reset(new Vs4DashConvert());

	//std::cout << "hello, word" << std::endl;
	//{
	//	char a[] = { 0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22, 0x22 };
	//	int64_t r= Char8ToLittleInt(a);
	//	printf("big number is %lld\n", r);

	//	ToFixedPointNumber1616(20.0, a);

	//	WriteInt32(257, a);
	//	a[4]++;
	//}

	//BitsPut bp(32);
	//bp.PutBits(32, 0x34567890);

	//char a[4];
	//memcpy(a, bp.GetBitsData(), 4);

	//BitsGet bg((unsigned char*)a);
	//int n = bg.GetBits(8);
	//n = bg.GetBits(1);
	//n = bg.GetBits(23);

	//FlvParseRes pr;
	//pr.flvHead.flv[0] = 0;

#ifdef _MSC_VER
	//TestReadFlv();
	//TestReadMp4();
	//TestReadMp4_dash();
	//TestDashConvert();
	//TestDashConvertGOP();
	TestVs4DashConvert();
#endif

#ifdef __EMSCRIPTEN__

	emscripten_run_script(
		"window.__vs_a = new Array();"
		"__vs_a[0]=function(){return new MediaSource;};"
		"__vs_a[1]=function(p1, p2){p1.appendBuffer(p2);};"
		"__vs_a[2]='video/mp4; codecs=\"avc1.42E01E, mp4a.40.2\"';"
		"__vs_a[3]=function(){return window.MediaSource && window.MediaSource.isTypeSupported(__vs_a[2]);};"
		"__vs_a[4]=Module.cwrap('_Z2abPci', 'number', ['number', 'number']);"
		"__vs_a[5]=Module.cwrap('_Z2acPci', 'number', ['number', 'number']);"
		"__vs_a[6]=function(p1,p2){return __vs_a[4](p1,p2);};"
		"__vs_a[7]=function(p1,p2){return __vs_a[5](p1,p2);};"
		"__vs_a[8]=Module.cwrap('_Z2aaPci', 'number', ['array', 'number']);"
		"__vs_a[9]=Module.cwrap('_Z2aaPci', 'number', ['number', 'number']);"
		"__vs_a[10]=function(e){var xhr=e.target;if(xhr.readyState == 4){if(xhr.status == 206){xhr._rd=(new Date()).getTime()-xhr._rst;var t=new Int8Array(xhr.response);xhr._cb0(0,t,xhr.response.byteLength);}else{xhr._cb0(1, xhr.status);}}};"
		);
#endif
	return 0;
}