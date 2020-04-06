#include "MSkyBox.h"

#include "Texture/MTextureCubeResource.h"

#include "MEngine.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"
#include "MMaterial.h"
#include "Texture/MTextureResource.h"
#include "Model/MModelMeshStruct.h"

#include "Node/Model/MStaticMeshInstance.h"


MTypeIdentifierImplement(MSkyBox, MObject)

MSkyBox::MSkyBox()
	: MObject()
	, m_pMeshData(nullptr)
	, m_pBoxMesh(nullptr)
	, m_pMeshData(nullptr)
	, m_pTextureCube(nullptr)
	, m_pTextureCubeResource(nullptr)
{

}

MSkyBox::~MSkyBox()
{
	if (m_pMeshData)
	{
		delete m_pMeshData;
		m_pMeshData = nullptr;
	}

	if (m_pBoxMesh)
	{
		m_pBoxMesh->DestroyBuffer(m_pEngine->GetDevice());
		delete m_pBoxMesh;
		m_pBoxMesh = nullptr;
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
		m_pTextureCubeResource->SetResChangedCallback([this](const unsigned int& eReloadType){
			m_pTextureCube = static_cast<MTextureCubeResource*>(m_pTextureCubeResource->GetResource())->GetTextureCubeTemplate();
			return true;
		});

		return true;
	}

	return false;
}

void MSkyBox::OnCreated()
{
	MMaterialResource* pMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SKYBOX);
	MMaterial* pMaterial = pMaterialRes->GetMaterialTemplate();

	

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

	m_pMeshData = new MModelMeshStruct();
	m_pMeshData->SetMesh(m_pBoxMesh);
	m_pMeshInstance->SetMeshData(m_pMeshData);

	m_pMeshInstance->SetMaterial(pMaterial);
}
