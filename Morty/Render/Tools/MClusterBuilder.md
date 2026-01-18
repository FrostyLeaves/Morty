# MClusterBuilder Technical Documentation

## Overview

`MClusterBuilder` implements a Nanite-like hierarchical LOD (Level of Detail) system. It breaks a mesh into multiple clusters, then through iterative simplification and grouping, builds a LOD hierarchy structure. The GPU can select appropriate LOD levels for rendering at runtime based on camera distance and screen-space error.

## Core Data Structures

### MClusterData (Internal Use)

Temporary cluster data used during the build process:

```cpp
struct MClusterData {
    uint32_t              vertexCount;  // vertex count
    std::vector<uint32_t> indices;      // index data
    int32_t               group;        // owning MClusterGroup ID (-1 means none)
    int32_t               refined;      // MClusterGroup ID that produced this cluster via simplification
    MClusterBounds        bounds;       // bounding sphere and error
};
```

### MCluster (Output)

Final output cluster data:

```cpp
struct MCluster {
    int32_t        group;          // owning MClusterGroup ID
    int32_t        refined;        // source MClusterGroup ID from simplification
    uint32_t       indicesOffset;  // offset in MClusterGroup index array
    uint32_t       indicesNum;     // index count
    MClusterBounds bounds;         // bounding sphere and error
};
```

### MClusterGroup (Current Implementation - Tree Structure)

Current cluster group structure using tree hierarchy:

```cpp
struct MClusterGroup {
    uint32_t       clusterOffset;      // start offset in m_allClusters
    uint32_t       clusterNum;         // number of clusters contained
    MClusterBounds bounds;             // merged bounding sphere and error

    // Tree structure (for GPU traversal)
    int32_t        parentGroupId;      // parent node ID (M_INVALID_INT means root)
    int32_t        firstChildGroupId;  // first child node ID
    uint32_t       childGroupCount;    // child count (children are stored contiguously)
};
```

### MClusterLodData (Output)

Metadata for each LOD level:

```cpp
struct MClusterLodData {
    uint32_t groupOffset;  // start offset of this LOD level in m_allGroups
    uint32_t groupNum;     // number of groups in this LOD level
};
```

## Current Implementation Issues

### Problem: Tree Structure Cannot Correctly Represent LOD Hierarchy

During LOD simplification, clusters from **different parent groups** may be assigned to the same partition due to spatial proximity, then merged and simplified into a new group. This new group should logically be a **child of all original parents**.

```
LOD 0:  G0 (contains C0, C1)    G1 (contains C2, C3)
                |                       |
        simplifies to C4        simplifies to C5

LOD 1:  Partition assigns C4 and C5 together (spatially adjacent)
        -> new group G2 (contains C4, C5)

Expected: G2 is a shared child of both G0 and G1
Current:  G2.parent = G0, G1 loses its relationship with G2
```

Current data structure limitations:
- `parentGroupId` can only store one parent
- `firstChildGroupId + childGroupCount` assumes children are stored contiguously

This causes:
1. Some groups become orphaned leaf nodes
2. GPU traversal cannot correctly find all children

---

## Nanite's DAG Structure

Unreal Nanite uses a **DAG (Directed Acyclic Graph)** structure, not a tree structure:

- **DAG**: A child group can have **multiple parents**
- **Tree**: A child group can only have **one parent**

```
Nanite (DAG):                    Current Implementation (Tree):

    G0      G1                       G0      G1
     \      /                         \
      \    /                           \
       \  /                             \
        G2                               G2 (G1 loses relationship with G2)
       (child of both G0 and G1)
```

### Why DAG is Needed

1. **Spatial Proximity Priority**: meshoptimizer's partition algorithm groups by spatial location, ignoring which parent clusters come from
2. **Simplification Quality**: Merging spatially adjacent clusters produces better simplification results
3. **Hierarchy Correctness**: Each parent should correctly point to its simplified children

---

## New Data Structure Design (DAG)

### Design Goals

1. Support a group having multiple parents
2. Support a group having multiple non-contiguous children
3. Maintain GPU traversal efficiency
4. Avoid duplicate rendering of the same group

### Recommended Solution: Index Array with Visited Marking

In a DAG, a group may be referenced by multiple parents, requiring duplicate processing prevention.

