/**
 * @File         MCamera
 * 
 * @Created      2019-08-28 17:14:21
 *
 * @Author       Morty
**/

#ifndef _M_MCAMERA_H_
#define _M_MCAMERA_H_
#include "MGlobal.h"
#include "M3DNode.h"

class MORTY_CLASS MCamera : public M3DNode
{
public:
	M_OBJECT(MCamera);
    MCamera();
    virtual ~MCamera();

public:

	void SetZNear(const float& fZNear);
	void SetZFar(const float& fZFar);
	float GetZNear(){ return m_fZNear; }
	float GetZFar(){ return m_fZFar; }

private:

	float m_fZNear;
	float m_fZFar;

};


#endif
