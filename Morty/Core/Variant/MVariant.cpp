#include "MVariant.h"

#include "MVariant_generated.h"

using namespace morty;

const MVariant& MVariant::operator=(const MVariant& other)
{
    m_memory = other.m_memory;
    m_struct = other.m_struct;
    m_array  = other.m_array;

    m_offset = other.m_offset;
    m_size   = other.m_size;
    m_type   = other.m_type;

    return *this;
}

void MVariant::operator=(MVariant&& other)
{
    m_memory = other.m_memory;
    m_struct = other.m_struct;
    m_array  = other.m_array;

    m_offset = other.m_offset;
    m_size   = other.m_size;
    m_type   = other.m_type;
}

void MVariant::ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset)
{
    if (MEVariantType::EStruct == GetType())
    {
        MORTY_ASSERT(m_struct);
        m_struct->ResetMemory(pMemory, nOffset);
    }
    else if (MEVariantType::EArray == GetType())
    {
        MORTY_ASSERT(m_array);
        m_array->ResetMemory(pMemory, nOffset);
    }


    m_memory = pMemory;
    m_offset = nOffset;
}

fbs::MVariantData EnumVariantToFb(MEVariantType eType)
{
    const static std::map<MEVariantType, fbs::MVariantData> Table = {
            {MEVariantType::EUInt, fbs::MVariantData::UInt},
            {MEVariantType::EFloat, fbs::MVariantData::Float},
            {MEVariantType::EInt, fbs::MVariantData::Int},
            {MEVariantType::EMatrix3, fbs::MVariantData::Matrix3},
            {MEVariantType::EMatrix4, fbs::MVariantData::Matrix4},
            {MEVariantType::EVector2, fbs::MVariantData::Vector2},
            {MEVariantType::EVector3, fbs::MVariantData::Vector3},
            {MEVariantType::EVector4, fbs::MVariantData::Vector4},
            {MEVariantType::EStruct, fbs::MVariantData::MVariantStruct},
            {MEVariantType::EArray, fbs::MVariantData::MVariantArray},
            {MEVariantType::ENone, fbs::MVariantData::NONE},
    };

    return Table.at(eType);
}

MEVariantType EnumVariantToFb(fbs::MVariantData eType)
{
    const static std::map<fbs::MVariantData, MEVariantType> Table = {
            {fbs::MVariantData::UInt, MEVariantType::EUInt},
            {fbs::MVariantData::Float, MEVariantType::EFloat},
            {fbs::MVariantData::Int, MEVariantType::EInt},
            {fbs::MVariantData::Matrix3, MEVariantType::EMatrix3},
            {fbs::MVariantData::Matrix4, MEVariantType::EMatrix4},
            {fbs::MVariantData::Vector2, MEVariantType::EVector2},
            {fbs::MVariantData::Vector3, MEVariantType::EVector3},
            {fbs::MVariantData::Vector4, MEVariantType::EVector4},
            {fbs::MVariantData::MVariantStruct, MEVariantType::EStruct},
            {fbs::MVariantData::MVariantArray, MEVariantType::EArray},
            {fbs::MVariantData::NONE, MEVariantType::ENone},
    };

    return Table.at(eType);
}

