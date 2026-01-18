/**
 * @File         MMeshImporter
 *
 * @Created      2025-01-14
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Mesh/MMesh.h"
#include "Mesh/MVertex.h"
#include "Resource/MMeshResource.h"

#include <memory>

struct aiMesh;

namespace morty
{

class MSkeleton;
class MEngine;
class MResourceSystem;

/**
 * @brief Mesh Importer - Handles importing mesh data from Assimp
 *
 * This class is responsible for:
 * - Converting Assimp mesh data to Morty mesh format
 * - Processing vertices (with and without bones)
 * - Processing indices
 * - Binding vertex weights to bones
 * - Creating MMeshResource
 */
class MORTY_API MMeshImporter
{
public:
    explicit MMeshImporter(MEngine* engine);
    ~MMeshImporter();

    /**
     * @brief Enable Nanite cluster generation for static meshes
     */
    void SetEnableNanite(bool enable) { m_enableNanite = enable; }

    /**
     * @brief Import a mesh from Assimp
     * @param pAiMesh Assimp mesh data
     * @param pSkeleton Skeleton (nullptr for static mesh)
     * @param strMeshName Output mesh name
     * @return MMeshResource, nullptr on failure
     */
    std::shared_ptr<MMeshResource> ImportMesh(aiMesh* pAiMesh, MSkeleton* pSkeleton, MString& strMeshName);


private:
    /**
     * @brief Process mesh vertices (static mesh)
     */
    void ProcessMeshVertices(aiMesh* mesh, MMesh<MVertex>* pMMesh);

    /**
     * @brief Process mesh vertices (skeletal mesh)
     */
    void ProcessMeshVertices(aiMesh* mesh, MMesh<MVertexWithBones>* pMMesh);

    /**
     * @brief Process mesh indices
     */
    void ProcessMeshIndices(aiMesh* mesh, MIMesh* pMMesh);

    /**
     * @brief Bind vertex weights to bones
     */
    void BindVertexAndBones(MSkeleton* pSkeleton, aiMesh* mesh, MMesh<MVertexWithBones>* pMMesh);

private:
    MEngine*                                                    m_engine;
    MResourceSystem*                                            m_resourceSystem;
    std::unordered_map<aiMesh*, std::shared_ptr<MMeshResource>> m_cache;
    bool                                                        m_enableNanite = false;
};

}// namespace morty
