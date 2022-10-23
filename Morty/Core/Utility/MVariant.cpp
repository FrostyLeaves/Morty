#include "Utility/MVariant.h"

uint32_t MContainer::s_unPackageSize = 16;

MContainer::MContainer()
	: m_unByteSize(0)
	, m_data()
	, m_ContainerType(MVariant::MEVariantType::EStruct)
{

}

MContainer::~MContainer()
{
}

uint32_t MContainer::GetSize() const
{
	return m_unByteSize;
}

MByte* MContainer::GetData()
{
	if (m_data.size() < m_unByteSize)
		m_data.resize(m_unByteSize);

	for (MStructMember& sm : m_vMember)
	{
		memcpy(m_data.data() + sm.unBeginOffset, sm.var.GetData(), sm.var.GetSize());
	}

	return m_data.data();
}

MContainer::MContainer(const MContainer& var)
{
	m_data.clear();
	m_unByteSize = var.m_unByteSize;
	m_vMember.clear();

	//m_vMember = var.m_vMember;
	for (const MStructMember& sm : var.m_vMember)
	{
		MStructMember nsm;
		nsm.strName = sm.strName;
		nsm.var = sm.var;
		nsm.unBeginOffset = sm.unBeginOffset;
		m_vMember.push_back(nsm);
	}
}

const MContainer& MContainer::operator=(const MContainer& var)
{
	m_data = var.m_data;
	m_unByteSize = var.m_unByteSize;
	m_vMember = var.m_vMember;
	return var;
}

MVariant::MVariant(const bool& var)
{
	m_pData = (new MByte[sizeof(int) * 1]);
	*(int*)(m_pData) = var;
	m_eType = MEVariantType::EBool;
	m_unByteSize = sizeof(int);
}

MVariant::MVariant(const int& var)
{
	m_pData = (new MByte[sizeof(int) * 1]);
	memcpy(m_pData, &var, sizeof(int) * 1);
	m_eType = MEVariantType::EInt;
	m_unByteSize = sizeof(int);
}

MVariant::MVariant(const float& var)
{
	m_pData = (new MByte[sizeof(float) * 1]);
	memcpy(m_pData, &var, sizeof(float) * 1);
	m_eType = MEVariantType::EFloat;
	m_unByteSize = sizeof(float);
}

MVariant::MVariant(const Vector2& var)
{
	m_pData = (new MByte[sizeof(float) * 2]);
	memcpy(m_pData, var.m, sizeof(float) * 2);
	m_eType = MEVariantType::EVector2;
	m_unByteSize = sizeof(float) * 2;
}

MVariant::MVariant(const Vector3& var)
{
	m_pData = (new MByte[sizeof(float) * 3]);
	memcpy(m_pData, var.m, sizeof(float) * 3);
	m_eType = MEVariantType::EVector3;
	m_unByteSize = sizeof(float) * 3;
}

MVariant::MVariant(const Vector4& var)
{
	m_pData = (new MByte[sizeof(float) * 4]);
	memcpy(m_pData, var.m, sizeof(float) * 4);
	m_eType = MEVariantType::EVector4;
	m_unByteSize = sizeof(float) * 4;
}

MVariant::MVariant(const Matrix3& var)
{
	m_pData = (new MByte[sizeof(float) * 12]);
	memcpy(m_pData + 0, var.m[0], sizeof(float) * 3);
	memcpy(m_pData + sizeof(float) * 4, var.m[1], sizeof(float) * 3);
	memcpy(m_pData + sizeof(float) * 8, var.m[2], sizeof(float) * 3);
	m_eType = MEVariantType::EMatrix3;
	m_unByteSize = sizeof(float) * 4 * 3;
}

MVariant::MVariant(const Matrix4& var)
{
	m_pData = (new MByte[sizeof(float) * 16]);
	memcpy(m_pData, var.m, sizeof(float) * 16);
	m_eType = MEVariantType::EMatrix4;
	m_unByteSize = sizeof(float) * 16;
}

MVariant::MVariant(const MStruct& var)
{
	m_pData = (MByte*)(new MStruct(var));
	m_eType = MEVariantType::EStruct;
}

MVariant::MVariant(const MVariantArray& var)
{
	m_pData = (MByte*)(new MVariantArray(var));
	m_eType = MEVariantType::EArray;
}

