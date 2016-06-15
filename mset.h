#pragma once
#include <complex>
#include <algorithm>
#include <list>
#include "RealRect.h"

using namespace std;
typedef std::pair<double,double> pt;
//template <class t>
class Cmset
{
public:
	Cmset() : m_rect(100, 100, -2.0, -2.0, 2.0, 2.0),m_colorMap(0) { init(); }
	Cmset(double x0, double y0, double x1, double y1, int width, int height)
		: m_rect(width, height, x0, y0, x1, y1), m_colorMap(0) { init(); }

	Cmset(const Cmset& src);
	Cmset& operator=(const Cmset& rhs);
	~Cmset();
	void resize(int width, int length);
	HDC GetBitmap(HDC& hdc);
	int GetWidth()  { return m_rect.width(); }
	int GetLength() { return m_rect.length(); }
	double GetX0()  { return m_rect.getX0(); }
	double GetY0()  { return m_rect.getY0(); }
	double GetX1()  { return m_rect.getX1(); }
	double GetY1()  { return m_rect.getY1(); }
	void zoom(RECT zrect);
	void SetThreshold(long n);
	void set(double x0, double y0, double x1, double y1);
	void DoubleThreshold();
	void HalfThreshold();
	void prev();
	void next();
private:
	// Parameters for the area of the complex plane to be analyzed.
	RealRect m_rect;
	list<RealRect> m_history;
	list<RealRect>::iterator m_histIndex;
//	complex<double> ll;        // lower left corner.
//	complex<double> ur;        // upper right corner.
//	int w,l;           // vertical and horizontal resolution
	long m_threshold;        // how many itteration to perform before deciding that point is in the set.
	long* m_counts;
	COLORREF* m_colorMap;
	HBITMAP m_hbmmz;
	static const int maxIntensity;

	void init();
	void copyPrivateData(const Cmset& rhs);
	void compute(long newThreshold = -1);
	void setColorMap(); 
};