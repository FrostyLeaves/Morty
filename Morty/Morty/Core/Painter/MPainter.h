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
#include "MType.h"
#include <vector>

class MIMesh;
class MIViewport;
class MORTY_CLASS MPaintCollage
{
public:
	MPaintCollage();
	~MPaintCollage();

	void Clean();

	//设置线的颜色
	void SetLineColor(const MColor& color) { m_lineColor = color; }
	void SetFillColor(const MColor& color) { m_fillColor = color; }
	void SetLineWidth(const float& fWidth) { m_fLineWidth = fWidth; }

	void Draw2DLine(const Vector3& v3Begin, const Vector3& v3End);

public:
	MIMesh* GetRenderMesh(MIViewport* pViewport);
// 
// 	void DrawRect(const Vector3& v3Center, const Vector3& v3Normal, const float& fWidth, const float& fHeight);
// 
// 	void DrawCircle(const Vector3& v3Center, const Vector3& v3Up, const Vector3& v3Normal, const float& fRadius, const float& fAngle = 360.0f);

protected:

	MColor m_lineColor;
	MColor m_fillColor;;
	float m_fLineWidth;
};

class MORTY_CLASS MPainter : public MObject
{
	struct MPainterVertex { Vector2 pos; Vector4 color; };
public:
    MPainter();
    virtual ~MPainter();

public:


	MIMesh* GetMesh2DLine(const Vector3& v3Begin, const Vector3& v3End, const MColor& lineColor, const float& fThickness = 1.0f);

	void DrawCollage(MPaintCollage& collage);


public:

	void SetAttachedViewport(MIViewport* pViewport);

private:

	MIViewport* m_pViewport;
};


#endif
