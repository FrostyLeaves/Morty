/**
 * @File         MILight
 * 
 * @Created      2019-09-19 12:35:21
 *
 * @Author       Morty
**/

#ifndef _M_MILIGHT_H_
#define _M_MILIGHT_H_
#include "MGlobal.h"
#include "M3DNode.h"
#include "Vector.h"

class MORTY_CLASS MILight : public M3DNode
{
public:
    MILight();
    virtual ~MILight();

public:

	virtual void SetLightColor(const Vector3& color) = 0;
	virtual Vector3 GetLightColor() = 0;

private:

};


#endif
