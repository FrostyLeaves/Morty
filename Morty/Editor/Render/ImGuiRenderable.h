#ifndef _IMGUI_MESH_INSTANCE_H_
#define _IMGUI_MESH_INSTANCE_H_

#include "imgui.h"

#include "Render/MMesh.h"
#include "Resource/MResource.h"

#include <map>

class MEngine;
class MTexture;
class MIRenderer;
class MIRenderCommand;
class MShaderPropertyBlock;
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
		std::shared_ptr<MTexture> pTexture;
		std::shared_ptr<MShaderPropertyBlock> pParamSet;
	};

	MImGuiTextureDest* GetTexturParamSet(ImGuiTexture tex);

private:

	MEngine* m_pEngine;

	MMesh<ImDrawVert> m_Mesh;
	std::shared_ptr<MMaterial> m_pMaterial;
	MResourceRef m_FontTexture;

	std::map<ImGuiTexture, MImGuiTextureDest*> m_tImGuiDrawTexture;
};







#endif