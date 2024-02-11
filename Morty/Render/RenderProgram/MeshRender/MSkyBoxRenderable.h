/**
 * @File         MSkyBoxRenderable
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

MORTY_SPACE_BEGIN

class MScene;
class MSkyBoxComponent;
class MMaterialBatchGroup;
class MORTY_API MSkyBoxRenderable : public IRenderable
{
public:

	void SetMesh(MIMesh* pMesh);
	void SetMaterial(const std::shared_ptr<MMaterial>& pMaterial);
	void SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter);

	void Render(MIRenderCommand* pCommand) override;

private:

	MIMesh* m_pMesh = nullptr;
	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::vector<std::shared_ptr<IPropertyBlockAdapter>> m_vFramePropertyAdapter;
};

MORTY_SPACE_END