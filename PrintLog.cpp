
#include "PrintLog.h"
#include <cstdio>
#include <cstdarg>

void DoPrintLog(const char* pFormat, ...)
{
	const int buflen(1024);
	char buf[buflen] = { 0 };
	va_list ap;

	if (!pFormat)
		return;

	va_start(ap, pFormat);
#ifdef _MSC_VER 
	vsprintf_s(buf, buflen, pFormat, ap);
#else
	vsprintf(buf, pFormat, ap);
#endif
	va_end(ap);

	printf("%s\n", buf);
	//fprintf(stdout, "%s\n", buf);
}