flatbuffers::Offset<void> MVariant::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    flatbuffers::Offset<void> fbVariantData;
    const fbs::MVariantData   fbVariantType = EnumVariantToFb(GetType());
    MORTY_ASSERT(fbVariantType != fbs::MVariantData::NONE);

    if (GetType() == MEVariantType::EUInt)
    {
        fbVariantData = fbb.CreateStruct(fbs::UInt(GetValue<uint32_t>())).Union();
    }
    else if (GetType() == MEVariantType::EFloat)
    {
        fbVariantData = fbb.CreateStruct(fbs::Float(GetValue<float>())).Union();
    }
    else if (GetType() == MEVariantType::EInt)
    {
        fbVariantData = fbb.CreateStruct(fbs::Int(GetValue<int>())).Union();
    }
    else if (GetType() == MEVariantType::EMatrix3)
    {
        fbVariantData = fbb.CreateStruct(*GetValue<Matrix3>().Serialize(fbb)).Union();
    }
    else if (GetType() == MEVariantType::EMatrix4)
    {
        fbVariantData = fbb.CreateStruct(*GetValue<Matrix4>().Serialize(fbb)).Union();
    }
    else if (GetType() == MEVariantType::EVector2)
    {
        fbVariantData = fbb.CreateStruct(*GetValue<Vector2>().Serialize(fbb)).Union();
    }
    else if (GetType() == MEVariantType::EVector3)
    {
        fbVariantData = fbb.CreateStruct(*GetValue<Vector3>().Serialize(fbb)).Union();
    }
    else if (GetType() == MEVariantType::EVector4)
    {
        fbVariantData = fbb.CreateStruct(*GetValue<Vector4>().Serialize(fbb)).Union();
    }
    else if (GetType() == MEVariantType::EStruct)
    {
        fbVariantData = GetValue<MVariantStruct>().Serialize(fbb);
    }
    else if (GetType() == MEVariantType::EArray)
    {
        fbVariantData = GetValue<MVariantArray>().Serialize(fbb);
    }
    else { MORTY_ASSERT(false); }

    fbs::MVariantBuilder builder(fbb);
    builder.add_data(fbVariantData);
    builder.add_data_type(fbVariantType);
    return builder.Finish().Union();
}

void MVariant::Deserialize(
        const void*                     pBufferPointer,
        std::shared_ptr<MVariantMemory> pMemory,
        size_t                          nOffset
)
{
    const fbs::MVariant* fbData = reinterpret_cast<const fbs::MVariant*>(pBufferPointer);

    m_offset = nOffset;
    m_type   = EnumVariantToFb(fbData->data_type());

    if (GetType() == MEVariantType::EStruct)
    {
        const fbs::MVariantStruct* fbStruct =
                reinterpret_cast<const fbs::MVariantStruct*>(
                        fbData->data_as_MVariantStruct()
                );
        m_size = fbStruct->size();
    }
    else if (GetType() == MEVariantType::EArray)
    {
        const fbs::MVariantArray* fbArray = reinterpret_cast<const fbs::MVariantArray*>(
                fbData->data_as_MVariantStruct()
        );
        m_size = fbArray->size();
    }
    else { m_size = TypeSize(m_type); }

    if (!pMemory)
    {
        pMemory = std::make_shared<MVariantMemory>();
        pMemory->AllocMemory(m_size);
    }

    m_memory = pMemory;

    if (GetType() == MEVariantType::EUInt) { SetValue(fbData->data_as_UInt()->value()); }
    else if (GetType() == MEVariantType::EFloat)
    {
        SetValue(fbData->data_as_Float()->value());
    }
    else if (GetType() == MEVariantType::EInt)
    {
        SetValue(fbData->data_as_Int()->value());
    }
    else if (GetType() == MEVariantType::EMatrix3)
    {
        GetValue<Matrix3>().Deserialize(fbData->data_as_Matrix3());
    }
    else if (GetType() == MEVariantType::EMatrix4)
    {
        GetValue<Matrix4>().Deserialize(fbData->data_as_Matrix4());
    }
    else if (GetType() == MEVariantType::EVector2)
    {
        GetValue<Vector2>().Deserialize(fbData->data_as_Vector2());
    }
    else if (GetType() == MEVariantType::EVector3)
    {
        GetValue<Vector3>().Deserialize(fbData->data_as_Vector3());
    }
    else if (GetType() == MEVariantType::EVector4)
    {
        GetValue<Vector4>().Deserialize(fbData->data_as_Vector4());
    }
    else if (GetType() == MEVariantType::EStruct)
    {
        m_struct = std::make_shared<MVariantStruct>();
        m_struct->Deserialize(fbData->data_as_MVariantStruct(), pMemory, m_offset);
    }
    else if (GetType() == MEVariantType::EArray)
    {
        m_array = std::make_shared<MVariantArray>();
        m_array->Deserialize(fbData->data_as_MVariantArray(), pMemory, m_offset);
    }
    else { MORTY_ASSERT(false); }
}

