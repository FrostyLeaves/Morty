#include "MSkyBox.h"

#include "Texture/MTextureCubeResource.h"

#include "MEngine.h"
#include "MIRenderer.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"
#include "Texture/MTextureResource.h"
#include "Model/MMeshResource.h"

#include "Node/Model/MStaticMeshInstance.h"


M_OBJECT_IMPLEMENT(MSkyBox, MObject)

MSkyBox::MSkyBox()
	: MObject()
	, m_pMaterial(nullptr)
	, m_pMeshParamSet(nullptr)
	, m_pTransformParam(nullptr)
	, m_pBoxMesh(nullptr)
	, m_pTextureCube(nullptr)
	, m_TextureCubeResource(nullptr)
{

}

MSkyBox::~MSkyBox()
{
}

bool MSkyBox::Load(MResource* pResource)
{
	if (MTextureCubeResource* pCubeRes = dynamic_cast<MTextureCubeResource*>(pResource))
	{
		m_TextureCubeResource.SetResource(pResource);
		m_TextureCubeResource.SetResChangedCallback([this](const uint32_t& eReloadType){
			m_pTextureCube = static_cast<MTextureCubeResource*>(m_TextureCubeResource.GetResource())->GetTextureCubeTemplate();
			return true;
		});

		return true;
	}

	return false;
}

void MSkyBox::OnCreated()
{
	Super::OnCreated();

	MResourceManager* pResourceManager = GetEngine()->GetResourceManager();

	MResource* pSkyBoxVSResource = pResourceManager->LoadResource("./Shader/skybox.mvs");
	MResource* pSkyBoxPSResource = pResourceManager->LoadResource("./Shader/skybox.mps");
	m_pMaterial = pResourceManager->LoadVirtualResource<MMaterialResource>("SkyBox_Material_For_Object_" + MStringHelper::ToString(GetObjectID()));
	m_pMaterial->SetRasterizerType(MERasterizerType::ECullNone);
	m_pMaterial->LoadVertexShader(pSkyBoxVSResource);
	m_pMaterial->LoadPixelShader(pSkyBoxPSResource);
	m_pMaterial->AddRef();

	const MString vTexturePath[6] = {
		"ashcanyon_lf.tga",
		"ashcanyon_rt.tga",
		"ashcanyon_up.tga",
		"ashcanyon_dn.tga",
		"ashcanyon_ft.tga",
		"ashcanyon_bk.tga",
	};

	MTextureResource* vTextureRes[6];
	for (int i = 0; i < 6; ++i)
	{
		vTextureRes[i] = static_cast<MTextureResource*>(pResourceManager->LoadResource("./Texture/skybox/" + vTexturePath[i]));
	}

	MTextureCubeResource* pTextureCubeRes = pResourceManager->LoadVirtualResource<MTextureCubeResource>("SkyBox_TextureCube_For_Object_" + MStringHelper::ToString(GetObjectID()));
	pTextureCubeRes->SetTextures(vTextureRes);

	m_pMaterial->SetTexutreParam("SkyTexCube", pTextureCubeRes);

	if (MShaderParamSet* pParamSet = m_pMaterial->GetMeshParamSet())
	{
		m_pMeshParamSet = pParamSet->Clone();
		m_pTransformParam = m_pMeshParamSet->FindConstantParam("_M_E_cbMeshMatrix");
	}

	m_pBoxMesh = new MMesh<Vector3>();

	m_pBoxMesh->CreateVertices(8);
	m_pBoxMesh->CreateIndices(12, 3);

	m_pBoxMesh->GetVertices()[0] = Vector3(-1.0, -1.0, 1.0);
	m_pBoxMesh->GetVertices()[1] = Vector3(-1.0, 1.0, 1.0);
	m_pBoxMesh->GetVertices()[2] = Vector3(1.0, 1.0, 1.0);
	m_pBoxMesh->GetVertices()[3] = Vector3(1.0, -1.0, 1.0);

	m_pBoxMesh->GetVertices()[4] = Vector3(-1.0, -1.0, -1.0);
	m_pBoxMesh->GetVertices()[5] = Vector3(-1.0, 1.0, -1.0);
	m_pBoxMesh->GetVertices()[6] = Vector3(1.0, 1.0, -1.0);
	m_pBoxMesh->GetVertices()[7] = Vector3(1.0, -1.0, -1.0);
	
	const float const indexs[] = {
		3, 2, 6,
		3, 6, 7,//right

		0, 1, 5,
		0, 5, 4,//left

		5, 1, 2,
		5, 2, 6,//top

		4, 0, 3,
		4, 3, 7,//bottom

		0, 1, 2,
 		0, 2, 3,//front

 		4, 5, 6,
 		4, 6, 7,//back
	};

	for (int i = 0; i < m_pBoxMesh->GetIndicesLength(); ++i)
		m_pBoxMesh->GetIndices()[i] = indexs[i];
}

void MSkyBox::OnDelete()
{
	m_pMaterial->SubRef();

	if (m_pMeshParamSet)
	{
		m_pMeshParamSet->DestroyBuffer(GetEngine()->GetDevice());
		delete m_pMeshParamSet;
		m_pMeshParamSet = nullptr;
	}

	if (m_pBoxMesh)
	{
		m_pBoxMesh->DestroyBuffer(m_pEngine->GetDevice());
		delete m_pBoxMesh;
		m_pBoxMesh = nullptr;
	}

	m_TextureCubeResource.SetResource(nullptr);

	Super::OnDelete();
}
