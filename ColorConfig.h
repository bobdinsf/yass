#pragma once
using namespace std;
struct ColorPoint
{
	ColorPoint(long v = 0, COLORREF c = RGB(0, 0, 0)) :
		value(v), color(c) {}
	long value;
	COLORREF color;
	BYTE r() { return GetRValue(color); }
	BYTE g() { return GetGValue(color); }
	BYTE b() { return GetBValue(color); }
};
struct ColorConfig
{
	ColorConfig(long n = 256) : nColors(n) {}
	long nColors;
	vector<ColorPoint> colorPoints;

};