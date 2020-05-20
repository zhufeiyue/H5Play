#pragma once

#define DOPRINTFLOGZZZZ
void DoPrintLog(const char* format, ...);

#ifdef DOPRINTFLOGZZZZ
#define TRACE0(s) DoPrintLog(s);
#define TRACE1(s,p1) DoPrintLog(s,p1);
#define TRACE2(s,p1,p2) DoPrintLog(s,p1,p2);
#define TRACE3(s,p1,p2,p3) DoPrintLog(s,p1,p2,p3);
#define TRACE4(s,p1,p2,p3,p4) DoPrintLog(s,p1,p2,p3,p4);

#else
#define TRACE0
#define TRACE1
#define TRACE2
#define TRACE3
#endif

