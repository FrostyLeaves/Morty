#ifndef _IMGUI_MESH_INSTANCE_H_
#define _IMGUI_MESH_INSTANCE_H_

#include "imgui.h"

#include "MMesh.h"
#include "MTexture.h"

#include <map>

class MEngine;
class MIRenderer;
class MRenderCommand;
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
	void Render(MIRenderer* pRenderer, MRenderCommand* pCommand);
	

protected:

	struct MImGuiTextureDest
	{
		int nDestroyCount;
		MITexture* pTexture;
		MShaderParamSet* pParamSet;
	};

	MImGuiTextureDest* GetTexturParamSet(MITexture* key);

private:

	MEngine* m_pEngine;

	MMesh<ImDrawVert> m_Mesh;
	MMaterial* m_pMaterial;
	MTexture m_FontTexture;

	std::map<MITexture*, MImGuiTextureDest*> m_tImGuiDrawTexture;
};







#endif