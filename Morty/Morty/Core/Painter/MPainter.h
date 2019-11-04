/**
 * @File         MPainter
 * 
 * @Created      2019-11-04 22:49:27
 *
 * @Author       Morty
**/

#ifndef _M_MPAINTER_H_
#define _M_MPAINTER_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Vector.h"

class MORTY_CLASS MPainter : public MObject
{
public:
    MPainter();
    virtual ~MPainter();

public:


public:

	void CleanAll();

	void SetColor(const MColor& color);

	void DrawLine(const Vector3& v3Begin, const Vector3& v3End);

	void DrawRect(const Vector3& v3Center, const Vector3& v3Normal, const float& fWidth, const float& fHeight);

	void DrawCircle(const Vector3& v3Center, const Vector3& v3Up, const Vector3& v3Normal, const float& fRadius, const float& fAngle = 360.0f);

private:

};


#endif
