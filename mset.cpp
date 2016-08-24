#include "stdafx.h"
#include "yass.h"
const int CMSet::maxIntensity = 100;  // % of color intensity to use. 
const int CMSet::nInitWidth = 1000;
const int CMSet::nInitLength = 1000;
const int CMSet::nInitialThreshold = 1000;
void CMSet::init()	
{
	m_nComputeThreads = 0;
	reallocTheCounts();
	dprintf(L"m_counts allocated for %d pixels at %x", m_rect.area(), m_counts);
	m_threshold = nInitialThreshold;
	setColorMap();
	m_history.empty();
	m_histIndex = m_history.begin();
	bRefreshTimerRunning = false;
}

void CMSet::SetWindowHandle(HWND hWnd)
{
	m_hWnd = hWnd;
}
CMSet::CMSet(const CMSet& src)
{
	*this = src;
}
CMSet& CMSet::operator=(const CMSet& rhs)
{
	if (this != &rhs)
	{
		copyPrivateData(rhs);
	}
	return *this;
}
CMSet::~CMSet()
{
	delete [] m_counts;
	delete [] m_colorMap;
}
void CMSet::resize(int width, int length)
{
	if (m_rect.width() == width && m_rect.length() == length)
		return;  // Mset size if OK.
	m_rect.setSize(width,length);
	delete [] m_counts;
	reallocTheCounts();
	dprintf(L"m_counts allocated for %d pixels at %x", m_rect.area(), m_counts);
}
HDC CMSet::GetBitmap(HDC& hdc)
{
	int w = m_rect.width();
	int l = m_rect.length();
	m_hbmmz = CreateBitmap(w,l,1,32,NULL);
	HDC hdcmz = CreateCompatibleDC(hdc);
	HGDIOBJ retVal = SelectObject(hdcmz, m_hbmmz);
	for (int j = 0; j < l; ++j)  // Outer loop is rows
	{
		for (int i = 0; i < w; ++i) // Inner loop is columns
		{
			long lVal = m_counts[j*w + i];
			int jBitMap = l - j - 1;
			SetPixel(hdcmz,
				     i, jBitMap,
				     m_colorMap[lVal]);
		}
	}
	return hdcmz;
}
void CMSet::zoom(RECT zrect)
{
	m_history.push_back(m_rect);
	m_rect.zoom(zrect.left,zrect.right,zrect.top,zrect.bottom);
}
void CMSet::SetThreshold(long n)
{
	m_threshold = n;
	setColorMap();
}
void CMSet::set(double x0, double y0, double x1, double y1)
{
	m_history.push_back(m_rect);
	m_rect.setX0(x0);
	m_rect.setY0(y0);
	m_rect.setX1(x1);
	m_rect.setY1(y1);
	m_rect.setPxlWidth();
}
void CMSet::DoubleThreshold()
{
	// Neet to check for overflow
	SetThreshold(m_threshold *= 2);
}
void CMSet::HalfThreshold()
{
	long t = m_threshold /= 2;
	if (t < 10) 
		t = 10;
	SetThreshold(t);
}
void CMSet::prev()
{
	if (m_history.empty())
		return;
	if (m_histIndex != m_history.begin())
	{
		--m_histIndex;
	}
	m_rect = *m_histIndex;
}
void CMSet::next()
{
	if (m_history.empty())
		return;
	if (++m_histIndex == m_history.end())
	{
		--m_histIndex;
	}
	m_rect = *m_histIndex;
}
void CMSet::copyPrivateData(const CMSet& rhs)
{
	m_rect = rhs.m_rect;
	m_threshold = rhs.m_threshold;
	reallocTheCounts();
	memcpy(m_counts,rhs.m_counts,m_rect.area()*sizeof(long));
	m_history = rhs.m_history;
}
static struct WorkerData
{
	WorkerData() : r(), bNewThresh(false), pMSet(0), x(0), y(0), w(0), l(0) {}
	WorkerData(RealRect rect, bool b = false, CMSet* p = 0) : r(rect), bNewThresh(b), pMSet(p) {}
	RealRect r;
	bool bNewThresh;
	CMSet* pMSet;
	int x;
	int y;
	int w;
	int l;
} wData[16];
static bool arrCompDone[16];
static DWORD arrThreadID[16];
static HANDLE arrThreadHandle[16];

