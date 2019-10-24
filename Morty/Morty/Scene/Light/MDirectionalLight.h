/**
 * @File         MDirectionalLight
 * 
 * @Created      2019-09-19 17:45:15
 *
 * @Author       Morty
**/

#ifndef _M_MDIRECTIONALLIGHT_H_
#define _M_MDIRECTIONALLIGHT_H_
#include "MGlobal.h"
#include "MILight.h"

class MORTY_CLASS MDirectionalLight : public MILight
{
public:
	M_OBJECT(MDirectionalLight);
    MDirectionalLight();
    virtual ~MDirectionalLight();

public:

	void SetDirection(const Vector3& v3Direction);
	Vector3 GetDirection() { return m_v3Direction; }

	void SetColor(const Vector3& v3Color);
	Vector3 GetColor(){ return m_v3Color; }

private:

	Vector3 m_v3Direction;
	Vector3 m_v3Color;

};


#endif
