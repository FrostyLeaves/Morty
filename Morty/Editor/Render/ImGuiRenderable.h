#ifndef _IMGUI_MESH_INSTANCE_H_
#define _IMGUI_MESH_INSTANCE_H_

#include "imgui.h"

#include "MMesh.h"
#include "MResource.h"

#include <map>

class MEngine;
class MTexture;
class MIRenderer;
class MIRenderCommand;
class MShaderParamSet;
class ImGuiRenderable
{
public:
	ImGuiRenderable(MEngine* pEngine);
	virtual ~ImGuiRenderable();


	void UpdateMesh();

public:

	void Initialize();
	void Release();

	void InitializeFont();
	void ReleaseFont();

	void InitializeMaterial();
	void ReleaseMaterial();

	void ReleaseMesh();


	void Tick(const float& fDelta);

	void WaitTextureReady(MIRenderCommand* pCommand);
	void Render(MIRenderCommand* pCommand);
	

protected:

	struct MImGuiTextureDest
	{
		int nDestroyCount;
		MTexture* pTexture;
		MShaderParamSet* pParamSet;
	};

	MImGuiTextureDest* GetTexturParamSet(MTexture* key);

private:

	MEngine* m_pEngine;

	MMesh<ImDrawVert> m_Mesh;
	std::shared_ptr<MMaterial> m_pMaterial;
	MResourceKeeper m_FontTexture;

	std::map<MTexture*, MImGuiTextureDest*> m_tImGuiDrawTexture;
};







#endif