size_t MVariant::TypeSize(MEVariantType eType)
{
    MORTY_ASSERT(MEVariantType::ENone != eType);
    MORTY_ASSERT(MEVariantType::EArray != eType);
    MORTY_ASSERT(MEVariantType::EStruct != eType);

    static std::map<MEVariantType, size_t> VariantTypeSize = {
            {MEVariantType::ENone, 0},
            {MEVariantType::EUInt, sizeof(int)},
            {MEVariantType::EInt, sizeof(int)},
            {MEVariantType::EFloat, sizeof(float)},
            {MEVariantType::EVector2, sizeof(Vector2)},
            {MEVariantType::EVector3, sizeof(Vector3)},
            {MEVariantType::EVector4, sizeof(Vector4)},
            {MEVariantType::EMatrix3, sizeof(float) * 4 * 3},
            {MEVariantType::EMatrix4, sizeof(float) * 4 * 4},
            {MEVariantType::EStruct, 0},
            {MEVariantType::EArray, 0},
    };

    return VariantTypeSize[eType];
}

size_t MVariant::GetOffset() const { return m_offset; }

size_t MVariant::GetSize() const
{
    if (m_struct) { return m_struct->Size(); }
    if (m_array) { return m_array->Size(); }

    return TypeSize(m_type);
}

MEVariantType MVariant::GetType() const { return m_type; }

MByte*        MVariant::GetData() const
{
    if (m_struct) { return m_struct->Data(); }
    if (m_array) { return m_array->Data(); }

    return m_memory->Data() + m_offset;
}

MVariant::MVariant(
        const std::shared_ptr<MVariantMemory>& pMemory,
        size_t                                 nOffset,
        size_t                                 nSize,
        MEVariantType                          eType
)
    : m_offset(nOffset)
    , m_size(nSize)
    , m_type(eType)
    , m_memory(pMemory)
{}

MVariantStruct::MVariantStruct()
    : m_memory(std::make_shared<MVariantMemory>())
    , m_offset(0)
    , m_size(0)
    , m_member()
{}

bool MVariantStruct::HasVariant(const MStringId& strName)
{
    MORTY_ASSERT(m_locked);
    return m_member.find(strName) != m_member.end();
}

MVariant& MVariantStruct::FindVariant(const MStringId& strName)
{
    MORTY_ASSERT(m_locked);
    auto findResult = m_member.find(strName);
    if (findResult == m_member.end())
    {
        static MVariant InvalidValue;
        return InvalidValue;
    }

    return findResult->second;
}

void MVariantStruct::ResetMemory(
        const std::shared_ptr<MVariantMemory>& pMemory,
        size_t                                 nOffset
)
{
    MORTY_ASSERT(m_locked);
    for (auto& pr: m_member)
    {
        MVariant& member = pr.second;
        member.ResetMemory(pMemory, member.GetOffset() - m_offset + nOffset);
    }
    m_memory = pMemory;
    m_offset = nOffset;
}

MVariantArray::MVariantArray()
    : m_memory(std::make_shared<MVariantMemory>())
    , m_offset(0)
    , m_size(0)
    , m_member()
{}

MVariant& MVariantArray::operator[](const size_t& nIdx)
{
    MORTY_ASSERT(m_locked);
    if (nIdx >= m_member.size())
    {
        MORTY_ASSERT(nIdx < m_member.size());
        static MVariant InvalidValue;
        return InvalidValue;
    }

    return m_member[nIdx];
}

void MVariantArray::ResetMemory(
        const std::shared_ptr<MVariantMemory>& pMemory,
        size_t                                 nOffset
)
{
    MORTY_ASSERT(m_locked);
    for (MVariant& member: m_member)
    {
        member.ResetMemory(pMemory, member.GetOffset() - m_offset + nOffset);
    }
    m_memory = pMemory;
    m_offset = nOffset;
}

