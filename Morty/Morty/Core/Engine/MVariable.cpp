#include "MVariable.h"

MStruct::MStruct() : m_unByteSize(0)
, m_pData(nullptr)
{

}

MStruct::~MStruct()
{
	if (m_pData)
		delete[] m_pData;
}

void MStruct::AppendVariable(const MString& strName, const MVariable& var)
{
	MStructMember sm;
	sm.strName = strName;
	sm.var = var;

	m_vMember.push_back(sm);
	m_unByteSize += var.GetSize();
	if (m_pData)
		delete[] m_pData;

	m_pData = new unsigned char[m_unByteSize];
}

void MStruct::AppendVariable(const MString& strName, const MString& type)
{
	MStructMember sm;
	sm.strName = strName;
	
	if (type == "float4")
	{
		sm.var = MVariable(Vector4());
	}
	else if (type == "float4x4")
	{
		sm.var = MVariable(Matrix4());
	}

	m_vMember.push_back(sm);
	m_unByteSize += sm.var.GetSize();
	if (m_pData)
		delete[] m_pData;

	m_pData = new unsigned char[m_unByteSize];
}

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

void* MStruct::GetData()
{

	if (nullptr == m_pData)
		m_pData = new unsigned char[m_unByteSize];

	unsigned int offset = 0;
	for (MStructMember& sm : m_vMember)
	{
		memcpy(m_pData + offset, sm.var.GetData(), sm.var.GetSize());
		offset += sm.var.GetSize();
	}

	return m_pData;
}

MStruct::MStruct(const MStruct& var)
{
	m_pData = nullptr;
	m_unByteSize = var.m_unByteSize;
	m_vMember.clear();
	for (const MStructMember& sm : var.m_vMember)
	{
		MStructMember nsm;
		nsm.strName = sm.strName;
		nsm.var = sm.var;
		m_vMember.push_back(nsm);
	}
}

const MStruct& MStruct::operator=(const MStruct& var)
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
	m_pData[0] = var;
	m_eType = EFloat;
	m_unByteSize = sizeof(float);
}

MVariable::MVariable(const Vector4& var)
{
	m_pData = (new unsigned char[sizeof(float) * 4]);
	memcpy(m_pData, var.m, sizeof(float) * 4);
	m_eType = EVector4;
	m_unByteSize = sizeof(float) * 4;
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

MVariable::MVariable()
	: m_pData(nullptr)
	, m_eType(ENone)
	, m_unByteSize(0)
{

}

void* MVariable::GetData()
{
	if (EStruct == m_eType)
		return ((MStruct*)(m_pData))->GetData();

	return m_pData;
}

unsigned int MVariable::GetSize() const
{
	if (EStruct == m_eType)
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
		else
			delete[] m_pData;

		m_eType = ENone;
		m_unByteSize = 0;
		m_pData = nullptr;
	}
}
