#include "MSkyBox.h"

#include "MTextureCubeResource.h"

#include "MEngine.h"
#include "MResourceManager.h"
#include "MMaterialResource.h"
#include "MMaterial.h"
#include "MTextureResource.h"
#include "MStaticMeshInstance.h"

#include "MMesh.h"

MSkyBox::MSkyBox()
	: MObject()
	, m_pBoxMesh(nullptr)
	, m_pTextureCube(nullptr)
	, m_pTextureCubeResource(nullptr)
{

}

MSkyBox::~MSkyBox()
{
	if (m_pBoxMesh)
	{
		m_pBoxMesh->DestroyBuffer(m_pEngine->GetDevice());
	}

	if (m_pTextureCubeResource)
	{
		delete m_pTextureCubeResource;
		m_pTextureCubeResource = nullptr;
	}
}

bool MSkyBox::Load(MResource* pResource)
{
	if (MTextureCubeResource* pCubeRes = dynamic_cast<MTextureCubeResource*>(pResource))
	{
		if (m_pTextureCubeResource)
			delete m_pTextureCubeResource;

		m_pTextureCubeResource = new MResourceHolder(pResource);
		m_pTextureCubeResource->SetResChangedCallback([this](){
			m_pTextureCube = static_cast<MTextureCubeResource*>(m_pTextureCubeResource->GetResource())->GetTextureCubeTemplate();
		});

		return true;
	}

	return false;
}

void MSkyBox::OnCreated()
{
	MResource* pVSResource = m_pEngine->GetResourceManager()->Load("./Shader/skybox.mvs");
	MResource* pPSResource = m_pEngine->GetResourceManager()->Load("./Shader/skybox.mps");
	MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(m_pEngine->GetResourceManager()->Create(MResourceManager::MEResourceType::Material));
	pMaterialRes->LoadVertexShader(pVSResource);
	pMaterialRes->LoadPixelShader(pPSResource);

	MMaterial* pMaterial = m_pEngine->GetObjectManager()->CreateObject<MMaterial>();
	pMaterial->Load(pMaterialRes);

	static MString vTexturePath[6] = {
		"ashcanyon_rt.tga",
		"ashcanyon_lf.tga",
		"ashcanyon_up.tga",
		"ashcanyon_dn.tga",
		"ashcanyon_ft.tga",
		"ashcanyon_bk.tga",
	};

	MTextureResource* vTextureRes[6];
	for (int i = 0; i < 6; ++i)
	{
		vTextureRes[i] = static_cast<MTextureResource*>(m_pEngine->GetResourceManager()->Load("./Texture/skybox/" + vTexturePath[i]));
	}

	MTextureCubeResource* pTextureCubeRes = m_pEngine->GetResourceManager()->CreateResource<MTextureCubeResource>();
	pTextureCubeRes->SetTextures(vTextureRes);

	std::vector<MShaderTextureParam>& vTexParams = pMaterial->GetPixelTextureParams();
	pMaterial->SetPixelTexutreParam("SkyTexCube", pTextureCubeRes);

	m_pMeshInstance = m_pEngine->GetObjectManager()->CreateObject<MStaticMeshInstance>();

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
	
	float indexs[] = {
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

	m_pMeshInstance->SetMesh(m_pBoxMesh);

	m_pMeshInstance->SetMaterial(pMaterial);
}
