/**
 * @File         MBloomWork
 * 
 * @Created      2020-12-03 10:13:09
 *
 * @Author       DoubleYe
**/

#ifndef _M_MBLOOMWORK_H_
#define _M_MBLOOMWORK_H_
#include "MGlobal.h"
#include "MStandardPostProcessWork.h"

class MMaterial;
class MORTY_API MBloomWork : public MStandardPostProcessWork
{
public:
	M_OBJECT(MBloomWork);
    MBloomWork();
    virtual ~MBloomWork();

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
