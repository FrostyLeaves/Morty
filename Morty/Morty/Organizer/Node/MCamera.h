﻿/**
 * @File         MCamera
 * 
 * @Created      2019-08-28 17:14:21
 *
 * @Author       DoubleYe
**/

#ifndef _M_MCAMERA_H_
#define _M_MCAMERA_H_
#include "MGlobal.h"
#include "M3DNode.h"

class MBoundsAABB;
class MORTY_API MCamera : public M3DNode
{
public:
	enum class MECameraType{
		EPerspective = 1,
		EOrthographic = 2,
	};
public:
	M_OBJECT(MCamera);
    MCamera();
    virtual ~MCamera();
	
public:

	void SetCameraType(const MECameraType& eType) { m_eCameraType = eType; }
	MECameraType GetCameraType() const { return m_eCameraType; }

	void SetFov(const float& fFov);
	void SetZNear(const float& fZNear);
	void SetZFar(const float& fZFar);
	float GetFov() { return m_fFov; }
	float GetZNear(){ return m_fZNear; }
	float GetZFar(){ return m_fZFar; }

	void SetZNearFar(const Vector2& fZNearFar) { SetZNear(fZNearFar.x); SetZFar(fZNearFar.y); }
	Vector2 GetZNearFar() { return Vector2(GetZNear(), GetZFar()); }

	//Orthographic
	void SetWidth(const float& fWidth) { m_fWidth = fWidth; }
	float GetWidth() { return m_fWidth; }
	void SetHeight(const float& fHeight) { m_fHeight = fHeight; }
	float GetHeight() { return m_fHeight; }


public:
	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(MStruct& srt) override;

private:

	MECameraType m_eCameraType;

	float m_fFov;
	float m_fZNear;
	float m_fZFar;

	float m_fWidth;
	float m_fHeight;


};


#endif
