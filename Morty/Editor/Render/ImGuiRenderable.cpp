#include "ImGuiRenderable.h"

#include "Basic/MTexture.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "RHI/MRenderCommand.h"
#include "Shader/MShaderPropertyBlock.h"

#include "Resource/MTextureResource.h"
#include "Resource/MTextureResourceUtil.h"

#include "Utility/MGlobal.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

using namespace morty;

ImGuiRenderable::ImGuiRenderable(MEngine* pEngine)
    : m_engine(pEngine)
    , m_Mesh(true)
    , m_material(nullptr)
    , m_FontTexture()
    , m_imGuiDrawTexture()
{}

ImGuiRenderable::~ImGuiRenderable() {}

void ImGuiRenderable::UpdateMesh()
{
    auto draw_data = ImGui::GetDrawData();
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int  fb_width  = (int) (draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int  fb_height = (int) (draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;

    if (draw_data->TotalVtxCount > 0)
    {
        // Create or resize the vertex/index buffers
        m_Mesh.ResizeVertices(draw_data->TotalVtxCount * 3);
        m_Mesh.ResizeIndices(draw_data->TotalIdxCount * 3, 1);

        // Upload vertex/index data into a single contiguous GPU buffer
        ImDrawVert* vtx_dst = m_Mesh.GetVertices();
        uint32_t*   idx_dst = m_Mesh.GetIndices();

        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }

        m_Mesh.SetDirty();
    }
}

void ImGuiRenderable::Initialize()
{
    InitializeFont();
    InitializeMaterial();
}

void ImGuiRenderable::InitializeFont()
{
    MResourceSystem* pResourceSystem = m_engine->FindSystem<MResourceSystem>();

    ImGuiIO&         io = ImGui::GetIO();

    unsigned char*   pixels = nullptr;
    int              width = 0, height = 0;// width height
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    std::shared_ptr<MTextureResource> pFontTexture = pResourceSystem->CreateResource<MTextureResource>("ImGUI_Font");
    pFontTexture->Load(MTextureResourceUtil::LoadFromMemory(
            "ImGUI_Font",
            MSpan<MByte>(pixels, width * height * 4),
            width,
            height,
            4,
            MTexturePixelType::Byte8
    ));
    m_FontTexture.SetResource(pFontTexture);

    // Store our identifier
    io.Fonts->TexID = {pFontTexture->GetTextureTemplate(), intptr_t(pFontTexture->GetTextureTemplate().get()), 0};
}

void ImGuiRenderable::Release()
{
    ReleaseMaterial();
    ReleaseFont();

    ReleaseMesh();
}

void ImGuiRenderable::ReleaseFont() { m_FontTexture.SetResource(nullptr); }

void ImGuiRenderable::InitializeMaterial()
{
    MResourceSystem* pResourceSystem = m_engine->FindSystem<MResourceSystem>();

    auto             pTemplate = pResourceSystem->CreateResource<MMaterialTemplate>();
    pTemplate->LoadShader("Shader/Imgui/imgui.mvs");
    pTemplate->LoadShader("Shader/Imgui/imgui.mps");
    pTemplate->SetMaterialType(MEMaterialType::EImGui);
    pTemplate->SetCullMode(MECullMode::ECullNone);

    m_material = MMaterial::CreateMaterial(pTemplate);
}

void ImGuiRenderable::ReleaseMaterial()
{
    if (m_material) { m_material = nullptr; }
}

void ImGuiRenderable::ReleaseMesh()
{
    MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
    m_Mesh.DestroyBuffer(pRenderSystem->GetDevice());
}

void ImGuiRenderable::Tick(const float& fDelta)
{
    MORTY_UNUSED(fDelta);

    MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
    for (auto iter = m_imGuiDrawTexture.begin(); iter != m_imGuiDrawTexture.end();)
    {
        int& count = iter->second->nDestroyCount;
        ++count;

        if (count > 30)
        {
            iter->second->pPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
            iter = m_imGuiDrawTexture.erase(iter);
        }
        else { ++iter; }
    }

    UpdateMesh();
}

void ImGuiRenderable::WaitTextureReady(MIRenderCommand* pCommand)
{
    std::set<ImTextureID> tTextures;

    auto                  draw_data = ImGui::GetDrawData();
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            tTextures.insert(pcmd->TextureId);
        }
    }


    for (const ImTextureID& texid: tTextures)
    {
        if (MTexturePtr pTexture = texid.pTexture)
        {
            pCommand->AddRenderToTextureBarrier({pTexture.get()}, METextureBarrierStage::EPixelShaderSample);
        }
    }
}

