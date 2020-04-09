#include "MVariant.h"

unsigned int MContainer::s_unPackSize = 16;

MContainer::MContainer() : m_unByteSize(0)
, m_pData(nullptr)
{

}

MContainer::~MContainer()
{
	if (m_pData)
	{
		delete[] m_pData;
		m_pData = nullptr;
	}
}

void* MContainer::GetData()
{

	if (nullptr == m_pData)
		m_pData = new unsigned char[m_unByteSize];

	for (MStructMember& sm : m_vMember)
	{
		memcpy(m_pData + sm.unBeginOffset, sm.var.GetData(), sm.var.GetSize());
	}

	return m_pData;
}

MContainer::MContainer(const MContainer& var)
{
	m_pData = nullptr;
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
	if (m_unByteSize != var.m_unByteSize)
	{
		if (m_pData)
		{
			delete[] m_pData;
			m_pData = nullptr;
		}
	}
	m_unByteSize = var.m_unByteSize;
	m_vMember = var.m_vMember;
	return var;
}

MVariant::MVariant(const bool& var)
{
	m_pData = (new unsigned char[sizeof(float) * 1]);
	*(float*)(m_pData) = (var ? 1.0f : 0.0f);
	m_eType = EBool;
	m_unByteSize = sizeof(float);
}

MVariant::MVariant(const int& var)
{
	m_pData = (new unsigned char[sizeof(int) * 1]);
	memcpy(m_pData, &var, sizeof(int) * 1);
	m_eType = EInt;
	m_unByteSize = sizeof(int);
}

MVariant::MVariant(const float& var)
{
	m_pData = (new unsigned char[sizeof(float) * 1]);
	memcpy(m_pData, &var, sizeof(float) * 1);
	m_eType = EFloat;
	m_unByteSize = sizeof(float);
}

MVariant::MVariant(const Vector3& var)
{
	m_pData = (new unsigned char[sizeof(float) * 3]);
	memcpy(m_pData, var.m, sizeof(float) * 3);
	m_eType = EVector3;
	m_unByteSize = sizeof(float) * 3;
}

MVariant::MVariant(const Vector4& var)
{
	m_pData = (new unsigned char[sizeof(float) * 4]);
	memcpy(m_pData, var.m, sizeof(float) * 4);
	m_eType = EVector4;
	m_unByteSize = sizeof(float) * 4;
}

MVariant::MVariant(const Matrix3& var)
{
	m_pData = (new unsigned char[sizeof(float) * 12]);
	memcpy(m_pData + 0, var.m[0], sizeof(float) * 3);
	memcpy(m_pData + sizeof(float) * 4, var.m[1], sizeof(float) * 3);
	memcpy(m_pData + sizeof(float) * 8, var.m[2], sizeof(float) * 3);
	m_eType = EMatrix3;
	m_unByteSize = sizeof(float) * 4 * 3;
}

MVariant::MVariant(const Matrix4& var)
{
	m_pData = (new unsigned char[sizeof(float) * 16]);
	memcpy(m_pData, var.m, sizeof(float) * 16);
	m_eType = EMatrix4;
	m_unByteSize = sizeof(float) * 16;
}

MVariant::MVariant(const MStruct& var)
{
	m_pData = (unsigned char*)(new MStruct(var));
	m_eType = EStruct;
}

MVariant::MVariant(const MVariantArray& var)
{
	m_pData = (unsigned char*)(new MVariantArray(var));
	m_eType = EArray;
}

MVariant::MVariant()
	: m_pData(nullptr)
	, m_eType(ENone)
	, m_unByteSize(0)
{

}

MVariant::MVariant(const MString& var)
{
	m_pData = (unsigned char*)new MString(var);
	m_eType = EString;
	m_unByteSize = 0;
}

void* MVariant::GetData() const
{
	if (EStruct == m_eType || EArray == m_eType)
		return ((MStruct*)(m_pData))->GetData();

	return m_pData;
}

unsigned int MVariant::GetSize() const
{
	if (EStruct == m_eType || EArray == m_eType)
		return ((MStruct*)(m_pData))->GetSize();

	return m_unByteSize;
}

MVariant::MVariant(const MVariant& var)
{
	m_eType = var.m_eType;
	m_unByteSize = var.GetSize();
	if (EStruct == var.m_eType)
	{
		m_pData = (unsigned char*)(new MStruct());
		*((MStruct*)m_pData) = *((MStruct*)var.m_pData);
	}
	else if (EArray == var.m_eType)
	{
		m_pData = (unsigned char*)(new MVariantArray());
		*((MVariantArray*)m_pData) = *((MVariantArray*)var.m_pData);
	}
	else if (EString == var.m_eType)
	{
		m_pData = (unsigned char*)(new MString(*((MString*)var.m_pData)));
	}
	else
	{
		m_pData = new unsigned char[var.m_unByteSize];
		memcpy(m_pData, var.m_pData, var.m_unByteSize);
	}
}

