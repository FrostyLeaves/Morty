#include "MVariant.h"

#include "MVariant_generated.h"

const MVariant& MVariant::operator=(const MVariant& other)
{
	m_pMemory = other.m_pMemory;
	m_pStruct = other.m_pStruct;
	m_pArray = other.m_pArray;

	m_nOffset = other.m_nOffset;
	m_nSize = other.m_nSize;
	m_eType = other.m_eType;

	return *this;
}

void MVariant::operator=(MVariant&& other)
{
	m_pMemory = other.m_pMemory;
	m_pStruct = other.m_pStruct;
	m_pArray = other.m_pArray;

	m_nOffset = other.m_nOffset;
	m_nSize = other.m_nSize;
	m_eType = other.m_eType;
}

void MVariant::ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset)
{
	if (MEVariantType::EStruct == GetType())
	{
		if (m_pStruct)
		{
			m_pStruct->ResetMemory(pMemory, nOffset);
		}
	}
	else if (MEVariantType::EArray == GetType())
	{
		m_pArray->ResetMemory(pMemory, nOffset);
	}
	else
	{
		m_pMemory = pMemory;
		m_nOffset = nOffset;
	}
}

mfbs::MVariantData EnumVariantToFb(MEVariantType eType)
{
	const static std::map<MEVariantType, mfbs::MVariantData> Table = {
		{MEVariantType::EBool, mfbs::MVariantData_Bool},
		{MEVariantType::EFloat, mfbs::MVariantData_Float},
		{MEVariantType::EInt, mfbs::MVariantData_Int},
		{MEVariantType::EMatrix3, mfbs::MVariantData_Matrix3},
		{MEVariantType::EMatrix4, mfbs::MVariantData_Matrix4},
		{MEVariantType::EVector2, mfbs::MVariantData_Vector2},
		{MEVariantType::EVector3, mfbs::MVariantData_Vector3},
		{MEVariantType::EVector4, mfbs::MVariantData_Vector4},
		{MEVariantType::EStruct, mfbs::MVariantData_MVariantStruct},
		{MEVariantType::EArray, mfbs::MVariantData_MVariantArray},
		{MEVariantType::ENone, mfbs::MVariantData_NONE},
	};
	
	return Table.at(eType);
}

MEVariantType EnumVariantToFb(mfbs::MVariantData eType)
{
	const static std::map<mfbs::MVariantData, MEVariantType> Table = {
		{mfbs::MVariantData_Bool,MEVariantType::EBool},
		{mfbs::MVariantData_Float, MEVariantType::EFloat},
		{mfbs::MVariantData_Int, MEVariantType::EInt},
		{mfbs::MVariantData_Matrix3, MEVariantType::EMatrix3},
		{mfbs::MVariantData_Matrix4, MEVariantType::EMatrix4},
		{mfbs::MVariantData_Vector2, MEVariantType::EVector2},
		{mfbs::MVariantData_Vector3, MEVariantType::EVector3},
		{mfbs::MVariantData_Vector4, MEVariantType::EVector4},
		{mfbs::MVariantData_MVariantStruct, MEVariantType::EStruct},
		{mfbs::MVariantData_MVariantArray, MEVariantType::EArray},
		{mfbs::MVariantData_NONE, MEVariantType::ENone},
	};

	return Table.at(eType);
}