MVariant MVariant::Clone(
        const MVariant&                 source,
        std::shared_ptr<MVariantMemory> pMemory,
        size_t                          nOffset
)
{
    if (!pMemory)
    {
        pMemory = std::make_shared<MVariantMemory>();
        pMemory->AllocMemory(source.GetSize());

        memcpy(pMemory->Data(), source.GetData(), source.GetSize());
    }

    MVariant target;

    if (source.GetType() == MEVariantType::EStruct)
    {
        MORTY_ASSERT(source.m_struct);

        target.m_struct = MVariantStruct::Clone(source.m_struct, pMemory, nOffset);
        target.m_type   = MEVariantType::EStruct;
        target.m_offset = nOffset;
        target.m_size   = target.m_struct->Size();
        target.m_memory = pMemory;
    }
    else if (source.GetType() == MEVariantType::EArray)
    {
        MORTY_ASSERT(source.m_array);

        target.m_array  = MVariantArray::Clone(source.m_array, pMemory, nOffset);
        target.m_type   = MEVariantType::EArray;
        target.m_offset = nOffset;
        target.m_size   = target.m_array->Size();
        target.m_memory = pMemory;
    }
    else { target = MVariant(pMemory, nOffset, source.m_size, source.GetType()); }

    return target;
}

std::shared_ptr<MVariantStruct> MVariantStruct::Clone(
        const std::shared_ptr<MVariantStruct>& pSource,
        std::shared_ptr<MVariantMemory>        pMemory,
        size_t                                 nOffset
)
{
    MORTY_ASSERT(pSource->m_locked);
    std::shared_ptr<MVariantStruct> pTarget = std::make_shared<MVariantStruct>();

    pTarget->m_memory = pMemory;
    pTarget->m_offset = nOffset;
    pTarget->m_size   = pSource->Size();
    pTarget->m_locked = true;

    for (auto& pr: pSource->GetMember())
    {
        pTarget->m_member[pr.first] = MVariant::Clone(
                pr.second,
                pMemory,
                pr.second.GetOffset() - pSource->m_offset + nOffset
        );
    }

    return pTarget;
}

std::shared_ptr<MVariantArray> MVariantArray::Clone(
        const std::shared_ptr<MVariantArray>& pSource,
        std::shared_ptr<MVariantMemory>       pMemory,
        size_t                                nOffset
)
{
    MORTY_ASSERT(pSource->m_locked);
    std::shared_ptr<MVariantArray> pTarget = std::make_shared<MVariantArray>();

    pTarget->m_memory = pMemory;
    pTarget->m_offset = nOffset;
    pTarget->m_size   = pSource->Size();
    pTarget->m_locked = true;

    for (auto& pr: pSource->GetMember())
    {
        pTarget->m_member.push_back(
                MVariant::Clone(pr, pMemory, pr.GetOffset() - pSource->m_offset + nOffset)
        );
    }

    return pTarget;
}

flatbuffers::Offset<void> MVariantStruct::Serialize(flatbuffers::FlatBufferBuilder& fbb
) const
{
    std::vector<flatbuffers::Offset<fbs::MStructMember>> vMembers;

    for (auto& pr: m_member)
    {
        const auto                fbName    = fbb.CreateString(pr.first.ToString());
        const auto                fbVariant = pr.second.Serialize(fbb);

        fbs::MStructMemberBuilder builder(fbb);
        builder.add_name(fbName);
        builder.add_value(fbVariant.o);
        builder.add_relative_offset(
                static_cast<uint32_t>(pr.second.GetOffset() - m_offset)
        );
        vMembers.push_back(builder.Finish());
    }

    const auto                 fbMembers = fbb.CreateVector(vMembers);
    fbs::MVariantStructBuilder builder(fbb);
    builder.add_member(fbMembers);
    builder.add_size(static_cast<uint32_t>(Size()));
    return builder.Finish().Union();
}