const MVariant& MVariant::operator=(const MVariant& var)
{
	Clean();
	m_eType = var.m_eType;
	m_unByteSize = var.GetSize();
	if (EStruct == var.m_eType)
	{
		m_pData = (unsigned char*)(new MStruct());
		*((MStruct*)m_pData) = *((MStruct*)var.m_pData);
	}
	else if (EArray == var.m_eType)
	{
		m_pData = (unsigned char*)(new MVariantArray());
		*((MVariantArray*)m_pData) = *((MVariantArray*)var.m_pData);
	}
	else if (EString == var.m_eType)
	{
		m_pData = (unsigned char*)(new MString(*((MString*)var.m_pData)));
	}
	else
	{
		m_pData = new unsigned char[var.m_unByteSize];
		memcpy(m_pData, var.m_pData, var.m_unByteSize);
	}

	return var;
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
		case MVariant::EStruct:
		{
			MStruct& cStruct = *GetStruct();
			const MStruct& cSource = *var.GetStruct();

			for (unsigned int i = 0; i < cStruct.GetMemberCount(); ++i)
			{
				MContainer::MStructMember* pMem = cStruct.GetMember(i);
				if (const MVariant* pSourceChildVar = cSource.FindMember(pMem->strName))
					pMem->var.MergeFrom(*pSourceChildVar);
			}
			break;
		}

		case MVariant::EArray:
		{
			MVariantArray& cArray = *GetArray();
			const MVariantArray& cSource = *var.GetArray();

			unsigned int unSize = cArray.GetMemberCount();
			if (unSize > cSource.GetMemberCount()) unSize = cSource.GetMemberCount();

			for (unsigned int i = 0; i < unSize; ++i)
			{
				MContainer::MStructMember* pMem = cArray.GetMember(i);
				const MContainer::MStructMember* pSourceMem = cSource.GetMember(i);
				pMem->var.MergeFrom(pSourceMem->var);
			}

			break;
		}

		case MVariant::ENone:
			break;

		default:
		
			*this = var;
			break;

	}
}

void MVariant::Clean()
{
	if (ENone != m_eType)
	{
		if (EStruct == m_eType)
			delete ((MStruct*)m_pData);
		else if (EArray == m_eType)
			delete ((MVariantArray*)m_pData);
		else if (EString == m_eType)
			delete ((MString*)m_pData);
		else
			delete[] m_pData;

		m_eType = ENone;
		m_unByteSize = 0;
		m_pData = nullptr;
	}
}

unsigned int MContainer::AppendStructMember(MStructMember& mem)
{
	unsigned int unIndex = 0;
	unsigned int unRemainder = m_unByteSize % s_unPackSize;
	if (unRemainder != 0 && (s_unPackSize - unRemainder) >= mem.var.GetSize())
	{
		mem.unBeginOffset = m_unByteSize;
		m_vMember.push_back(mem);
		unIndex = m_vMember.size() - 1;
		m_unByteSize += mem.var.GetSize();
	}
	else
	{
		if (unRemainder != 0)
			m_unByteSize = (m_unByteSize / s_unPackSize + 1) * s_unPackSize;

		mem.unBeginOffset = m_unByteSize;
		m_vMember.push_back(mem);
		unIndex = m_vMember.size() - 1;
		m_unByteSize += mem.var.GetSize();
	}

	if (m_pData)
		delete[] m_pData;

	m_pData = new unsigned char[m_unByteSize];

	return unIndex;
}

void MStruct::AppendMVariant(const MString& strName, const MVariant& var)
{
	MStructMember sm;
	sm.strName = strName;
	sm.var = var;

	m_tVariantMap[strName] = AppendStructMember(sm);
}

void MStruct::SetMember(const MString& strName, const MVariant& var)
{
	std::unordered_map<MString, unsigned int>::iterator iter = m_tVariantMap.find(strName);
	if (iter != m_tVariantMap.end())
	{
		MStructMember& mem = m_vMember[iter->second];
		mem.var = var;
	}
}

MVariant* MStruct::FindMember(const MString& strName)
{
	std::unordered_map<MString, unsigned int>::iterator iter = m_tVariantMap.find(strName);
	if (iter != m_tVariantMap.end())
	{
		MStructMember& mem = m_vMember[iter->second];
		return &mem.var;
	}

	return nullptr;
}

const MVariant* MStruct::FindMember(const MString& strName) const
{
	std::unordered_map<MString, unsigned int>::const_iterator iter = m_tVariantMap.find(strName);
	if (iter != m_tVariantMap.end())
	{
		const MStructMember& mem = m_vMember[iter->second];
		return &mem.var;
	}

	return nullptr;
}

void MVariantArray::AppendMVariant(const MVariant& var)
{
	MStructMember sm;
	sm.strName = "";
	sm.var = var;

	AppendStructMember(sm);
}

MVariant& MContainer::operator[](const unsigned int& unIndex)
{
	if (0 <= unIndex && unIndex < m_vMember.size())
		return m_vMember[unIndex].var;

	static MVariant uselessVar;
	return uselessVar;
}
