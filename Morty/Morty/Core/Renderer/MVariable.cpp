#include "MVariable.h"

unsigned int MContainer::s_unPackSize = 16;

MContainer::MContainer() : m_unByteSize(0)
, m_pData(nullptr)
{

}

MContainer::~MContainer()
{
	if (m_pData)
		delete[] m_pData;
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

MVariable::MVariable(const float& var)
{
	m_pData = (new unsigned char[sizeof(float) * 1]);
	memcpy(m_pData, &var, sizeof(float) * 1);
	m_eType = EFloat;
	m_unByteSize = sizeof(float);
}

MVariable::MVariable(const Vector3& var)
{
	m_pData = (new unsigned char[sizeof(float) * 3]);
	memcpy(m_pData, var.m, sizeof(float) * 3);
	m_eType = EVector3;
	m_unByteSize = sizeof(float) * 3;
}

MVariable::MVariable(const Vector4& var)
{
	m_pData = (new unsigned char[sizeof(float) * 4]);
	memcpy(m_pData, var.m, sizeof(float) * 4);
	m_eType = EVector4;
	m_unByteSize = sizeof(float) * 4;
}

MVariable::MVariable(const Matrix3& var)
{
	m_pData = (new unsigned char[sizeof(float) * 12]);
	memcpy(m_pData + 0, var.m[0], sizeof(float) * 3);
	memcpy(m_pData + sizeof(float) * 4, var.m[1], sizeof(float) * 3);
	memcpy(m_pData + sizeof(float) * 8, var.m[2], sizeof(float) * 3);
	m_eType = EMatrix3;
	m_unByteSize = sizeof(float) * 4 * 3;
}

MVariable::MVariable(const Matrix4& var)
{
	m_pData = (new unsigned char[sizeof(float) * 16]);
	memcpy(m_pData, var.m, sizeof(float) * 16);
	m_eType = EMatrix4;
	m_unByteSize = sizeof(float) * 16;
}

MVariable::MVariable(const MStruct& var)
{
	m_pData = (unsigned char*)(new MStruct(var));
	m_eType = EStruct;
}

MVariable::MVariable(const MVariantArray& var)
{
	m_pData = (unsigned char*)(new MVariantArray(var));
	m_eType = EArray;
}

MVariable::MVariable()
	: m_pData(nullptr)
	, m_eType(ENone)
	, m_unByteSize(0)
{

}

void* MVariable::GetData()
{
	if (EStruct == m_eType || EArray == m_eType)
		return ((MStruct*)(m_pData))->GetData();

	return m_pData;
}

unsigned int MVariable::GetSize() const
{
	if (EStruct == m_eType || EArray == m_eType)
		return ((MStruct*)(m_pData))->GetSize();

	return m_unByteSize;
}

MVariable::MVariable(const MVariable& var)
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
	else
	{
		m_pData = new unsigned char[var.m_unByteSize];
		memcpy(m_pData, var.m_pData, var.m_unByteSize);
	}
}

const MVariable& MVariable::operator=(const MVariable& var)
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
	else
	{
		m_pData = new unsigned char[var.m_unByteSize];
		memcpy(m_pData, var.m_pData, var.m_unByteSize);
	}

	return var;
}

MVariable::~MVariable()
{
	Clean();
}

void MVariable::Clean()
{
	if (ENone != m_eType)
	{
		if (EStruct == m_eType)
			delete ((MStruct*)m_pData);
		else if (EArray == m_eType)
			delete ((MVariantArray*)m_pData);
		else
			delete[] m_pData;

		m_eType = ENone;
		m_unByteSize = 0;
		m_pData = nullptr;
	}
}

void MContainer::AppendStructMember(MStructMember& mem)
{
	unsigned int unRemainder = m_unByteSize % s_unPackSize;
	if (unRemainder != 0 && (s_unPackSize - unRemainder) > mem.var.GetSize())
	{
		mem.unBeginOffset = m_unByteSize;
		m_vMember.push_back(mem);
	}
	else
	{
		if (unRemainder != 0)
			m_unByteSize = (m_unByteSize / s_unPackSize + 1) * s_unPackSize;

		mem.unBeginOffset = m_unByteSize;
		m_vMember.push_back(mem);
		m_unByteSize += mem.var.GetSize();
	}

	if (m_pData)
		delete[] m_pData;

	m_pData = new unsigned char[m_unByteSize];
}

void MStruct::AppendVariable(const MString& strName, const MVariable& var)
{
	MStructMember sm;
	sm.strName = strName;
	sm.var = var;

	AppendStructMember(sm);
}
// 
// void MStruct::AppendVariable(const MString& strName, const MString& type)
// {
// 	MStructMember sm;
// 	sm.strName = strName;
// 
// 	if (type == "float4")
// 	{
// 		sm.var = MVariable(Vector4());
// 	}
// 	else if (type == "float3")
// 	{
// 		sm.var = MVariable(Vector3());
// 	}
// 	else if (type == "float3x3")
// 	{
// 		sm.var = MVariable(Matrix3());
// 	}
// 	else if (type == "float4x4")
// 	{
// 		sm.var = MVariable(Matrix4());
// 	}
// 
// 	AppendStructMember(sm);
// }

void MStruct::SetMember(const MString& strName, const MVariable& var)
{
	for (MStructMember& mem : m_vMember)
	{
		if (mem.strName == strName)
		{
			mem.var = var;
			break;
		}
	}
}

MVariable* MStruct::FindMember(const MString& strName)
{
	for (MStructMember& mem : m_vMember)
	{
		if (mem.strName == strName)
		{
			return &mem.var;
			break;
		}
	}

	return nullptr;
}

void MVariantArray::AppendVariable(const MVariable& var)
{
	MStructMember sm;
	sm.strName = "";
	sm.var = var;

	AppendStructMember(sm);
}

MVariable& MVariantArray::operator[](const unsigned int& unIndex)
{
	if (0 <= unIndex && unIndex < m_vMember.size())
		return m_vMember[unIndex].var;

	static MVariable uselessVar;
	return uselessVar;
}