```cpp
// CPU-side data structure
struct MClusterGroup {
    uint32_t       clusterOffset;
    uint32_t       clusterNum;
    MClusterBounds bounds;

    // DAG structure: index array
    uint32_t       childIndicesOffset;
    uint32_t       childCount;
};

// GPU-side additional data
struct MClusterGroupRenderData {
    uint32_t valid;
    uint32_t clusterBeginIndex;
    uint32_t clusterCount;
    uint32_t childIndicesOffset;
    uint32_t childCount;
    Vector3  position;
    float    radius;
    float    error;
};

// Visited marking used during GPU traversal (reset each frame)
RWBuffer<uint> groupVisited;  // bitmask or atomic counter
```

**GPU Traversal Logic**:
```slang
void TraverseLODTree_DAG(...) {
    // Initialize: add root groups to stack
    for (int i = 0; i < meshResource.rootGroupCount; ++i) {
        stack.push(meshResource.rootGroupBegin + i);
    }

    while (!stack.empty()) {
        uint groupIdx = stack.pop();

        // Atomic marking to avoid duplicate visits
        uint oldVal;
        InterlockedExchange(groupVisited[groupIdx], 1, oldVal);
        if (oldVal == 1) continue;  // already visited

        ClusterGroupData group = clusterGroups[groupIdx];

        if (ShouldSubdivide(group, camera)) {
            // Traverse children
            for (uint i = 0; i < group.childCount; ++i) {
                uint childIdx = childIndicesArray[group.childIndicesOffset + i];
                stack.push(childIdx);
            }
        } else {
            // Output clusters
            for (uint i = 0; i < group.clusterCount; ++i) {
                outputCluster(group.clusterBeginIndex + i);
            }
        }
    }
}
```

