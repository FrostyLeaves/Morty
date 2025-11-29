#include "ImGuiRenderer.h"
#include "Utility/MGlobal.h"
#include "Basic/MTexture.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
#include "Resource/MTextureResource.h"
#include "Resource/MTextureResourceUtil.h"
#include "Shader/MShaderParameterSet.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

using namespace morty;

ImGuiRenderer::ImGuiRenderer(MEngine* engine)
    : m_engine(engine)
    , m_Mesh(true)
    , m_material(nullptr)
    , m_FontTexture()
    , m_imGuiDrawTexture()
{}

void ImGuiRenderer::UpdateMesh()
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

void ImGuiRenderer::Initialize()
{
    InitializeFont();
    InitializeMaterial();
}

void ImGuiRenderer::InitializeFont()
{
    auto           resourceSystem = m_engine->FindSystem<MResourceSystem>();

    ImGuiIO&       io = ImGui::GetIO();

    unsigned char* pixels = nullptr;
    int            width = 0, height = 0;// width height
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    std::shared_ptr<MTextureResource> pFontTexture = resourceSystem->CreateResource<MTextureResource>("ImGUI_Font");
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

void ImGuiRenderer::Release()
{
    ReleaseMaterial();
    ReleaseFont();

    ReleaseMesh();
}

void ImGuiRenderer::ReleaseFont() { m_FontTexture.SetResource(nullptr); }

void ImGuiRenderer::InitializeMaterial()
{
    auto resourceSystem = m_engine->FindSystem<MResourceSystem>();
    auto pTemplate      = resourceSystem->CreateResource<MMaterialTemplate>();
    //pTemplate->LoadShader("ShaderSlang/Main/ImGui/ImGuiModule.slang");
    pTemplate->LoadShader("Shader/Imgui/imgui.hlsl");

    auto pass = pTemplate->SetPass(
            MRenderGlobal::DEFAULT_PASS_NAME,
            MRenderGlobal::DEFAULT_VERTEX_ENTRY,
            MRenderGlobal::DEFAULT_PIXEL_ENTRY
    );
    pass->SetBlendState(
            0,
            {true,
             MEBlendFactor::SrcAlpha,
             MEBlendFactor::OneMinusSrcAlpha,
             MEBlendOp::Add,
             MEBlendFactor::SrcAlpha,
             MEBlendFactor::OneMinusSrcAlpha,
             MEBlendOp::Add,
             0xF}
    );
    pass->SetCullMode(MECullMode::ECullNone);


    m_material = MMaterial::CreateMaterial(pTemplate);
}

void ImGuiRenderer::ReleaseMaterial()
{
    if (m_material) { m_material = nullptr; }
}

void ImGuiRenderer::ReleaseMesh()
{
    auto renderSystem = m_engine->FindSystem<MRenderSystem>();
    m_Mesh.DestroyBuffer(renderSystem->GetDevice());
}

void ImGuiRenderer::Tick(const float& delta)
{
    MORTY_UNUSED(delta);

    auto renderSystem = m_engine->FindSystem<MRenderSystem>();
    for (auto iter = m_imGuiDrawTexture.begin(); iter != m_imGuiDrawTexture.end();)
    {
        int& count = iter->second->nDestroyCount;
        ++count;

        if (count > 30)
        {
            iter->second->pParameterSet->DestroyBuffer(renderSystem->GetDevice());
            iter = m_imGuiDrawTexture.erase(iter);
        }
        else { ++iter; }
    }

    UpdateMesh();
}

void ImGuiRenderer::Render(MRenderPassCmd* pCommand)
{
    auto  draw_data = ImGui::GetDrawData();
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    float fb_width  = (draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    float fb_height = (draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;

    pCommand->SetViewport({.x = 0.0f, .y = fb_height, .width = fb_width, .height = -fb_height});

    Vector2 scale;
    scale.x = 2.0f / draw_data->DisplaySize.x;
    scale.y = 2.0f / draw_data->DisplaySize.y;
    Vector2 translate;
    translate.x = -1.0f - draw_data->DisplayPos.x * scale.x;
    translate.y = -1.0f - draw_data->DisplayPos.y * scale.y;


    auto propertyBlock = m_material->GetMaterialParameterSet();
    propertyBlock->SetValue(MShaderPropertyName::IMGUI_SCALE, scale);
    propertyBlock->SetValue(MShaderPropertyName::IMGUI_TRANSLATE, translate);

    pCommand->SetMaterial(m_material.get(), m_material->GetTemplate()->GetDefaultPass());

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
                using_texture       = pcmd->TextureId;
                MTexturePtr texture = using_texture.texture;
                if (auto dest = GetTexturParameterSet(using_texture))
                {
                    dest->nDestroyCount = 0;
                    pCommand->SetShaderParameterSet(dest->pParameterSet);
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
                        {.x      = clip_rect.x,
                         .y      = clip_rect.y,
                         .width  = clip_rect.z - clip_rect.x,
                         .height = clip_rect.w - clip_rect.y}
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

ImGuiRenderer::MImGuiTextureDest* ImGuiRenderer::GetTexturParameterSet(ImGuiTexture key)
{
    auto findResult = m_imGuiDrawTexture.find(key);

    if (findResult == m_imGuiDrawTexture.end())
    {
        auto* pDest = new MImGuiTextureDest();

        pDest->texture       = key.texture;
        pDest->nDestroyCount = 0;
        pDest->pParameterSet = m_material->GetTemplate()->CreateParameterSet(MRenderGlobal::SHADER_PARAM_SET_MESH);


        static const MStringId TexNameList[] = {
                MStringId("image"),
                MStringId("arrayImage"),
                MStringId("uintImage"),
                MStringId("uintArrayImage"),
        };


        MVariantStruct& imguiUniform = pDest->pParameterSet->GetConstantParams()[0]->var.GetValue<MVariantStruct>();
        {
            int        nImageType         = 0;
            int        nSingleChannelFlag = 0;
            int        nImageIndex        = 0;
            const bool isTexArray         = key.texture->GetTextureType() == METextureType::ETexture2DArray;
            Vector2    f2ImageSize;
            switch (key.texture->GetFormat())
            {
                case METextureFormat::Depth:
                case METextureFormat::UNorm_R8:
                case METextureFormat::Float_R32:
                    nImageType         = 0;
                    nSingleChannelFlag = 1;
                    break;

                case METextureFormat::UInt_R8:
                    nImageType  = 2;
                    f2ImageSize = Vector2(key.texture->GetSize2D().x, key.texture->GetSize2D().y);
                    break;

                default: nImageType = 0; break;
            }

            if (isTexArray)
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
                pDest->pParameterSet->GetConstantParams()[0]->SetDirty();
            }

            pDest->pParameterSet->SetTexture(TexNameList[nImageType], key.texture);
        }


        m_imGuiDrawTexture[key] = pDest;
        return pDest;
    }

    return findResult->second;
}