MVariant::MVariant()
	: m_pData(nullptr)
	, m_eType(MEVariantType::ENone)
	, m_unByteSize(0)
{

}

MVariant::MVariant(const MString& var)
{
	m_unByteSize = 0;
	m_pData = (MByte*)new MString();
	*((MString*)m_pData) = var;
	m_eType = MEVariantType::EString;
}

MVariant::MVariant(const Quaternion& var)
{
	m_pData = (new MByte[sizeof(float) * 4]);
	memcpy(m_pData, &var.w, sizeof(float));
	memcpy(m_pData + sizeof(float), var.m, sizeof(float) * 3);
	m_eType = MEVariantType::EQuaternion;
	m_unByteSize = sizeof(float) * 4;
}

MByte* MVariant::GetData() const
{
	if (MEVariantType::EStruct == m_eType || MEVariantType::EArray == m_eType)
		return ((MStruct*)(m_pData))->GetData();

	return m_pData;
}

uint32_t MVariant::GetSize() const
{
	if (MEVariantType::EStruct == m_eType || MEVariantType::EArray == m_eType)
		return ((MStruct*)(m_pData))->GetSize();
	else if (MEVariantType::EString == m_eType)
		return ((MString*)(m_pData))->size();

	return m_unByteSize;
}

MVariant::MVariant(const MVariant& var)
	: m_pData(nullptr)
	, m_eType(MEVariantType::ENone)
	, m_unByteSize(0)
{
	*this = var;
}

const MVariant& MVariant::operator=(const MVariant& var)
{
	Clean();
	m_eType = var.m_eType;
	m_unByteSize = var.GetSize();
	if (MEVariantType::EStruct == var.m_eType)
	{
		m_pData = (MByte*)(new MStruct());
		*((MStruct*)m_pData) = *((MStruct*)var.m_pData);
	}
	else if (MEVariantType::EArray == var.m_eType)
	{
		m_pData = (MByte*)(new MVariantArray());
		*((MVariantArray*)m_pData) = *((MVariantArray*)var.m_pData);
	}
	else if (MEVariantType::EString == var.m_eType)
	{
		m_pData = (MByte*)(new MString(*((MString*)var.m_pData)));
	}
	else
	{
		m_pData = new MByte[var.m_unByteSize];
		memcpy(m_pData, var.m_pData, var.m_unByteSize);
	}

	return var;
}

void MVariant::Move(MVariant& var)
{
	Clean();

	m_pData = var.m_pData;
	m_eType = var.m_eType;
	m_unByteSize = var.m_unByteSize;

	var.m_pData = nullptr;
	var.m_eType = MEVariantType::ENone;
	var.m_unByteSize = 0;
}

MVariant::~MVariant()
{
	Clean();
}

void MVariant::MergeFrom(const MVariant& var)
{
	if (GetType() != var.GetType())
		return;

	switch (GetType())
	{
		case MVariant::MEVariantType::EStruct:
		{
			MStruct& cStruct = *GetStruct();
			const MStruct& cSource = *var.GetStruct();

			for (uint32_t i = 0; i < cStruct.GetMemberCount(); ++i)
			{
				MContainer::MStructMember* pMem = cStruct.GetMember(i);
				if (const MVariant* pSourceChildVar = cSource.GetValue(pMem->strName))
					pMem->var.MergeFrom(*pSourceChildVar);
			}
			break;
		}

		case MVariant::MEVariantType::EArray:
		{
			MVariantArray& cArray = *GetArray();
			const MVariantArray& cSource = *var.GetArray();

			uint32_t unSize = cArray.GetMemberCount();
			if (unSize > cSource.GetMemberCount()) unSize = cSource.GetMemberCount();

			for (uint32_t i = 0; i < unSize; ++i)
			{
				MContainer::MStructMember* pMem = cArray.GetMember(i);
				const MContainer::MStructMember* pSourceMem = cSource.GetMember(i);
				pMem->var.MergeFrom(pSourceMem->var);
			}

			break;
		}

		case MVariant::MEVariantType::ENone:
			break;

		default:
		
			*this = var;
			break;

	}
}