void MVariantStruct::Deserialize(
        const void*                     pBufferPointer,
        std::shared_ptr<MVariantMemory> pMemory,
        size_t                          nOffset
)
{
    MORTY_ASSERT(pMemory);

    m_memory = pMemory;
    m_offset = nOffset;

    const fbs::MVariantStruct* fbData =
            reinterpret_cast<const fbs::MVariantStruct*>(pBufferPointer);
    m_size = fbData->size();

    if (fbData->member())
    {
        for (const auto& fbMember: *fbData->member())
        {
            //const auto fbVariant = reinterpret_cast<const fbs::MVariant*>(fbMember->value());

            const size_t    nOffset = fbMember->relative_offset() + m_offset;
            const MStringId strName = MStringId(fbMember->name()->c_str());

            MVariant        variant(m_memory, nOffset, 0, MEVariantType::ENone);
            variant.Deserialize(fbMember->value(), pMemory, nOffset);

            m_member[strName] = std::move(variant);
        }
    }

    m_locked = true;
}

flatbuffers::Offset<void> MVariantArray::Serialize(flatbuffers::FlatBufferBuilder& fbb
) const
{
    std::vector<flatbuffers::Offset<fbs::MArrayMember>> vMembers;

    for (auto& value: m_member)
    {
        const auto               fbVariant = value.Serialize(fbb);

        fbs::MArrayMemberBuilder builder(fbb);
        builder.add_value(fbVariant.o);
        builder.add_relative_offset(static_cast<uint32_t>(value.GetOffset() - m_offset));
        vMembers.push_back(builder.Finish());
    }

    const auto                fbMembers = fbb.CreateVector(vMembers);
    fbs::MVariantArrayBuilder builder(fbb);
    builder.add_member(fbMembers);
    builder.add_size(static_cast<uint32_t>(Size()));
    return builder.Finish().Union();
}

void MVariantArray::Deserialize(
        const void*                     pBufferPointer,
        std::shared_ptr<MVariantMemory> pMemory,
        size_t                          nOffset
)
{
    MORTY_ASSERT(pMemory);

    m_memory = pMemory;
    m_offset = nOffset;

    const fbs::MVariantArray* fbData =
            reinterpret_cast<const fbs::MVariantArray*>(pBufferPointer);
    m_size = fbData->size();

    if (fbData->member())
    {
        for (const auto& fbMember: *fbData->member())
        {
            //const auto fbVariant = reinterpret_cast<const fbs::MVariant*>(fbMember->value());

            const size_t nOffset = fbMember->relative_offset() + m_offset;

            MVariant     variant(m_memory, nOffset, 0, MEVariantType::ENone);
            variant.Deserialize(fbMember->value(), pMemory, nOffset);

            m_member.emplace_back(std::move(variant));
        }
    }

    m_locked = true;
}


