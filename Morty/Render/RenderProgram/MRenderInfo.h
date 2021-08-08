#ifndef M_RENDER_INFO_H_
#define M_RENDER_INFO_H_

#include "MGlobal.h"
#include "Vector.h"
#include "MBounds.h"
#include "MComponent.h"
#include "MShaderParamSet.h"

class MIMesh;
class MMaterial;
class MRenderableMeshComponent;
struct MRenderInfo
{
	MRenderInfo();

	// basic
	float fDelta;
	float fGameTime;
	class MViewport* pViewport;


	// component
	MEntity* pCameraEntity;
	MEntity* pDirectionalLightEntity;


	// bounds
	MBoundsAABB cShadowRenderAABB;
	MBoundsAABB cMeshRenderAABB;
	
	
	// shadow
	class MTexture* pShadowMapTexture;
	Matrix4 m4DirLightInvProj;


	// mesh
	std::map<MMaterial*, std::vector<MRenderableMeshComponent*>> m_tMaterialGroupMesh;

};



#endif