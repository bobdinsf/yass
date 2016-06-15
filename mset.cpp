#include "stdafx.h"
const int Cmset::maxIntensity = 100;  // % of color intensity to use. 
void Cmset::init()	
{
	m_counts = new long[m_rect.area()];
	compute();
	m_threshold = 100;
	setColorMap();
	m_history.empty();
	m_histIndex = m_history.begin();
}
Cmset::Cmset(const Cmset& src)
{
	*this = src;
}
Cmset& Cmset::operator=(const Cmset& rhs)
{
	if (this != &rhs)
	{
		copyPrivateData(rhs);
	}
	return *this;
}
Cmset::~Cmset()
{
	delete [] m_counts;
	delete [] m_colorMap;
}
void Cmset::resize(int width, int length)
{
	m_rect.setSize(width,length);
	delete [] m_counts;
	m_counts = new long[m_rect.area()];
	compute();
}
HDC Cmset::GetBitmap(HDC& hdc)
{
	int w = m_rect.width();
	int l = m_rect.length();
	m_hbmmz = CreateBitmap(w,l,1,32,NULL);
	HDC hdcmz = CreateCompatibleDC(hdc);
	HGDIOBJ retVal = SelectObject(hdcmz, m_hbmmz);
	for (int j = l-1; j >= 0; --j)
	{
		for (int i = 0; i < w; ++i)
		{
			SetPixel(hdcmz,i,j, m_colorMap[m_counts[i*l + j]]);
		}
	}
	return hdcmz;
}
void Cmset::zoom(RECT zrect)
{
	m_history.push_back(m_rect);
	m_rect.zoom(zrect.left,zrect.right,zrect.top,zrect.bottom);
	compute();
}
void Cmset::SetThreshold(long n)
{
	compute(n);
	m_threshold = n;
	setColorMap();
}
void Cmset::set(double x0, double y0, double x1, double y1)
{
	m_history.push_back(m_rect);
	m_rect.setX0(x0);
	m_rect.setY0(y0);
	m_rect.setX1(x1);
	m_rect.setY1(y1);
	m_rect.setPxlWidth();
	compute();

}
void Cmset::DoubleThreshold()
{
	// Neet to check for overflow
	SetThreshold(m_threshold *= 2);
	compute();
}
void Cmset::HalfThreshold()
{
	long t = m_threshold /= 2;
	if (t < 10) 
		t = 10;
	SetThreshold(t);
	compute();
}
void Cmset::prev()
{
	if (m_history.empty())
		return;
	if (m_histIndex != m_history.begin())
	{
		--m_histIndex;
	}
	m_rect = *m_histIndex;
	compute();
}
void Cmset::next()
{
	if (m_history.empty())
		return;
	if (++m_histIndex == m_history.end())
	{
		--m_histIndex;
	}
	m_rect = *m_histIndex;
	compute();
}
void Cmset::copyPrivateData(const Cmset& rhs)
{
	m_rect = rhs.m_rect;
	m_threshold = rhs.m_threshold;
	m_counts = new long[m_rect.area()];
	memcpy(m_counts,rhs.m_counts,m_rect.area()*sizeof(long));
	m_history = rhs.m_history;
}



void Cmset::compute(long newThreshold)
{
	bool bNewThresh = (newThreshold > 0);
	if (!bNewThresh)
		newThreshold = m_threshold;
	double step = m_rect.pxlWidth();
	int i,j;
	double nReal;
	double nImag;
	for (j = m_rect.length()-1, nImag = m_rect.getY0();
			j >= 0;
			--j, nImag += step)
	{
		for (i = 0, nReal = m_rect.getX0();
				i < m_rect.width();
				++i, nReal += step)
		{
			int countsIndex = i*m_rect.length()+j;
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
					
			} while ( a*a + b*b < 4 && ++n < newThreshold );
			if (n == newThreshold)
			{
				a = 0;
			}
			m_counts[countsIndex] = n;
		}
	}
}
void Cmset::setColorMap()
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