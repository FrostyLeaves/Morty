/**
 * @File         MRenderTaskNodeOutput
 * 
 * @Created      2021-07-19 09:56:11
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERTASKNODEOUTPUT_H_
#define _M_MRENDERTASKNODEOUTPUT_H_
#include "MGlobal.h"
#include "MColor.h"
#include "MTaskNodeOutput.h"

class MTexture;
class MORTY_API MRenderTaskNodeOutput : public MTaskNodeOutput
{
    MORTY_CLASS(MRenderTaskNodeOutput)
public:
    MRenderTaskNodeOutput();
    virtual ~MRenderTaskNodeOutput();

public:

	void SetClear(const bool& b) { m_bClear = b; }
	bool GetClear() const { return m_bClear; }

	void SetClearColor(const MColor& color) { m_clearColor = color; }
	MColor GetClearColor() const { return m_clearColor; }

	void SetTexture(MTexture* pTexture) { m_pTexture = pTexture; }
	MTexture* GetTexture() { return m_pTexture; }

private:

	bool m_bClear;
	MColor m_clearColor;

	MTexture* m_pTexture;
};


#endif
