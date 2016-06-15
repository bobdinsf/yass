#pragma once
#include <utility>
#include <iostream>
using namespace std;
// A class to encapsulate a part of the complex plane mapped onto a bitmap rectangle. 
class RealRect
{
public:
	// two sets of coordinates.  
	// Windows style pixels                      complex plane (where x is real and y is imaginary)
	// 0,0            w,0                        x0,y1                  x1,y1
	//                               ====>
	// 0,h            w,h                        x0,y0                  x1,y0
	// 
	// This is the minimum set of information we need to proceed.
	// Require this class to retain the aspect ration of 1:1.  
	//    so for a given w,h we can't have arbitrary x0,y0 and x1,y1.
	// Define the area of the complex plane by using two points
	// 
	// p defines the size of pixel.   

	RealRect(int _w = 0, int _h = 0,double _x0 = 0.0,double _y0 = 0.0, double _x1 = 0.0, double _y1 = 0.0) 
		: w(_w),h(_h),x0(_x0),y0(_y0),x1(_x1),y1(_y1) 
	{
		if (x0 > x1)		swap(x0,x1);
		if (y0 > y1)		swap(y0,y1);
		setPxlWidth();
	}
	RealRect(const RealRect& r)
	{
		*this = r;
	}
	~RealRect()
	{
	}
	RealRect& operator=(const RealRect& rhs)
	{
		if (this != &rhs)
		{
			w = rhs.w;
			h = rhs.h;
			x0 = rhs.x0;
			y0 = rhs.y0;
			x1 = rhs.x1;
			y1 = rhs.y1;
			pw = rhs.pw;
		}
		return *this;
	}
	int width()  { return w; }
	int length() { return h; }
	double pxlWidth() { return pw; }
	double getX0() { return x0; }
	double getX1() { return x1; }
	double getY0() { return y0; }
	double getY1() { return y1; }
	void setX0( double v ) { x0 = v; }
	void setY0( double v ) { y0 = v; }
	void setX1( double v ) { x1 = v; }
	void setY1( double v ) { y1 = v; }
	void setSize(int width, int height)
	{
		w = width;
		h = height;
		setPxlWidth();
	}
	void zoom(int left, int right, int top, int bottom)
	{
		double _x0 = x0;
		double _y0 = y0;
		x0 = _x0 + left * pw;
		x1 = _x0 + right * pw;
		y0 = _y0 + (h - bottom) * pw;
		y1 = _y0 + (h - top) * pw;
		setPxlWidth();
	}
	void resize(int left, int right, int top, int bottom)
	{
		w = right - left;
		h = bottom - top;
		setPxlWidth();
	}
	int area() { return w*h; }
	void setPxlWidth()
	{
		if (w == 0)
		{
			if (h == 0)	pw = 0;
			else        pw = (y1 - y0) / h;
		}
		else if (h == 0)
			pw = (x1 - x0) / w;
		else 
			pw = max( (x1 - x0) / w  , (y1 - y0) / h);
	}
private:
	int w,h;
	double x0,y0,x1,y1;
	double pw;


};

bool unitTestsForRealRect();