DWORD WINAPI TFunc(LPVOID lpParam)
{
	WorkerData* pParams = (WorkerData*)lpParam;
	pParams->pMSet->computeWorker(pParams->r, pParams->bNewThresh,
		pParams->x,pParams->y,pParams->w,pParams->l);
	return 0;
}
void CMSet::compute(long newThreshold)
{
	dprintf(L"Compute()");
	bool bNewThresh = (newThreshold > 0);
	if (!bNewThresh)
		newThreshold = m_threshold;
	m_step = m_rect.pxlWidth();
	
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			//dprintf(L"n threads = %d", m_nComputeThreads);
			int iWdata = i * 4 + j;
			wData[iWdata].r = m_rect;
			wData[iWdata].bNewThresh = bNewThresh;
			wData[iWdata].pMSet = this;
			wData[iWdata].x = (i * m_rect.width()) / 4;
			wData[iWdata].y = (j * m_rect.length()) / 4;
			wData[iWdata].w = m_rect.width() / 4;
			wData[iWdata].l = m_rect.length() / 4;
			++m_nComputeThreads;
			arrThreadHandle[iWdata] =
				CreateThread(
					NULL,
					0,
					TFunc,
					&wData[iWdata],
					0,
					&arrThreadID[iWdata]);
		}
	}
	dprintf(L"Compute threads running. Start a timer to refresh screen");
	
	
	//WaitForMultipleObjects(16, arrThreadHandle, TRUE, INFINITE);

}
inline long CMSet::value(double& nReal, double& nImag)
{
	//complex<double> c(nReal,nImag);
	//complex<double> z(0,0);
	double a, b, c, d, a_plus_b, nexta;
	a = 0;
	b = 0;
	c = nReal;
	d = nImag;
	long n = 0;
	do
	{
		//We need to compute:  z = z*z + c;
		//
		// Let z = a + bi
		// and c = c + di
		// so we need    (a + bi)(a + bi) + (c + di)
		//         or    a^2 + 2abi - b^2 + c + di
		//         or    a^2 - b^2 + c + (2ab + d)i
		//         or    (a + b)(a - b) + c + (2ab + d)i
		//
		__asm
		{
			fld QWORD PTR a
			fadd QWORD PTR b              // a + b
			fstp QWORD PTR a_plus_b
			fld QWORD PTR a
			fsub QWORD PTR b              // a - b
			fmul QWORD PTR a_plus_b       // (a - b)(a + b)
			fadd QWORD PTR c              // (a - b)(a + b) + c
			fstp QWORD PTR nexta
			fld1
			fld1
			fadd                          // 2
			fmul QWORD PTR a              // 2a
			fmul QWORD PTR b              // 2ab
			fadd QWORD PTR d              // 2ab + d
			fstp QWORD PTR b
		}
		a = nexta;

	} while (a*a + b*b < 4 && ++n < m_threshold);

	return n;
}
void CMSet::computeWorker(RealRect r, bool bNewThresh,
	int x, int y, int w, int l)
{
	double fReal;
	double fImag;
	double y0 = r.getY0();
	double x0 = r.getX0();
	int ix, iy;
	int stopX = x + w;
	int stopY = y + l;
	for (fImag = y0 + y*m_step, iy = y; iy < stopY; ++iy, fImag += m_step)
	{
		for (fReal = x0 + x*m_step, ix = x; ix < stopX; ++ix, fReal += m_step)
		{
			long n = value(fReal, fImag);
			if (n == m_threshold)
			{
				int stop = 0;
			}
			size_t index = iy*m_rect.width() + ix;
			m_counts[index] = n;
		}

	}
	//dprintf(L"compute finished post WM_USER+1 message");
	decrementNCompThreads();
}

#if 0
void /*CMSet::computeWorker*/_f(RealRect r, bool bNewThresh,
	int x, int y, int w, int l)
{
	dprintf(L"computeWorker([%d,%d],%d)", r.width(), r.length(), bNewThresh );
	dprintf(L"m_counts = %x", m_counts);

	int i,j;
	double nReal;
	double nImag;
	for (j = y+l, nImag = r.getY0()+((y+l)*m_step);
			j >= y;
			--j, nImag += m_step)
	{
		for (i = x, nReal = r.getX0()+(x*m_step);
				i < x+w;
				++i, nReal += m_step)
		{
			//dprintf(L"%d, %d", i, j);
			int countsIndex = i*r.length()+j;
			if (bNewThresh && m_counts[countsIndex] < m_threshold)
				continue;
			//complex<double> c(nReal,nImag);
			//complex<double> z(0,0);
			double a,b,c,d,a_plus_b,nexta;
			a = 0;
			b = 0;
			c = nReal;
			d = nImag;
			int n = 0;
			do
			{
				//We need to compute:  z = z*z + c;
				//
				// Let z = a + bi
				// and c = c + di
				// so we need    (a + bi)(a + bi) + (c + di)
				//         or    a^2 + 2abi - b^2 + c + di
				//         or    a^2 - b^2 + c + (2ab + d)i
				//         or    (a + b)(a - b) + c + (2ab + d)i
				//
				__asm
				{
					fld QWORD PTR a
					fadd QWORD PTR b              // a + b
					fstp QWORD PTR a_plus_b 
					fld QWORD PTR a  
					fsub QWORD PTR b              // a - b
					fmul QWORD PTR a_plus_b       // (a - b)(a + b)
					fadd QWORD PTR c              // (a - b)(a + b) + c
					fstp QWORD PTR nexta
					fld1                          
					fld1                          
					fadd                          // 2
					fmul QWORD PTR a              // 2a
					fmul QWORD PTR b              // 2ab
					fadd QWORD PTR d              // 2ab + d
					fstp QWORD PTR b                    
				}
				a = nexta;
					
			} while ( a*a + b*b < 4 && ++n < m_threshold);
			if (n == m_threshold)
			{
				a = 0;
			}
			dprintf(L"m_counts[%d, %d]=%d %s", i, j, n, n==m_threshold?"<---------":"");
			m_counts[countsIndex] = n;
		}
	}
}
#endif

