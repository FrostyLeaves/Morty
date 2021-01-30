/**
 * @File         MBasicRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MBasicRenderProgram_H_
#define _M_MBasicRenderProgram_H_
#include "MGlobal.h"
#include "MMesh.h"
#include "Vector.h"
#include "MIRenderProgram.h"

#include <vector>

class MMaterial;
class MORTY_API MBasicRenderProgram : public MIRenderProgram
{
public:


public:
	M_OBJECT(MBasicRenderProgram);
	MBasicRenderProgram();
    virtual ~MBasicRenderProgram();

public:

    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport) override;

	void DrawMeshInstance(MIRenderer* pRenderer, MIMeshInstance* pMeshInstance) ;
public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

private:


    MMaterial* m_pMaterial;
	MMesh<Vector2> m_TransparentDrawMesh;

};

#endif