TEST_CASE("variant complex struct test")
{
    /* root(struct)								offset: 0
        --node_1(struct)						offset: 0
            --add_offset(float)					offset: 0
            --node_2(array)						offset: 16
                --node_3_1(struct)				offset: 16
                    --node_4_1(float)			offset: 16
                    --node_4_2(vector2)			offset: 20
                    --node_4_3(vector3)			offset: 32
                --node_3_2(struct)				offset: 48
                    --node_4_1(float)			offset: 48
                    --node_4_2(vector2)			offset: 52
                    --node_4_3(vector3)			offset: 64
    */

    MStringId             name_node_1("node_1");
    MStringId             name_node_2("node_2");
    MStringId             name_node_4_1("node_4_1");
    MStringId             name_node_4_2("node_4_2");
    MStringId             name_node_4_3("node_4_3");

    MVariantStruct        node_3;
    MVariantStructBuilder node_3_builder(node_3);
    node_3_builder.AppendVariant(name_node_4_1, 1.0f);
    node_3_builder.AppendVariant(name_node_4_2, Vector2(2.0f, 3.0f));
    node_3_builder.AppendVariant(name_node_4_3, Vector3(4.0f, 5.0f, 6.0f));
    node_3_builder.Finish();

    MVariantArray        node_2;
    MVariantArrayBuilder node_2_builder(node_2);
    node_2_builder.AppendVariant(node_3);
    node_2_builder.AppendVariant(node_3);
    node_2_builder.Finish();

    MVariantStruct        node_1;
    MVariantStructBuilder node_1_builder(node_1);
    node_1_builder.AppendVariant(name_node_2, node_2);
    node_1_builder.Finish();

    MVariantStruct        root;
    MVariantStructBuilder root_builder(root);
    root_builder.AppendVariant(MStringId("add_offset"), 7.0f);
    root_builder.AppendVariant(name_node_1, node_1);
    root_builder.Finish();

    MVariant              rootVariant(root);

    std::vector<MVariant> vTestGroup = {rootVariant, MVariant::Clone(rootVariant)};

    for (size_t nIdx = 0; nIdx < vTestGroup.size(); ++nIdx)
    {
        MVariant root       = vTestGroup[nIdx];
        auto&    rootStruct = root.GetValue<MVariantStruct>();

        MByte*   rootData = root.GetData();

        CHECK(rootStruct.Offset() == 0);
        CHECK(rootStruct.Size() == 80);
        CHECK(rootStruct.Data() - rootStruct.Offset() == rootData);

        auto& get_node_1 = rootStruct.FindVariant(name_node_1).GetValue<MVariantStruct>();
        CHECK(get_node_1.Offset() == 16);
        CHECK(get_node_1.Size() == 64);
        CHECK(get_node_1.Data() - get_node_1.Offset() == rootData);

        auto& get_node_2 = get_node_1.FindVariant(name_node_2).GetValue<MVariantArray>();
        CHECK(get_node_2.Offset() == 16);
        CHECK(get_node_2.Size() == 64);
        CHECK(get_node_2.Data() - get_node_2.Offset() == rootData);

        auto& get_node_3_1 = get_node_2[0].GetValue<MVariantStruct>();
        CHECK(get_node_3_1.Offset() == 16);
        CHECK(get_node_3_1.Size() == 28);
        CHECK(get_node_3_1.Data() - get_node_3_1.Offset() == rootData);

        auto& get_node_3_1_4_1 = get_node_3_1.FindVariant(name_node_4_1);
        CHECK(get_node_3_1_4_1.GetOffset() == 16);
        CHECK(get_node_3_1_4_1.GetSize() == 4);
        CHECK(get_node_3_1_4_1.GetData() - get_node_3_1_4_1.GetOffset() == rootData);


        auto& get_node_3_1_4_2 = get_node_3_1.FindVariant(name_node_4_2);
        CHECK(get_node_3_1_4_2.GetOffset() == 20);
        CHECK(get_node_3_1_4_2.GetSize() == 8);
        CHECK(get_node_3_1_4_2.GetData() - get_node_3_1_4_2.GetOffset() == rootData);

        auto& get_node_3_1_4_3 = get_node_3_1.FindVariant(name_node_4_3);
        CHECK(get_node_3_1_4_3.GetOffset() == 32);
        CHECK(get_node_3_1_4_3.GetSize() == 12);
        CHECK(get_node_3_1_4_3.GetData() - get_node_3_1_4_3.GetOffset() == rootData);


        auto& get_node_3_2 = get_node_2[1].GetValue<MVariantStruct>();
        CHECK(get_node_3_2.Offset() == 48);
        CHECK(get_node_3_2.Size() == 28);
        CHECK(get_node_3_2.Data() - get_node_3_2.Offset() == rootData);

        auto& get_node_3_2_4_1 = get_node_3_2.FindVariant(name_node_4_1);
        CHECK(get_node_3_2_4_1.GetOffset() == 48);
        CHECK(get_node_3_2_4_1.GetSize() == 4);
        CHECK(get_node_3_2_4_1.GetData() - get_node_3_2_4_1.GetOffset() == rootData);


        auto& get_node_3_2_4_2 = get_node_3_2.FindVariant(name_node_4_2);
        CHECK(get_node_3_2_4_2.GetOffset() == 52);
        CHECK(get_node_3_2_4_2.GetSize() == 8);
        CHECK(get_node_3_2_4_2.GetData() - get_node_3_2_4_2.GetOffset() == rootData);

        auto& get_node_3_2_4_3 = get_node_3_2.FindVariant(name_node_4_3);
        CHECK(get_node_3_2_4_3.GetOffset() == 64);
        CHECK(get_node_3_2_4_3.GetSize() == 12);
        CHECK(get_node_3_2_4_3.GetData() - get_node_3_2_4_3.GetOffset() == rootData);
    }
}
