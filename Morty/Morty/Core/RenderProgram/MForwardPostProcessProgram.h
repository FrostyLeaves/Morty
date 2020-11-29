/**
 * @File         MForwardPostProcessProgram
 * 
 * @Created      2020-11-29 15:32:27
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFORWARDPOSTPROCESSPROGRAM_H_
#define _M_MFORWARDPOSTPROCESSPROGRAM_H_
#include "MGlobal.h"
#include "MForwardRenderProgram.h"

class MForwardHDRWork;
class MORTY_CLASS MForwardPostProcessProgram : public MForwardRenderProgram
{
public:
	M_OBJECT(MForwardPostProcessProgram);
    MForwardPostProcessProgram();
    virtual ~MForwardPostProcessProgram();

public:
	virtual void Render(MIRenderer* pRenderer, const std::vector<MViewport*>& vViewports) override;

protected:

	virtual void Initialize() override;
	virtual void Release() override;

protected:

	MForwardHDRWork* m_pRenderHDRWork;
};

#endif
