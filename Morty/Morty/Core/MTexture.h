/**
 * @File         MTexture
 * 
 * @Created      2019-09-11 16:12:31
 *
 * @Author       Morty
**/

#ifndef _M_MTEXTURE_H_
#define _M_MTEXTURE_H_
#include "MGlobal.h"
#include "Vector.h"

class MORTY_CLASS MTexture
{
public:
    MTexture();
    virtual ~MTexture();

public:

	void SetSize(const Vector2& v2Size);
	Vector2 GetSize() { return m_v2Size; }

	unsigned char* GetImageData(){ return m_pImageData; }

private:

	unsigned char* m_pImageData;
	Vector2 m_v2Size;

	unsigned int m_unImageDataArraySize;

};


#endif
