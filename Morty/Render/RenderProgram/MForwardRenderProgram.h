/**
 * @File         MForwardRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARDRENDERPROGRAM_H_
#define _M_MFORWARDRENDERPROGRAM_H_
#include "MGlobal.h"

#include "MType.h"
#include "MMesh.h"
#include "MBounds.h"
#include "MTaskNode.h"
#include "MTaskGraph.h"
#include "MRenderPass.h"

class MViewport;
class MMaterial;
class MRenderCommand;
class MRenderableMeshComponent;
class MORTY_API MForwardRenderProgram : public MTaskNode
{
public:
	MORTY_CLASS(MForwardRenderProgram);

	MForwardRenderProgram();
    virtual ~MForwardRenderProgram();

public:

	void Run(MTaskNode* pTaskNode);

protected:

	void Initialize();
	void Release();



	void GenerateRenderGroup(MViewport* pViewport);

protected:

	MBoundsAABB cMeshRenderAABB;
	std::map<MMaterial*, std::vector<MIMesh*>> m_tMaterialGroup;
};

#endif