flatbuffers::Offset<void> MVariant::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	flatbuffers::Offset<void> fbVariantData;
	const mfbs::MVariantData fbVariantType = EnumVariantToFb(GetType());
	MORTY_ASSERT(fbVariantType != mfbs::MVariantData_NONE);

	if (GetType() == MEVariantType::EBool)
	{
		fbVariantData = fbb.CreateStruct(mfbs::Bool(GetValue<bool>())).Union();
	}
	else if(GetType() == MEVariantType::EFloat)
	{
		fbVariantData = fbb.CreateStruct(mfbs::Float(GetValue<float>())).Union();
	}
	else if (GetType() == MEVariantType::EInt)
	{
		fbVariantData = fbb.CreateStruct(mfbs::Int(GetValue<int>())).Union();
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
	else if(GetType() == MEVariantType::EArray)
	{
		fbVariantData = GetValue<MVariantArray>().Serialize(fbb);
	}
	else
	{
		MORTY_ASSERT(false);
	}

	mfbs::MVariantBuilder builder(fbb);
	builder.add_data(fbVariantData);
	builder.add_data_type(fbVariantType);
	return builder.Finish().Union();
}

void MVariant::Deserialize(const void* pBufferPointer, std::shared_ptr<MVariantMemory> pMemory, size_t nOffset)
{
	const mfbs::MVariant* fbData = reinterpret_cast<const mfbs::MVariant*>(pBufferPointer);

	m_nOffset = nOffset;
	m_eType = EnumVariantToFb(fbData->data_type());

	if (GetType() == MEVariantType::EStruct)
	{
		const mfbs::MVariantStruct* fbStruct = reinterpret_cast<const mfbs::MVariantStruct*>(fbData->data_as_MVariantStruct());
		m_nSize = fbStruct->size();
	}
	else if(GetType() == MEVariantType::EArray)
	{
		const mfbs::MVariantArray* fbArray = reinterpret_cast<const mfbs::MVariantArray*>(fbData->data_as_MVariantStruct());
		m_nSize = fbArray->size();
	}
	else
	{
		m_nSize = TypeSize(m_eType);
	}

	if (!pMemory)
	{
		pMemory = std::make_shared<MVariantMemory>();
		pMemory->AllocMemory(m_nSize);
	}

	m_pMemory = pMemory;

	if (GetType() == MEVariantType::EBool)
	{
		SetValue(fbData->data_as_Bool()->value());
	}
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
		m_pStruct = std::make_shared<MVariantStruct>();
		m_pStruct->Deserialize(fbData->data_as_MVariantStruct(), pMemory, m_nOffset);
	}
	else if (GetType() == MEVariantType::EArray)
	{
		m_pArray = std::make_shared<MVariantArray>();
		m_pArray->Deserialize(fbData->data_as_MVariantArray(), pMemory, m_nOffset);
	}
	else
	{
		MORTY_ASSERT(false);
	}

}