**Advantages**:
- Clean data structure
- Avoids duplicate visits through atomic operations
- No need to store parent relationships (GPU traversal doesn't need them)

**Disadvantages**:
- Need to reset visited buffer each frame
- Atomic operations have some overhead

---

## Recommended Solution Details

### Data Structure Definition

```cpp
// ===== CPU Side =====

struct MClusterGroup {
    uint32_t       clusterOffset;       // cluster offset in global array
    uint32_t       clusterNum;          // cluster count
    MClusterBounds bounds;              // bounding sphere and error

    uint32_t       childIndicesOffset;  // child indices offset in global array
    uint32_t       childCount;          // child count
};

// Global child indices array
std::vector<int32_t> m_groupChildIndices;


// ===== GPU Side =====

struct MClusterGroupRenderData {
    uint32_t valid;
    uint32_t clusterBeginIndex;
    uint32_t clusterCount;
    uint32_t childIndicesOffset;
    uint32_t childCount;
    Vector3  position;
    float    radius;
    float    error;
};

// GPU Buffers
StructuredBuffer<MClusterGroupRenderData> clusterGroups;
StructuredBuffer<uint> childIndicesArray;  // global child indices
RWBuffer<uint> groupVisited;               // visited marking reset each frame
```

### Build Algorithm Modification

```cpp
void MClusterBuilder::BuildCluster(const InputData& input) {
    // Phase 1: Clusterize
    std::vector<MClusterData> clusters = Clusterize(input);
    std::vector<int> pending;  // cluster indices to process

    // Initial clusters: group = -1, refined = -1
    for (int i = 0; i < clusters.size(); ++i) {
        pending.push_back(i);
    }

    int lodLevel = 0;

    // Phase 2: Iterative simplification
    while (pending.size() > 1) {
        // Record current LOD's starting group index
        uint32_t lodGroupOffset = m_allGroups.size();

        // Partition
        auto partitions = PartitionCluster(input, clusters, pending, remap);

        std::vector<int> newPending;

        for (const auto& partition : partitions) {
            // Collect all parent group IDs from clusters in partition
            std::set<int32_t> parentGroupIds;
            for (int clusterIdx : partition) {
                if (clusters[clusterIdx].group != -1) {
                    parentGroupIds.insert(clusters[clusterIdx].group);
                }
            }

            // Merge and simplify
            auto mergedIndices = MergeClusterIndices(clusters, partition);
            float error;
            auto simplifiedIndices = Simplify(input, mergedIndices, locks, targetCount, &error);

            bool terminal = (simplifiedIndices.size() > mergedIndices.size() * SimplifyThreshold);

            // Compute bounds
            MClusterBounds bounds = terminal
                ? MClusterBounds{..., error = FLT_MAX}
                : BoundsCompute(input, simplifiedIndices, error);

            // Create new group (DAG version)
            MClusterGroup newGroup;
            newGroup.clusterOffset = m_allClusters.size();
            newGroup.clusterNum = partition.size();
            newGroup.bounds = bounds;
            newGroup.childIndicesOffset = 0;  // fill later
            newGroup.childCount = 0;          // fill later

            int32_t newGroupId = m_allGroups.size();

            // Record parent relationships for building child relationships later
            m_groupParents.push_back(std::vector<int32_t>(parentGroupIds.begin(), parentGroupIds.end()));

            // Add clusters to global array
            for (int clusterIdx : partition) {
                MCluster outputCluster;
                outputCluster.group = newGroupId;
                outputCluster.refined = clusters[clusterIdx].refined;
                // ... other fields
                m_allClusters.push_back(outputCluster);

                clusters[clusterIdx].group = newGroupId;
            }

            m_allGroups.push_back(newGroup);

            // If not terminal, re-Clusterize simplified result
            if (!terminal) {
                auto newClusters = Clusterize(input, simplifiedIndices);
                for (auto& c : newClusters) {
                    c.group = newGroupId;
                    c.refined = newGroupId;
                    int newIdx = clusters.size();
                    clusters.push_back(c);
                    newPending.push_back(newIdx);
                }
            }
        }

        // Record LOD info
        m_lods.push_back({lodGroupOffset, m_allGroups.size() - lodGroupOffset});

        pending = newPending;
        lodLevel++;
    }

    // Phase 3: Build child relationships
    BuildChildRelationships();

    // Phase 4: Output root group (if only one cluster remains)
    if (pending.size() == 1) {
        // ... output final root group
    }
}

void MClusterBuilder::BuildChildRelationships() {
    // For each group, based on its parent IDs, add itself to parent's children list

    // First collect each group's children
    std::vector<std::vector<int32_t>> childrenLists(m_allGroups.size());

    for (int32_t groupId = 0; groupId < m_allGroups.size(); ++groupId) {
        for (int32_t parentId : m_groupParents[groupId]) {
            if (parentId >= 0 && parentId < m_allGroups.size()) {
                childrenLists[parentId].push_back(groupId);
            }
        }
    }

    // Build global childIndices array
    m_groupChildIndices.clear();
    for (int32_t groupId = 0; groupId < m_allGroups.size(); ++groupId) {
        m_allGroups[groupId].childIndicesOffset = m_groupChildIndices.size();
        m_allGroups[groupId].childCount = childrenLists[groupId].size();

        for (int32_t childId : childrenLists[groupId]) {
            m_groupChildIndices.push_back(childId);
        }
    }
}
```

### Root Group Identification

Root groups are those without parents (i.e., groups from first iteration where all clusters have `group = -1`):

```cpp
std::vector<int32_t> MClusterBuilder::GetRootGroups() const {
    std::vector<int32_t> roots;
    for (int32_t i = 0; i < m_allGroups.size(); ++i) {
        if (m_groupParents[i].empty()) {
            roots.push_back(i);
        }
    }
    return roots;
}
```

**Note**: Groups produced in the first iteration (LOD 0) have clusters that all have `group = -1`, so these groups have empty parent lists and are root groups.

---

## GPU Shader Modification

```slang
// Child indices buffer
StructuredBuffer<uint> gChildIndices;

// Visited marking buffer (reset to 0 each frame)
RWBuffer<uint> gGroupVisited;

void TraverseLODTree_DAG(
    in MMeshRenderData meshRenderData,
    in MViewRenderData viewData
) {
    // Use shared memory as stack
    groupshared uint stack[STACK_SIZE];
    groupshared uint stackTop;

    // Initialize
    if (localIdx == 0) {
        stackTop = 0;
        // Add root groups to stack
        for (uint i = 0; i < meshRenderData.rootGroupCount; ++i) {
            stack[stackTop++] = meshRenderData.rootGroupBegin + i;
        }
    }
    GroupMemoryBarrierWithGroupSync();

    // BFS traversal
    while (true) {
        // Try to pop a group from stack
        uint groupIdx;
        bool hasWork = false;

        // ... atomic pop from stack ...

        if (!hasWork) break;

        // Atomic marking to avoid duplicate visits
        uint oldVisited;
        InterlockedExchange(gGroupVisited[groupIdx], 1, oldVisited);
        if (oldVisited == 1) continue;  // already visited

        MClusterGroupRenderData group = gClusterGroups[groupIdx];

        if (ShouldSubdivide(group, viewData)) {
            // Need finer LOD, traverse children
            for (uint i = 0; i < group.childCount; ++i) {
                uint childIdx = gChildIndices[group.childIndicesOffset + i];
                // Push to stack
                // ... atomic push to stack ...
            }
        } else {
            // Current LOD is sufficient, output clusters
            for (uint i = 0; i < group.clusterCount; ++i) {
                OutputVisibleCluster(group.clusterBeginIndex + i);
            }
        }
    }
}
```

---

## Data Structure Comparison

| Property | Current (Tree) | New Design (DAG) |
|----------|---------------|------------------|
| Parent Relationship | Single `parentGroupId` | Stored in build-time temp array (CPU only) |
| Child Relationship | `firstChildGroupId + childCount` (contiguous) | `childIndicesOffset + childCount` (index array) |
| GPU Children Access | Direct computation `firstChild + i` | Lookup `childIndices[offset + i]` |
| Duplicate Visit Handling | Not needed (tree has no duplicate paths) | Requires `visited` marking |
| Root Identification | `parentGroupId == INVALID` | Groups with empty parent list |

---

## Serialization Design (FlatBuffers)

### Current Serialization Structure

**MCluster.fbs** (current):
```fbs
table MClusterGroup
{
    cluster_offset: uint;
    cluster_num: uint;
    bounds: MClusterBounds;
    parent_group_id: int = -1;
    first_child_group_id: int = -1;
    child_group_count: uint = 0;
}
```

**MMeshResource.fbs** (current):
```fbs
table MMeshResource
{
    bounds_obb: MBoundsOBB;
    bounds_sphere: MBoundsSphere;
    vertex_type: MEMeshVertexType;
    vertex: [byte];
    index: [uint];
    cluster: [MCluster];
    group: [MClusterGroup];
    lod: [MSlice];
    pages: [MClusterPage];
}
```

### New Serialization Structure (DAG)

**MCluster.fbs** (modified):
```fbs
table MSlice
{
    offset: uint;
    num: uint;
}

table MClusterBounds
{
    position: Vector3;
    radius: float;
    error: float;
}

table MCluster
{
    group: int;
    refined: int;
    indices_offset: uint;
    indices_num: uint;
    bounds: MClusterBounds;
}

table MClusterPage
{
    vertex: [byte];
    index: [uint];
}

// Modified: MClusterGroup supports DAG
table MClusterGroup
{
    cluster_offset: uint;
    cluster_num: uint;
    bounds: MClusterBounds;

    // DAG structure: use offset + count to index into global array
    child_indices_offset: uint = 0;
    child_count: uint = 0;
}

file_identifier "MFBS";
```

**MMeshResource.fbs** (modified):
```fbs
include "Math/Vector.fbs";
include "Math/Quaternion.fbs";
include "Math/Matrix.fbs";
include "Utility/MBoundsOBB.fbs";
include "Utility/MBoundsSphere.fbs";
include "Model/MSkeleton.fbs";
include "Utility/MRenderGlobal.fbs";
include "Mesh/MCluster.fbs";

namespace morty.fbs;

enum MEMeshVertexType:byte
{
    normal = 0,
    skeleton = 1,
}

table MMeshResource
{
    bounds_obb: MBoundsOBB;
    bounds_sphere: MBoundsSphere;
    vertex_type: MEMeshVertexType;
    vertex: [byte];
    index: [uint];

    cluster: [MCluster];
    group: [MClusterGroup];
    lod: [MSlice];
    pages: [MClusterPage];

    // New: DAG child indices array
    group_child_indices: [int];
}

root_type MMeshResource;
file_identifier "MFBS";
```

### C++ Data Structure Modification

**MCluster.h** (modified):
```cpp
struct MClusterGroup {
    uint32_t       clusterOffset;
    uint32_t       clusterNum;
    MClusterBounds bounds;

    // DAG structure
    uint32_t       childIndicesOffset = 0;
    uint32_t       childCount = 0;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const void* pBufferPointer);
};
```

**MMesh.h** (modified):
```cpp
class MORTY_API MIMesh
{
public:
    // ... existing interface ...

    // New: DAG child indices array
    std::vector<int32_t>&       GetGroupChildIndices() { return m_groupChildIndices; }
    const std::vector<int32_t>& GetGroupChildIndices() const { return m_groupChildIndices; }

protected:
    // ... existing members ...
    std::vector<int32_t>     m_groupChildIndices;
};
```

### MCluster.cpp Serialization Implementation

```cpp
flatbuffers::Offset<void> MClusterGroup::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    auto fbBounds = bounds.Serialize(fbb);

    fbs::MClusterGroupBuilder builder(fbb);
    builder.add_cluster_offset(clusterOffset);
    builder.add_cluster_num(clusterNum);
    builder.add_bounds(fbBounds.o);
    builder.add_child_indices_offset(childIndicesOffset);
    builder.add_child_count(childCount);

    return builder.Finish().Union();
}

void MClusterGroup::Deserialize(const void* pBufferPointer)
{
    const auto* fbData = reinterpret_cast<const fbs::MClusterGroup*>(pBufferPointer);
    clusterOffset      = fbData->cluster_offset();
    clusterNum         = fbData->cluster_num();
    bounds.Deserialize(fbData->bounds());
    childIndicesOffset = fbData->child_indices_offset();
    childCount         = fbData->child_count();
}
```

### MMeshResource.cpp Serialization Implementation

**Serialize**:
```cpp
flatbuffers::Offset<void> MMeshResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    const auto fbObb    = boundsOBB.Serialize(fbb);
    const auto fbSphere = boundsSphere.Serialize(fbb);
    const auto fbVertex = fbb.CreateVector(mesh->GetVerticesVector());
    const auto fbIndex  = fbb.CreateVector(mesh->GetIndicesVector());

    // Clusters, Groups, LODs, Pages
    std::vector<flatbuffers::Offset<morty::fbs::MCluster>>      fbClusterArray(mesh->GetClusters().size());
    std::vector<flatbuffers::Offset<morty::fbs::MClusterGroup>> fbGroupArray(mesh->GetClusterGroup().size());
    std::vector<flatbuffers::Offset<morty::fbs::MSlice>>        fbLodArray(mesh->GetClusterLodData().size());
    std::vector<flatbuffers::Offset<morty::fbs::MClusterPage>>  fbPageArray(mesh->GetClusterPages().size());

    std::transform(mesh->GetClusters().begin(), mesh->GetClusters().end(), fbClusterArray.begin(),
        [&fbb](const auto& item) { return item.Serialize(fbb).o; });
    std::transform(mesh->GetClusterGroup().begin(), mesh->GetClusterGroup().end(), fbGroupArray.begin(),
        [&fbb](const auto& item) { return item.Serialize(fbb).o; });
    std::transform(mesh->GetClusterLodData().begin(), mesh->GetClusterLodData().end(), fbLodArray.begin(),
        [&fbb](const auto& item) { return item.Serialize(fbb).o; });
    std::transform(mesh->GetClusterPages().begin(), mesh->GetClusterPages().end(), fbPageArray.begin(),
        [&fbb](const auto& item) { return item.Serialize(fbb).o; });

    const auto fbClusters = fbb.CreateVector(fbClusterArray);
    const auto fbGroups   = fbb.CreateVector(fbGroupArray);
    const auto fbLods     = fbb.CreateVector(fbLodArray);
    const auto fbPages    = fbb.CreateVector(fbPageArray);

    // New: child indices array
    const auto fbChildIndices = fbb.CreateVector(mesh->GetGroupChildIndices());

    fbs::MMeshResourceBuilder builder(fbb);

    builder.add_bounds_obb(fbObb.o);
    builder.add_bounds_sphere(fbSphere.o);
    builder.add_vertex_type(static_cast<fbs::MEMeshVertexType>(eVertexType));
    builder.add_vertex(fbVertex.o);
    builder.add_index(fbIndex.o);
    builder.add_cluster(fbClusters.o);
    builder.add_group(fbGroups.o);
    builder.add_lod(fbLods.o);
    builder.add_pages(fbPages.o);
    builder.add_group_child_indices(fbChildIndices.o);

    return builder.Finish().Union();
}
```

**Deserialize**:
```cpp
void MMeshResourceData::Deserialize(const void* pBufferPointer)
{
    const fbs::MMeshResource* fbData = fbs::GetMMeshResource(pBufferPointer);

    eVertexType = static_cast<MEMeshVertexType>(fbData->vertex_type());
    mesh        = MMeshUtil::CreateMeshFromType(eVertexType);

    // Vertex and Index data
    const auto nVertexNum = static_cast<uint32_t>(fbData->vertex()->size() / mesh->GetVertexStructSize());
    mesh->ResizeVertices(nVertexNum);
    memcpy(mesh->GetVertices(), fbData->vertex()->data(), nVertexNum * mesh->GetVertexStructSize());

    const auto nIndexNum = static_cast<uint32_t>(fbData->index()->size() / sizeof(uint32_t));
    mesh->ResizeIndices(nIndexNum, 1);
    memcpy(mesh->GetIndices(), fbData->index()->data(), nIndexNum * sizeof(uint32_t));

    boundsOBB.Deserialize(fbData->bounds_obb());
    boundsSphere.Deserialize(fbData->bounds_sphere());

    // Clusters, Groups, LODs, Pages
    std::vector<MCluster> clusters(fbData->cluster()->size());
    std::transform(fbData->cluster()->begin(), fbData->cluster()->end(), clusters.begin(),
        [](const auto& item) { MCluster c; c.Deserialize(item); return c; });

    std::vector<MClusterGroup> groups(fbData->group()->size());
    std::transform(fbData->group()->begin(), fbData->group()->end(), groups.begin(),
        [](const auto& item) { MClusterGroup g; g.Deserialize(item); return g; });

    std::vector<MClusterLodData> lods(fbData->lod()->size());
    std::transform(fbData->lod()->begin(), fbData->lod()->end(), lods.begin(),
        [](const auto& item) { MClusterLodData l{}; l.Deserialize(item); return l; });

    std::vector<MClusterPage> pages(fbData->pages()->size());
    std::transform(fbData->pages()->begin(), fbData->pages()->end(), pages.begin(),
        [](const auto& item) { MClusterPage p{}; p.Deserialize(item); return p; });

    mesh->GetClusters()       = std::move(clusters);
    mesh->GetClusterGroup()   = std::move(groups);
    mesh->GetClusterLodData() = std::move(lods);
    mesh->GetClusterPages()   = std::move(pages);

    // New: child indices array
    if (fbData->group_child_indices())
    {
        mesh->GetGroupChildIndices().assign(
            fbData->group_child_indices()->begin(),
            fbData->group_child_indices()->end()
        );
    }
}
```

### MClusterBuilder Output Modification

```cpp
void MClusterBuilder::Generate(MIMesh* mesh)
{
    // ... existing code ...

    // Output to mesh
    mesh->GetClusters()       = std::move(m_allClusters);
    mesh->GetClusterGroup()   = std::move(m_allGroups);
    mesh->GetClusterLodData() = std::move(m_lods);
    mesh->GetClusterPages()   = std::move(m_clusterPages);

    // New: output child indices array
    mesh->GetGroupChildIndices() = std::move(m_groupChildIndices);
}
```

---

## Implementation Tasks

1. **Modify MCluster.fbs**: Update `MClusterGroup` table structure
2. **Modify MMeshResource.fbs**: Add `group_child_indices` field
3. **Modify MCluster.h**: Update `MClusterGroup` struct
4. **Modify MCluster.cpp**: Update serialization/deserialization implementation
5. **Modify MMesh.h**: Add `m_groupChildIndices` member
6. **Modify MMeshResource.cpp**: Update serialization/deserialization implementation
7. **Modify MClusterBuilder**: Implement DAG build algorithm and `BuildChildRelationships()`
8. **Modify GPU Buffers**: Add `childIndices` and `visited` buffers
9. **Modify Shader**: Implement DAG traversal logic
10. **Regenerate FlatBuffers**: Run `flatc` to generate new `_generated.h` files

---

## Configuration Parameters

```cpp
const size_t MaxVertices  = 64;       // max vertices per cluster
const size_t MaxTriangles = 128;      // max triangles per cluster
const size_t PartitionSize = 24;      // target partition size

const float SimplifyRatio = 0.5f;         // simplification target ratio (50%)
const float SimplifyThreshold = 0.85f;    // simplification failure threshold (if only simplified <15%, consider failed)
```
