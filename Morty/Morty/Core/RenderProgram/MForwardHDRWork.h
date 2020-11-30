/**
 * @File         MForwardHDRWork
 * 
 * @Created      2020-11-29 11:05:51
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFORWARDHDRWORK_H_
#define _M_MFORWARDHDRWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MStandardPostProcessWork.h"

class MMaterial;
class MORTY_CLASS MForwardHDRWork : public MStandardPostProcessWork
{
public:
	M_OBJECT(MForwardHDRWork);
    MForwardHDRWork();
    virtual ~MForwardHDRWork();

public:

    virtual void Render(MPostProcessRenderInfo& info) override;


	virtual void Initialize(MIRenderProgram* pRenderProgram) override;
	virtual void Release() override;

	void InitializeMaterial();
	void ReleaseMaterial();
private:

	MMaterial* m_pHDRMaterial;
};

#endif
