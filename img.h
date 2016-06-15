#pragma once


// This will be a container for bitmap images.  
// Bitmaps are stored in a device context so this will keep track of those.
// Since images have potentially a lot of data try to avoid copying them.
// Use reference counting and write on modify semantics.

class Img
{
public:
	inline Img();
	Img(const Img& img) { *this = img; }
	Img& operator=(const Img& rhs) 
	{  
		if (this != &rhs) 
			copyPrivateData(rhs); 
		return *this; 
	}
private:
	int w;
	int h;
	HBITMAP hbm;
	HDC hdcbm;

	void copyPrivateData(const Img& rhs)
	{
	}
};

Img::Img() : w(1),h(1)
{
	HDC hdc = GetDC(NULL);
	// Default constructor will create a single pixel bitmap.
	hbm = CreateBitmap(w,h,1,32,NULL);
	hdcbm = CreateCompatibleDC(hdc);
	SelectObject(hdcbm,hbm);
	ReleaseDC(NULL,hdc);
}