inline BYTE interpolate(int x, int x1, BYTE y1, int x2, BYTE y2)
{
	double m(double(y2 - y1) / double(x2 - x1));
	double b = y1 - m*x1;
	return (BYTE)(unsigned(m*x + b) & 0xFF);
}

void CMSet::setColorMap()
{
	int v = this->m_threshold;
	int nStripyness = 0;
	vector<ColorPoint> colorConfig;
	colorConfig.push_back(ColorPoint(    0, RGB(32 , 64,   128 )));
//	colorConfig.push_back(ColorPoint(v / 2, RGB(0,   128, 255)));
#if 0
	colorConfig.push_back(ColorPoint(6 * v / 8, RGB(0, 127, 64)));
	colorConfig.push_back(ColorPoint(3 * v / 8, RGB(0, 255, 0)));
	colorConfig.push_back(ColorPoint(4 * v / 8, RGB(255, 255, 255)));

	colorConfig.push_back(ColorPoint(5 * v / 8, RGB(255, 0, 255)));
	colorConfig.push_back(ColorPoint(6 * v / 8, RGB(64, 64, 255)));
	colorConfig.push_back(ColorPoint(7 * v / 8, RGB(77, 0166, 44)));
#endif
	colorConfig.push_back(ColorPoint(v - 1, RGB(255, 255,  255)));

	if (colorConfig.size() < 2)
	{
		MessageBox(NULL, L"Not enough colors in config", L"Error", MB_ICONERROR);
	}

	delete[] m_colorMap;
	m_colorMap = new COLORREF[m_threshold + 1];

	size_t iConfigPoint = 0;
	ColorPoint* thisPt = &colorConfig[iConfigPoint];
	ColorPoint* nextPt = &colorConfig[iConfigPoint + 1];

	for (int i = 0; i < m_threshold; ++i)
	{
#pragma warning(push)
#pragma warning(disable : 4244)
		m_colorMap[i] = RGB(
			interpolate(i%v, thisPt->r(), thisPt->value, nextPt->r(), nextPt->value),
			interpolate(i%v, thisPt->g(), thisPt->value, nextPt->g(), nextPt->value),
			interpolate(i%v, thisPt->b(), thisPt->value, nextPt->b(), nextPt->value) );
		m_colorMap[++i] = RGB(
			nStripyness + interpolate(i%v, thisPt->r(), thisPt->value, nextPt->r(), nextPt->value),
			nStripyness + interpolate(i%v, thisPt->g(), thisPt->value, nextPt->g(), nextPt->value),
			nStripyness + interpolate(i%v, thisPt->b(), thisPt->value, nextPt->b(), nextPt->value));
#pragma warning(pop)
		if (i >= nextPt->value)
		{
			if (iConfigPoint < (colorConfig.size() - 2))
			{
				// We are at a new point
				++iConfigPoint;
				thisPt = &colorConfig[iConfigPoint];
				nextPt = &colorConfig[iConfigPoint + 1];
			}
			else
			{
				iConfigPoint = 0;
				thisPt = &colorConfig[iConfigPoint];
				nextPt = &colorConfig[iConfigPoint + 1];
			}
		}
	}
	// Fill in special color for value at the Threshold.
	m_colorMap[m_threshold] = RGB(255,255,0);
}
#if 0
void CMSet::setColorMap()
{
	int r1,g1,b1;
	int r2,g2,b2;
	int r = 0;
	int g = 0;
	int b = 0;
	delete [] m_colorMap;
	m_colorMap = new COLORREF[m_threshold +1];
	for (int i = 0; i < m_threshold; ++i)
	{
		int t = i;// +1;
		r = (t & 0x7) << 5;
		b = ((t >> 3) & 0x7) << 5;
		g = ((t >> 6) & 0x7) << 5;

		r1 = r * maxIntensity / 100;
		r2 = r1 + (100 - maxIntensity) * 255 / 100;
		b1 = b * maxIntensity / 100;
		b2 = b1 + (100 - maxIntensity) * 255 / 100;
		g1 = g * maxIntensity / 100;
		g2 = g1 + (100 - maxIntensity) * 255 / 100;
		//WCHAR s[1000];
		//wsprintf(s,L"%6x, %6x, %6x\n%6x, %6x, %6x\n%6x, %6x, %6x",r,g,b,r1,g1,b1,r2,g2,b2);
		//MessageBox(NULL,s,L"debug",MB_OK);
		m_colorMap[i] = RGB(b2,g2,r2);
	}
	int dark = 256 - 256 * maxIntensity / 100;
	int light = 256 - dark;
	//m_colorMap[m_threshold] = RGB(dark, dark, dark);
	m_colorMap[m_threshold] = RGB(light, light, light);
}
#endif
void CMSet::reallocTheCounts()
{
	int nPixels = m_rect.area();
	m_counts = new long[nPixels];
	memset(m_counts, 0, nPixels * sizeof(long));
}