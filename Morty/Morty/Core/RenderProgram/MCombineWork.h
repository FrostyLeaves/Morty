/**
 * @File         MCombineWork
 * 
 * @Created      2020-11-30 17:58:42
 *
 * @Author       Pobrecito
**/

#ifndef _M_MCOMBINEWORK_H_
#define _M_MCOMBINEWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MStandardPostProcessWork.h"

#include <array>

class MMaterial;
class MORTY_CLASS MCombineWork : public MStandardPostProcessWork
{
public:
	M_OBJECT(MCombineWork);
	MCombineWork();
	virtual ~MCombineWork();

public:

	virtual void Initialize(MIRenderProgram* pRenderProgram) override;
	virtual void Release() override;

	virtual void Render(MPostProcessRenderInfo& info) override;


protected:

	void InitializeMaterial();
	void ReleaseMaterial();

protected:

	MMaterial* m_pMaterial;

};
#endif