void MVariant::Clean()
{
	if (MEVariantType::ENone != m_eType)
	{
		if (m_pData)
		{
			if (MEVariantType::EStruct == m_eType)
				delete ((MStruct*)m_pData);
			else if (MEVariantType::EArray == m_eType)
				delete ((MVariantArray*)m_pData);
			else if (MEVariantType::EString == m_eType)
				delete (MString*)m_pData;
			else
				delete[] m_pData;
		}

		m_eType = MEVariantType::ENone;
		m_unByteSize = 0;
		m_pData = nullptr;
	}
}

uint32_t MContainer::AppendStructMember(MStructMember& mem)
{
	uint32_t unIndex = 0;
	uint32_t unRemainder = m_unByteSize % s_unPackageSize;
	if (unRemainder != 0 && (s_unPackageSize - unRemainder) >= mem.var.GetSize())
	{
		mem.unBeginOffset = m_unByteSize;
		m_unByteSize += mem.var.GetSize();

		m_vMember.push_back(MStructMember());
		m_vMember.back().strName = mem.strName;
		m_vMember.back().unBeginOffset = mem.unBeginOffset;
		m_vMember.back().var.Move(mem.var);

		unIndex = m_vMember.size() - 1;
	}
	else
	{
		if (unRemainder != 0)
			m_unByteSize = (m_unByteSize / s_unPackageSize + 1) * s_unPackageSize;

		mem.unBeginOffset = m_unByteSize;
		m_unByteSize += mem.var.GetSize();

		
		if (MVariant::MEVariantType::EArray == m_ContainerType && m_unByteSize % s_unPackageSize != 0)
			m_unByteSize += (s_unPackageSize - m_unByteSize % s_unPackageSize);

		m_vMember.push_back(MStructMember());
		m_vMember.back().strName = mem.strName;
		m_vMember.back().unBeginOffset = mem.unBeginOffset;
		m_vMember.back().var.Move(mem.var);

		unIndex = m_vMember.size() - 1;
	}

	m_data.resize(m_unByteSize);

	return unIndex;
}

MStruct::MStruct() :MContainer()
, m_tVariantMap()
{
	m_ContainerType = MVariant::MEVariantType::EStruct;
}

void MStruct::SetValue(const MString& strName, const MVariant& var)
{
	std::unordered_map<MString, uint32_t>::iterator iter = m_tVariantMap.find(strName);
	if (iter != m_tVariantMap.end())
	{
		MStructMember& mem = m_vMember[iter->second];
		mem.var = var;
	}
	else
	{
		MStructMember sm;
		sm.strName = strName;
		sm.var = var;
		m_tVariantMap[strName] = AppendStructMember(sm);
	}
}

MVariant* MStruct::GetValue(const MString& strName)
{
	std::unordered_map<MString, uint32_t>::iterator iter = m_tVariantMap.find(strName);
	if (iter != m_tVariantMap.end())
	{
		MStructMember& mem = m_vMember[iter->second];
		return &mem.var;
	}

	return nullptr;
}

const MVariant* MStruct::GetValue(const MString& strName) const
{
	std::unordered_map<MString, uint32_t>::const_iterator iter = m_tVariantMap.find(strName);
	if (iter != m_tVariantMap.end())
	{
		const MStructMember& mem = m_vMember[iter->second];
		return &mem.var;
	}

	return nullptr;
}

void MStruct::Move(MStruct& sour)
{
	m_vMember = std::move(sour.m_vMember);
	m_tVariantMap = std::move(sour.m_tVariantMap);
}

MVariantArray::MVariantArray()
	: MContainer()
{
	m_ContainerType = MVariant::MEVariantType::EArray;
}

void MVariantArray::AppendValue(const MVariant& var)
{
	MStructMember sm;
	sm.strName = "";
	sm.var = var;

	AppendStructMember(sm);
}

void MVariantArray::Move(MVariantArray& sour)
{
	m_vMember = std::move(sour.m_vMember);
}

MVariant& MContainer::operator[](const uint32_t& unIndex)
{
	if (0 <= unIndex && unIndex < m_vMember.size())
		return m_vMember[unIndex].var;

	static MVariant uselessVar;
	return uselessVar;
}

const MVariant& MContainer::operator[](const uint32_t& unIndex) const
{
	if (0 <= unIndex && unIndex < m_vMember.size())
		return m_vMember[unIndex].var;

	static MVariant uselessVar;
	return uselessVar;
}
