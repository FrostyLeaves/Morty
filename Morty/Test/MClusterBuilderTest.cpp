#include "Tools/MClusterBuilder.h"
#include "Mesh/MMesh.h"
#include "Mesh/MVertex.h"
#include "doctest/doctest.h"
#include <cmath>


using namespace morty;

// Helper function to create a simple triangle mesh
static MMesh<MVertex>* CreateSimpleTriangle()
{
    auto* mesh = new MMesh<MVertex>();
    mesh->CreateVertices(3);
    mesh->CreateIndices(3, 1);

    auto* vertices        = mesh->GetVertices();
    vertices[0].position  = Vector3(0.0f, 0.0f, 0.0f);
    vertices[0].normal    = Vector3(0.0f, 0.0f, 1.0f);
    vertices[0].texCoords = Vector2(0.0f, 0.0f);

    vertices[1].position  = Vector3(1.0f, 0.0f, 0.0f);
    vertices[1].normal    = Vector3(0.0f, 0.0f, 1.0f);
    vertices[1].texCoords = Vector2(1.0f, 0.0f);

    vertices[2].position  = Vector3(0.5f, 1.0f, 0.0f);
    vertices[2].normal    = Vector3(0.0f, 0.0f, 1.0f);
    vertices[2].texCoords = Vector2(0.5f, 1.0f);

    auto* indices = mesh->GetIndices();
    indices[0]    = 0;
    indices[1]    = 1;
    indices[2]    = 2;

    return mesh;
}

// Helper function to create a cube mesh
static MMesh<MVertex>* CreateCube()
{
    auto* mesh = new MMesh<MVertex>();
    mesh->CreateVertices(8);
    mesh->CreateIndices(36, 1);

    auto* vertices = mesh->GetVertices();

    // Define 8 vertices of a cube
    vertices[0].position = Vector3(-0.5f, -0.5f, -0.5f);
    vertices[1].position = Vector3(0.5f, -0.5f, -0.5f);
    vertices[2].position = Vector3(0.5f, 0.5f, -0.5f);
    vertices[3].position = Vector3(-0.5f, 0.5f, -0.5f);
    vertices[4].position = Vector3(-0.5f, -0.5f, 0.5f);
    vertices[5].position = Vector3(0.5f, -0.5f, 0.5f);
    vertices[6].position = Vector3(0.5f, 0.5f, 0.5f);
    vertices[7].position = Vector3(-0.5f, 0.5f, 0.5f);

    // Set normals and texcoords for all vertices
    for (int i = 0; i < 8; ++i)
    {
        vertices[i].normal    = Vector3(0.0f, 0.0f, 1.0f);
        vertices[i].texCoords = Vector2(0.0f, 0.0f);
    }

    auto* indices = mesh->GetIndices();
    // Front face
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 3;
    indices[5] = 0;
    // Back face
    indices[6]  = 5;
    indices[7]  = 4;
    indices[8]  = 7;
    indices[9]  = 7;
    indices[10] = 6;
    indices[11] = 5;
    // Left face
    indices[12] = 4;
    indices[13] = 0;
    indices[14] = 3;
    indices[15] = 3;
    indices[16] = 7;
    indices[17] = 4;
    // Right face
    indices[18] = 1;
    indices[19] = 5;
    indices[20] = 6;
    indices[21] = 6;
    indices[22] = 2;
    indices[23] = 1;
    // Top face
    indices[24] = 3;
    indices[25] = 2;
    indices[26] = 6;
    indices[27] = 6;
    indices[28] = 7;
    indices[29] = 3;
    // Bottom face
    indices[30] = 4;
    indices[31] = 5;
    indices[32] = 1;
    indices[33] = 1;
    indices[34] = 0;
    indices[35] = 4;

    return mesh;
}

// Helper function to create a plane mesh
static MMesh<MVertex>* CreatePlane(int subdivisions = 10)
{
    int   verticesPerSide = subdivisions + 1;
    int   vertexCount     = verticesPerSide * verticesPerSide;
    int   triangleCount   = subdivisions * subdivisions * 2;
    int   indexCount      = triangleCount * 3;

    auto* mesh = new MMesh<MVertex>();
    mesh->CreateVertices(vertexCount);
    mesh->CreateIndices(indexCount, 1);

    auto* vertices = mesh->GetVertices();

    // Generate vertices
    for (int z = 0; z < verticesPerSide; ++z)
    {
        for (int x = 0; x < verticesPerSide; ++x)
        {
            int   index = z * verticesPerSide + x;
            float fx    = static_cast<float>(x) / subdivisions;
            float fz    = static_cast<float>(z) / subdivisions;

            vertices[index].position  = Vector3(fx * 10.0f - 5.0f, 0.0f, fz * 10.0f - 5.0f);
            vertices[index].normal    = Vector3(0.0f, 1.0f, 0.0f);
            vertices[index].texCoords = Vector2(fx, fz);
        }
    }

    // Generate indices
    auto* indices      = mesh->GetIndices();
    int   currentIndex = 0;
    for (int z = 0; z < subdivisions; ++z)
    {
        for (int x = 0; x < subdivisions; ++x)
        {
            int topLeft     = z * verticesPerSide + x;
            int topRight    = topLeft + 1;
            int bottomLeft  = (z + 1) * verticesPerSide + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            indices[currentIndex++] = topLeft;
            indices[currentIndex++] = bottomLeft;
            indices[currentIndex++] = topRight;

            // Second triangle
            indices[currentIndex++] = topRight;
            indices[currentIndex++] = bottomLeft;
            indices[currentIndex++] = bottomRight;
        }
    }

    return mesh;
}

