#pragma once

#include "resource.h"
#include "mset.h"
inline void dprintf(const wchar_t* format, ...)
{
	wchar_t bfr[1024];
	va_list args;
	va_start(args, format);
	wvsprintf(bfr, format, args);
	lstrcatW(bfr, L"\n");
	OutputDebugString(bfr);
}
