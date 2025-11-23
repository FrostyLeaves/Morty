/**
 * @File         MShaderProgramSystem
 * 
 * @Created      2025-09-02 00:13:40
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "Component/MComponent.h"
#include "Engine/MSystem.h"
#include "RHI/MRenderPass.h"

namespace morty
{

class MShaderProgram;
class MTaskNode;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MMeshBufferAdapter;
class MRenderMeshComponent;
class MMaterialPass;
typedef MHashCode MShaderProgramKey;

class MShaderProgramReference
{
public:
    MShaderProgramKey                        key;
    std::unordered_set<const MMaterialPass*> owner;
    std::unique_ptr<MShaderProgram>          program;
};

class MORTY_API MShaderProgramSystem : public MISystem
{
public:
    MORTY_CLASS(MShaderProgramSystem)

    explicit        MShaderProgramSystem() = default;

    MShaderProgram* CreateShaderProgram(const MMaterialPass* pass);
    MShaderProgram* FindShaderProgram(const MMaterialPass* pass);

    void            ReleaseShaderProgram(const MMaterialPass* pass);

public:
    std::map<MShaderProgramKey, std::shared_ptr<MShaderProgramReference>>    m_shaderProgramTable;
    std::map<const MMaterialPass*, std::shared_ptr<MShaderProgramReference>> m_materialPassTable;
};

}// namespace morty