size_t MVariant::TypeSize(MEVariantType eType)
{
	MORTY_ASSERT(MEVariantType::ENone != eType);
	MORTY_ASSERT(MEVariantType::EArray != eType);
	MORTY_ASSERT(MEVariantType::EStruct != eType);

	static std::map<MEVariantType, size_t> VariantTypeSize = {
		{MEVariantType::ENone, 0},
		{MEVariantType::EBool, sizeof(int)},
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

size_t MVariant::GetOffset() const
{
	return m_nOffset;
}

size_t MVariant::GetSize() const
{
	if (m_pStruct)
	{
		return m_pStruct->Size();
	}
	if (m_pArray)
	{
		return m_pArray->Size();
	}

	return TypeSize(m_eType);
}

MEVariantType MVariant::GetType() const
{
	return m_eType;
}

MByte* MVariant::GetData() const
{
	if(m_pStruct)
	{
		return m_pStruct->Data();
	}
	if (m_pArray)
	{
		return m_pArray->Data();
	}

	return m_pMemory->Data() + m_nOffset;
}

MVariant::MVariant(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset, size_t nSize, MEVariantType eType)
	: m_pMemory(pMemory)
	, m_nOffset(nOffset)
	, m_nSize(nSize)
	, m_eType(eType)
{
}

MVariantStruct::MVariantStruct()
	: m_pMemory(std::make_shared<MVariantMemory>())
	, m_nOffset(0)
	, m_nSize(0)
	, m_tMember()
{

}

bool MVariantStruct::HasVariant(const MString& strName)
{
	MORTY_ASSERT(m_bLocked);
	return m_tMember.find(strName) != m_tMember.end();
}

MVariant& MVariantStruct::FindVariant(const MString& strName)
{
	MORTY_ASSERT(m_bLocked);
	auto findResult = m_tMember.find(strName);
	if (findResult == m_tMember.end())
	{
		static MVariant InvalidValue;
		return InvalidValue;
	}

	return findResult->second;
}

void MVariantStruct::ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset)
{
	MORTY_ASSERT(m_bLocked);
	for (auto& pr : m_tMember)
	{
		MVariant& member = pr.second;
		member.ResetMemory(pMemory, member.GetOffset() - m_nOffset + nOffset);
	}
	m_nOffset = nOffset;
}

MVariantArray::MVariantArray()
	: m_pMemory(std::make_shared<MVariantMemory>())
	, m_nOffset(0)
	, m_nSize(0)
	, m_tMember()
{

}

MVariant& MVariantArray::operator[](const size_t& nIdx)
{
	MORTY_ASSERT(m_bLocked);
	if (nIdx >= m_tMember.size())
	{
		MORTY_ASSERT(nIdx < m_tMember.size());
		static MVariant InvalidValue;
		return InvalidValue;
	}

	return m_tMember[nIdx];
}

void MVariantArray::ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset)
{
	MORTY_ASSERT(m_bLocked);
	for (MVariant& member : m_tMember)
	{
		member.ResetMemory(pMemory, member.GetOffset() - m_nOffset + nOffset);
	}
	m_nOffset = nOffset;
}

MVariant MVariant::Clone(const MVariant& source, std::shared_ptr<MVariantMemory> pMemory, size_t nOffset)
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
		MORTY_ASSERT(source.m_pStruct);

		target.m_pStruct = MVariantStruct::Clone(source.m_pStruct, pMemory, nOffset);
		target.m_eType = MEVariantType::EStruct;
		target.m_nOffset = nOffset;
		target.m_nSize = target.m_pStruct->Size();
		target.m_pMemory = pMemory;
	}
	else if (source.GetType() == MEVariantType::EArray)
	{
		MORTY_ASSERT(source.m_pArray);

		target.m_pArray = MVariantArray::Clone(source.m_pArray, pMemory, nOffset);
		target.m_eType = MEVariantType::EArray;
		target.m_nOffset = nOffset;
		target.m_nSize = target.m_pArray->Size();
		target.m_pMemory = pMemory;
	}
	else
	{
		target = std::move(MVariant(pMemory, nOffset, source.m_nSize, source.GetType()));
	}

	return target;
}

std::shared_ptr<MVariantStruct> MVariantStruct::Clone(const std::shared_ptr<MVariantStruct>& pSource, std::shared_ptr<MVariantMemory> pMemory, size_t nOffset)
{
	MORTY_ASSERT(pSource->m_bLocked);
	std::shared_ptr<MVariantStruct> pTarget = std::make_shared<MVariantStruct>();

	pTarget->m_pMemory = pMemory;
	pTarget->m_nOffset = nOffset;
	pTarget->m_nSize = pSource->Size();
	pTarget->m_bLocked = true;

	for (auto& pr : pSource->GetMember())
	{
		pTarget->m_tMember[pr.first] = MVariant::Clone(pr.second, pMemory, pr.second.GetOffset() - pSource->m_nOffset + nOffset);
	}

	return pTarget;
}

std::shared_ptr<MVariantArray> MVariantArray::Clone(const std::shared_ptr<MVariantArray>& pSource, std::shared_ptr<MVariantMemory> pMemory, size_t nOffset)
{
	MORTY_ASSERT(pSource->m_bLocked);
	std::shared_ptr<MVariantArray> pTarget = std::make_shared<MVariantArray>();

	pTarget->m_pMemory = pMemory;
	pTarget->m_nOffset = nOffset;
	pTarget->m_nSize = pSource->Size();
	pTarget->m_bLocked = true;

	for (auto& pr : pSource->GetMember())
	{
		pTarget->m_tMember.push_back(MVariant::Clone(pr, pMemory, pr.GetOffset() - pSource->m_nOffset + nOffset));
	}

	return pTarget;
}