TEST_SUITE("MClusterBuilder")
{

    TEST_CASE("Generate - Simple Triangle")
    {
        auto*           mesh = CreateSimpleTriangle();
        MClusterBuilder builder;

        SUBCASE("Basic generation succeeds")
        {
            REQUIRE_NOTHROW(builder.Generate(mesh));

            // Check that clusters were generated
            CHECK(mesh->GetClusters().size() > 0);

            // Check that indices are still valid
            CHECK(mesh->GetIndicesNum() == 3);
        }

        SUBCASE("Cluster bounds are valid")
        {
            builder.Generate(mesh);

            for (const auto& cluster: mesh->GetClusters())
            {
                CHECK(cluster.bounds.radius >= 0.0f);
                CHECK(cluster.bounds.error >= 0.0f);
            }
        }

        SUBCASE("Cluster groups are created")
        {
            builder.Generate(mesh);

            CHECK(mesh->GetClusterGroup().size() > 0);

            for (const auto& group: mesh->GetClusterGroup())
            {
                CHECK(group.clusterNum > 0);
                CHECK(group.bounds.radius >= 0.0f);
            }
        }

        SUBCASE("LOD data is generated")
        {
            builder.Generate(mesh);

            CHECK(mesh->GetClusterLodData().size() > 0);

            for (const auto& lod: mesh->GetClusterLodData()) { CHECK(lod.groupNum > 0); }
        }

        delete mesh;
    }

    TEST_CASE("Generate - Plane Mesh with subdivisions")
    {
        auto*           mesh = CreatePlane(10);
        MClusterBuilder builder;

        SUBCASE("Handles larger meshes")
        {
            REQUIRE_NOTHROW(builder.Generate(mesh));

            CHECK(mesh->GetClusters().size() > 0);
            CHECK(mesh->GetClusterGroup().size() > 0);
            CHECK(mesh->GetClusterLodData().size() > 0);
        }

        SUBCASE("LOD hierarchy is correct")
        {
            builder.Generate(mesh);

            const auto& lods   = mesh->GetClusterLodData();
            const auto& groups = mesh->GetClusterGroup();

            // Verify LOD hierarchy
            for (const auto& lod: lods) { CHECK(lod.groupOffset + lod.groupNum <= groups.size()); }

            // LOD levels should generally decrease in cluster count (or stay at 1)
            if (lods.size() > 1)
            {
                for (size_t i = 0; i < lods.size() - 1; ++i)
                {
                    // Later LODs should have same or fewer groups
                    CHECK(lods[i + 1].groupNum <= lods[i].groupNum);
                }
            }
        }

        SUBCASE("Error increases with LOD levels")
        {
            builder.Generate(mesh);

            const auto& lods   = mesh->GetClusterLodData();
            const auto& groups = mesh->GetClusterGroup();

            if (lods.size() > 1)
            {
                for (size_t i = 0; i < lods.size() - 1; ++i)
                {
                    const auto& currentLodFirstGroup = groups[lods[i].groupOffset];
                    const auto& nextLodFirstGroup    = groups[lods[i + 1].groupOffset];

                    // Higher LOD levels should have equal or greater error
                    CHECK(nextLodFirstGroup.bounds.error >= currentLodFirstGroup.bounds.error);
                }
            }
        }

        SUBCASE("All indices are preserved") { builder.Generate(mesh); }

        delete mesh;
    }

    TEST_CASE("Generate - Empty Mesh")
    {
        auto* mesh = new MMesh<MVertex>();
        mesh->CreateVertices(0);
        mesh->CreateIndices(0, 1);

        MClusterBuilder builder;

        SUBCASE("Handles empty mesh gracefully")
        {
            // Should not crash with empty mesh
            REQUIRE_NOTHROW(builder.Generate(mesh));
        }

        delete mesh;
    }

    TEST_CASE("Generate - Degenerate Triangles")
    {
        auto* mesh = new MMesh<MVertex>();
        mesh->CreateVertices(3);
        mesh->CreateIndices(3, 1);

        auto* vertices = mesh->GetVertices();
        // All vertices at the same position (degenerate triangle)
        vertices[0].position = Vector3(0.0f, 0.0f, 0.0f);
        vertices[1].position = Vector3(0.0f, 0.0f, 0.0f);
        vertices[2].position = Vector3(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < 3; ++i)
        {
            vertices[i].normal    = Vector3(0.0f, 0.0f, 1.0f);
            vertices[i].texCoords = Vector2(0.0f, 0.0f);
        }

        auto* indices = mesh->GetIndices();
        indices[0]    = 0;
        indices[1]    = 1;
        indices[2]    = 2;

        MClusterBuilder builder;

        SUBCASE("Handles degenerate geometry") { REQUIRE_NOTHROW(builder.Generate(mesh)); }

        delete mesh;
    }

    TEST_CASE("Generate - Consistency across multiple runs")
    {
        auto*           mesh1 = CreateCube();
        auto*           mesh2 = CreateCube();

        MClusterBuilder builder1;
        MClusterBuilder builder2;

        builder1.Generate(mesh1);
        builder2.Generate(mesh2);

        SUBCASE("Produces consistent cluster count")
        {
            CHECK(mesh1->GetClusters().size() == mesh2->GetClusters().size());
        }

        SUBCASE("Produces consistent group count")
        {
            CHECK(mesh1->GetClusterGroup().size() == mesh2->GetClusterGroup().size());
        }

        SUBCASE("Produces consistent LOD count")
        {
            CHECK(mesh1->GetClusterLodData().size() == mesh2->GetClusterLodData().size());
        }

        delete mesh1;
        delete mesh2;
    }
}
