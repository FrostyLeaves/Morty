// MortyDemo.cpp : 瀹氫箟鎺у埗鍙板簲鐢ㄧ▼搴忕殑鍏ュ彛鐐广€?
//

#include "stdafx.h"
#include "MEngine.h"

#include "MObject.h"
#include "MMaterial.h"
#include "MSpatial.h"
#include "MResourceManager.h"
#include "MVariable.h"
#include "MMaterialResource.h"

#include "MShader.h"
#include "MVertex.h"
#include "MIRenderer.h"
#include "MMeshInstance.h"


class MySpatial : public MSpatial
{
public:

	MySpatial(){
		m_fTime = 0.0f;
		m_color = Vector4(0.5, 0.5, 0.5, 1);
	}


	virtual void OnTick(const float& fDelta)
	{
		m_fTime += fDelta;

// 		m_color.x = sin(m_fTime) * 0.5f + 0.5f;
// 		m_color.z = cos(m_fTime) * 0.5f + 0.5f;

		for (MNode* pChild : this->GetChildren())
		{
			MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pChild);
			MMaterial* pMaterial = pMeshIns->GetMaterial();

			std::vector<MShaderParam>& vParams = pMaterial->GetVertexShaderParams();
			for (MShaderParam& param : vParams)
			{
				if (param.strName == "cbPerObject")
				{
					param.var.GetStruct()->SetMember("TestColor", m_color);
				}
			}
		}

	}

private:

	Vector4 m_color;
	float m_fTime;
};

int main(int argc, char* argv[])
{
    
    
	MEngine engine;

	engine.Initialize();
	engine.CreateView();

	
	MResource* pVSResource = engine.GetResourceManager()->Load("./Shader/defaultv.mvs");
	MResource* pPSResource = engine.GetResourceManager()->Load("./Shader/defaultp.mps");
	MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(engine.GetResourceManager()->Create(MResourceManager::MEResourceType::Material));
	pMaterialRes->LoadVertexShader(pVSResource);
	pMaterialRes->LoadPixelShader(pPSResource);

	MMaterial* pMaterial = engine.GetObjectManager()->CreateObject<MMaterial>();
	pMaterial->Load(pMaterialRes);

	std::vector<MShaderParam>& vParams = pMaterial->GetVertexShaderParams();

	for (MShaderParam& param : vParams)
	{
		if (param.strName == "cbPerObject")
		{
			param.var.GetStruct()->SetMember("testColor", Vector4(1, 1, 1, 1));
		}
	}

	MResource* pResource = engine.GetResourceManager()->Load("./Model/box.fbx");
	MSpatial* pSpatial = engine.GetObjectManager()->CreateObject<MySpatial>();
	pSpatial->Load(pResource);
	pSpatial->SetPosition(Vector3(0, 0, 100));

	for (MNode* pChild : pSpatial->GetChildren())
	{
		MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pChild);
		pMeshIns->SetMaterial(pMaterial);
	}
	
	engine.SetRootNode(pSpatial);
	
	while (engine.MainLoop());

	engine.Release();

	return 0;
}