flatbuffers::Offset<void> MVariantStruct::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	std::vector<flatbuffers::Offset<mfbs::MStructMember>> vMembers;

	for(auto& pr : m_tMember)
	{
		const auto fbName = fbb.CreateString(pr.first);
		const auto fbVariant = pr.second.Serialize(fbb);

		mfbs::MStructMemberBuilder builder(fbb);
		builder.add_name(fbName);
		builder.add_value(fbVariant.o);
		builder.add_relative_offset(pr.second.GetOffset() - m_nOffset);
		vMembers.push_back(builder.Finish());
	}

	const auto fbMembers = fbb.CreateVector(vMembers);
	mfbs::MVariantStructBuilder builder(fbb);
	builder.add_member(fbMembers);
	builder.add_size(Size());
	return builder.Finish().Union();
}

void MVariantStruct::Deserialize(const void* pBufferPointer, std::shared_ptr<MVariantMemory> pMemory, size_t nOffset)
{
	MORTY_ASSERT(pMemory);

	m_pMemory = pMemory;
	m_nOffset = nOffset;

	const mfbs::MVariantStruct* fbData = reinterpret_cast<const mfbs::MVariantStruct*>(pBufferPointer);
	m_nSize = fbData->size();

	if(fbData->member())
	{
	    for(const auto& fbMember : *fbData->member())
	    {
			const auto fbVariant = reinterpret_cast<const mfbs::MVariant*>(fbMember->value());

			const size_t nOffset = fbMember->relative_offset() + m_nOffset;
			const MString strName = fbMember->name()->c_str();

			MVariant variant(m_pMemory, nOffset, 0, MEVariantType::ENone);
			variant.Deserialize(fbMember->value(), pMemory, nOffset);

			m_tMember[strName] = std::move(variant);
	    }
	}

	m_bLocked = true;
}

flatbuffers::Offset<void> MVariantArray::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	std::vector<flatbuffers::Offset<mfbs::MArrayMember>> vMembers;

	for (auto& value : m_tMember)
	{
		const auto fbVariant = value.Serialize(fbb);

		mfbs::MArrayMemberBuilder builder(fbb);
		builder.add_value(fbVariant.o);
		builder.add_relative_offset(value.GetOffset() - m_nOffset);
		vMembers.push_back(builder.Finish());
	}

	const auto fbMembers = fbb.CreateVector(vMembers);
	mfbs::MVariantArrayBuilder builder(fbb);
	builder.add_member(fbMembers);
	builder.add_size(Size());
	return builder.Finish().Union();
}

void MVariantArray::Deserialize(const void* pBufferPointer, std::shared_ptr<MVariantMemory> pMemory, size_t nOffset)
{
	MORTY_ASSERT(pMemory);

	m_pMemory = pMemory;
	m_nOffset = nOffset;

	const mfbs::MVariantArray* fbData = reinterpret_cast<const mfbs::MVariantArray*>(pBufferPointer);
	m_nSize = fbData->size();

	if (fbData->member())
	{
		for (const auto& fbMember : *fbData->member())
		{
			const auto fbVariant = reinterpret_cast<const mfbs::MVariant*>(fbMember->value());

			const size_t nOffset = fbMember->relative_offset() + m_nOffset;

			MVariant variant(m_pMemory, nOffset, 0, MEVariantType::ENone);
			variant.Deserialize(fbMember->value(), pMemory, nOffset);

			m_tMember.emplace_back(std::move(variant));
		}
	}

	m_bLocked = true;
}