void ImGuiRenderable::Render(MIRenderCommand* pCommand)
{
    auto draw_data = ImGui::GetDrawData();
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int  fb_width  = (int) (draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int  fb_height = (int) (draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;

    pCommand->SetViewport(MViewportInfo(0.0f, fb_height, fb_width, -fb_height));

    Vector2 scale;
    scale.x = 2.0f / draw_data->DisplaySize.x;
    scale.y = 2.0f / draw_data->DisplaySize.y;
    Vector2 translate;
    translate.x = -1.0f - draw_data->DisplayPos.x * scale.x;
    translate.y = -1.0f - draw_data->DisplayPos.y * scale.y;


    if (const std::shared_ptr<MShaderConstantParam>& pParam = m_material->GetMaterialPropertyBlock()->m_params[0])
    {
        MVariantStruct& imguiUniform = pParam->var.GetValue<MVariantStruct>();
        {
            imguiUniform.SetVariant(MShaderPropertyName::IMGUI_SCALE, scale);
            imguiUniform.SetVariant(MShaderPropertyName::IMGUI_TRANSLATE, translate);
            pParam->SetDirty();
        }
    }

    pCommand->SetUseMaterial(m_material);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2       clip_off   = draw_data->DisplayPos;      // (0,0) unless using multi-viewports
    ImVec2       clip_scale = draw_data->FramebufferScale;// (1,1) unless using retina display which are often (2,2)

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int          global_vtx_offset = 0;
    int          global_idx_offset = 0;
    ImGuiTexture using_texture     = {nullptr, intptr_t(), 0};
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            if (using_texture != pcmd->TextureId)
            {
                using_texture        = pcmd->TextureId;
                MTexturePtr pTexture = using_texture.pTexture;
                if (auto dest = GetTexturPropertyBlock(using_texture))
                {
                    dest->nDestroyCount = 0;
                    pCommand->SetShaderPropertyBlock(dest->pPropertyBlock);
                }
                else { MORTY_ASSERT(false); }
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
                if (clip_rect.x < 0.0f) clip_rect.x = 0.0f;
                if (clip_rect.y < 0.0f) clip_rect.y = 0.0f;

                pCommand->SetScissor(
                        MScissorInfo(clip_rect.x, clip_rect.y, clip_rect.z - clip_rect.x, clip_rect.w - clip_rect.y)
                );

                pCommand->DrawMesh(
                        &m_Mesh,
                        pcmd->IdxOffset + global_idx_offset,
                        pcmd->ElemCount,
                        pcmd->VtxOffset + global_vtx_offset
                );
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}

ImGuiRenderable::MImGuiTextureDest* ImGuiRenderable::GetTexturPropertyBlock(ImGuiTexture key)
{
    auto findResult = m_imGuiDrawTexture.find(key);

    if (findResult == m_imGuiDrawTexture.end())
    {
        MImGuiTextureDest* pDest = new MImGuiTextureDest();

        pDest->pTexture       = key.pTexture;
        pDest->nDestroyCount  = 0;
        pDest->pPropertyBlock = MMaterialTemplate::CreateMeshPropertyBlock(m_material->GetShaderProgram());


        MVariantStruct& imguiUniform = pDest->pPropertyBlock->m_params[0]->var.GetValue<MVariantStruct>();
        {
            int     nImageType         = 0;
            int     nSingleChannelFlag = 0;
            int     nImageIndex        = 0;
            Vector2 f2ImageSize;
            switch (key.pTexture->GetFormat())
            {
                case METextureFormat::Depth:
                case METextureFormat::UNorm_R8:
                case METextureFormat::Float_R32:
                    nImageType         = 0;
                    nSingleChannelFlag = 1;
                    break;

                case METextureFormat::UInt_R8:
                    nImageType  = 2;
                    f2ImageSize = Vector2(key.pTexture->GetSize2D().x, key.pTexture->GetSize2D().y);
                    break;

                default: nImageType = 0; break;
            }

            if (key.pTexture->GetTextureType() == METextureType::ETexture2DArray)
            {
                nImageType += 1;
                nImageIndex = static_cast<int>(key.nArrayIdx);
            }

            if (imguiUniform.GetVariant<int>(MShaderPropertyName::IMGUI_IMAGE_TYPE) != nImageType ||
                imguiUniform.GetVariant<int>(MShaderPropertyName::IMGUI_SINGLE_CHANNEL_FLAG) != nSingleChannelFlag ||
                imguiUniform.GetVariant<int>(MShaderPropertyName::IMGUI_IMAGE_INDEX) != nImageIndex ||
                imguiUniform.GetVariant<Vector2>(MShaderPropertyName::IMGUI_IMAGE_SIZE) != f2ImageSize)
            {
                imguiUniform.SetVariant(MShaderPropertyName::IMGUI_IMAGE_TYPE, nImageType);
                imguiUniform.SetVariant(MShaderPropertyName::IMGUI_SINGLE_CHANNEL_FLAG, nSingleChannelFlag);
                imguiUniform.SetVariant(MShaderPropertyName::IMGUI_IMAGE_INDEX, nImageIndex);
                imguiUniform.SetVariant(MShaderPropertyName::IMGUI_IMAGE_SIZE, f2ImageSize);
                pDest->pPropertyBlock->m_params[0]->SetDirty();
            }

            if (pDest->pPropertyBlock->m_textures[nImageType]->pTexture != key.pTexture)
            {
                pDest->pPropertyBlock->m_textures[nImageType]->pTexture = key.pTexture;
                pDest->pPropertyBlock->m_textures[nImageType]->SetDirty();
            }
        }


        m_imGuiDrawTexture[key] = pDest;
        return pDest;
    }

    return findResult->second;
}
