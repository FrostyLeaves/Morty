#pragma once


#include "imgui.h"

#include "Mesh/MMesh.h"
#include "Resource/MResource.h"

#include <map>

namespace morty
{

class MEngine;
class MTexture;
class MRenderPassCmd;
class IRenderCommand;
class MShaderParameterSet;
class ImGuiRenderer
{
public:
    ImGuiRenderer(MEngine* pEngine);

    virtual ~ImGuiRenderer() = default;


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

    void Render(MRenderPassCmd* pCommand);


protected:
    struct MImGuiTextureDest {
        int                                  nDestroyCount;
        MTexturePtr                          pTexture;
        std::shared_ptr<MShaderParameterSet> pParameterSet;
    };

    MImGuiTextureDest* GetTexturParameterSet(ImGuiTexture tex);

private:
    MEngine*                                   m_engine;

    MMesh<ImDrawVert>                          m_Mesh;
    std::shared_ptr<MMaterial>                 m_material;
    MResourceRef                               m_FontTexture;

    std::map<ImGuiTexture, MImGuiTextureDest*> m_imGuiDrawTexture;
};

}// namespace morty