#include "MPostProcessNode.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSkyBoxComponent.h"

#include "Mesh/MMeshManager.h"
#include "Manager/MEnvironmentManager.h"

void MPostProcessNode::SetMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	m_pMaterial = pMaterial;
}
