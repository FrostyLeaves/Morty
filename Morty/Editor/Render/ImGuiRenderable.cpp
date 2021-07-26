#include "ImGuiRenderable.h"

#include "MEngine.h"
#include "MTexture.h"
#include "MMaterial.h"
#include "MShaderParamSet.h"
#include "MRenderCommand.h"

#include "MTextureResource.h"

#include "MRenderSystem.h"
#include "MResourceSystem.h"

ImGuiRenderable::ImGuiRenderable(MEngine* pEngine)
	: m_pEngine(pEngine)
	, m_Mesh(true)
	, m_pMaterial(nullptr)
	, m_FontTexture()
	, m_tImGuiDrawTexture()
{

}

ImGuiRenderable::~ImGuiRenderable()
{

}

void ImGuiRenderable::UpdateMesh()
{
	auto draw_data = ImGui::GetDrawData();
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;

	if (draw_data->TotalVtxCount > 0)
	{
		// Create or resize the vertex/index buffers
		m_Mesh.ResizeVertices(draw_data->TotalVtxCount * 3);
		m_Mesh.ResizeIndices(draw_data->TotalIdxCount * 3, 1);

		// Upload vertex/index data into a single contiguous GPU buffer
		ImDrawVert* vtx_dst = m_Mesh.GetVertices();
		uint32_t* idx_dst = m_Mesh.GetIndices();

		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtx_dst += cmd_list->VtxBuffer.Size;
			idx_dst += cmd_list->IdxBuffer.Size;
		}

		m_Mesh.SetNeedUpload();
	}
}

void ImGuiRenderable::Initialize()
{
	InitializeFont();
	InitializeMaterial();
}

void ImGuiRenderable::InitializeFont()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();

	ImGuiIO& io = ImGui::GetIO();

	unsigned char* pixels;
	int width, height; // width height
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	size_t upload_size = width * height * 4 * sizeof(char);

	MTextureResource* pFontTexture = pResourceSystem->CreateResource<MTextureResource>("ImGUI_Font");
	pFontTexture->LoadFromMemory(pixels, width, height, 32);
	m_FontTexture.SetResource(pFontTexture);

	// Store our identifier
	io.Fonts->TexID = pFontTexture->GetTextureTemplate();

}

void ImGuiRenderable::Release()
{
	ReleaseMaterial();
	ReleaseFont();

	ReleaseMesh();
}

void ImGuiRenderable::ReleaseFont()
{
	m_FontTexture.SetResource(nullptr);
}

void ImGuiRenderable::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();

	m_pMaterial = pResourceSystem->CreateResource<MMaterial>();
	m_pMaterial->AddRef();

	m_pMaterial->LoadVertexShader("Shader/imgui.mvs");
	m_pMaterial->LoadPixelShader("Shader/imgui.mps");
}

void ImGuiRenderable::ReleaseMaterial()
{
	if (m_pMaterial)
	{
		m_pMaterial->SubRef();
		m_pMaterial = nullptr;
	}
}

void ImGuiRenderable::ReleaseMesh()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_Mesh.DestroyBuffer(pRenderSystem->GetDevice());
}

void ImGuiRenderable::Tick(const float& fDelta)
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	for (auto iter = m_tImGuiDrawTexture.begin(); iter != m_tImGuiDrawTexture.end(); )
	{
		int& count = iter->second->nDestroyCount;
		++count;

		if (count > 30)
		{
			iter->second->pParamSet->DestroyBuffer(pRenderSystem->GetDevice());
			iter = m_tImGuiDrawTexture.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	UpdateMesh();
}

void ImGuiRenderable::Render(MIRenderCommand* pCommand)
{
	auto draw_data = ImGui::GetDrawData();
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;

	pCommand->SetViewport(MViewportInfo(0.0f, fb_height, fb_width, -fb_height));
	m_pMaterial->SetMaterialType(MEMaterialType::EImGui);
	m_pMaterial->SetRasterizerType(MERasterizerType::ECullNone);
	pCommand->SetUseMaterial(m_pMaterial);

	Vector2 scale;
	scale.x = 2.0f / draw_data->DisplaySize.x;
	scale.y = 2.0f / draw_data->DisplaySize.y;
	Vector2 translate;
	translate.x = -1.0f - draw_data->DisplayPos.x * scale.x;
	translate.y = -1.0f - draw_data->DisplayPos.y * scale.y;

	if (MShaderConstantParam* pParam = m_pMaterial->GetMaterialParamSet()->m_vParams[0])
	{
		if (MStruct* pStruct = pParam->var.GetStruct())
		{
			pStruct->GetMember(0)->var = scale;
			pStruct->GetMember(1)->var = translate;
			pParam->SetDirty();
		}
	}

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render command lists
	// (Because we merged all buffers into a single one, we maintain our own offset into them)
	int global_vtx_offset = 0;
	int global_idx_offset = 0;
	void* using_texture = nullptr;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

			if (using_texture != pcmd->TextureId)
			{
				using_texture = pcmd->TextureId;
				if (auto dest = GetTexturParamSet((MTexture*)(using_texture)))
				{
					if (dest->nDestroyCount > 0) {
						--dest->nDestroyCount;
					}
					pCommand->SetShaderParamSet(dest->pParamSet);
				}
			}

			// Project scissor/clipping rectangles into framebuffer space
			ImVec4 clip_rect;
			clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
			clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
			clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
			clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

			if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
			{
				// Negative offsets are illegal for vkCmdSetScissor
				if (clip_rect.x < 0.0f)
					clip_rect.x = 0.0f;
				if (clip_rect.y < 0.0f)
					clip_rect.y = 0.0f;

				pCommand->SetScissor(MScissorInfo(clip_rect.x, clip_rect.y, clip_rect.z - clip_rect.x, clip_rect.w - clip_rect.y));

				pCommand->DrawMesh(&m_Mesh, pcmd->IdxOffset + global_idx_offset, pcmd->ElemCount, pcmd->VtxOffset + global_vtx_offset);
			}

		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}
}

ImGuiRenderable::MImGuiTextureDest* ImGuiRenderable::GetTexturParamSet(MTexture* key)
{
	auto findResult = m_tImGuiDrawTexture.find(key);

	if (findResult == m_tImGuiDrawTexture.end())
	{
		MImGuiTextureDest* pDest = new MImGuiTextureDest();

		pDest->pTexture = key;
		pDest->nDestroyCount = 0;
		pDest->pParamSet = m_pMaterial->GetMeshParamSet()->Clone();
		pDest->pParamSet->m_vTextures[0]->pTexture = key;
		pDest->pParamSet->m_vTextures[0]->SetDirty();
		
		if (MStruct* pStruct = pDest->pParamSet->m_vParams[0]->var.GetStruct())
		{
			switch (key->GetTextureLayout())
			{
			case METextureLayout::EDepth:
			case METextureLayout::ER32:
				pStruct->GetMember(0)->var = 1;
				break;

			default:
				pStruct->GetMember(0)->var = 0;
				break;
			}
			pDest->pParamSet->m_vParams[0]->SetDirty();
		}


		m_tImGuiDrawTexture[key] = pDest;
		return pDest;
	}

	return findResult->second;
}
