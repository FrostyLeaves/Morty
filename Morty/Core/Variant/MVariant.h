/**
 * @File         MVariant
 * 
 * @Created      2019-09-01 02:09:49
 *
 * @Author       DoubleYe
 *
 * Only For Shader.
**/

#ifndef _M_VARIANT_H_
#define _M_VARIANT_H_
#include "Utility/MGlobal.h"
#include "Utility/MString.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include <vector>
#include <map>
#include <unordered_map>  


#define M_VAR_GET_FUNC(TYPE, TYPE_NAME)\
 TYPE* Get##TYPE_NAME() { return m_eType == MEVariantType::E##TYPE_NAME ? (TYPE*)m_pData : nullptr;} \
\
const TYPE* Get##TYPE_NAME() const { return m_eType == MEVariantType::E##TYPE_NAME ? (const TYPE*)m_pData : nullptr;} \
\
template<> TYPE* GetTypedData<TYPE>(){ return Get##TYPE_NAME(); }\


class MContainer;
class MStruct;
class MVariantArray;

class MORTY_API MVariant
{
public:

	enum class MEVariantType
	{
		ENone,
		EBool,
		EInt,
		EFloat,
		EVector2,
		EVector3,
		EVector4,
		EMatrix3,
		EMatrix4,
		EQuaternion,
		EStruct,
		EArray,
		EString,		//Not for shader.
		EUser,
	};

	MVariant();
	MVariant(const bool& var);
	MVariant(const int& var);
	MVariant(const float& var);
	MVariant(const Vector2& var);
	MVariant(const Vector3& var);
	MVariant(const Vector4& var);
	MVariant(const Matrix3& var);
	MVariant(const Matrix4& var);
	MVariant(const Quaternion& var);
	MVariant(const MStruct& var);
	MVariant(const MVariantArray& var);
	MVariant(const MVariant& var);
	MVariant(const MString& var);

	template<typename T>
	MVariant(const T& var);

	MByte* GetData() const;
	void MemcpyData(MByte* pData);

	uint32_t GetSize() const;
	MEVariantType GetType() const { return m_eType; }

	bool IsTrue() const { return m_pData && *((int*)m_pData) != 0; }

	template<typename T> T* GetTypedData() { return nullptr; }

	template<> bool* GetTypedData<bool>() { return GetBool(); }
	bool* GetBool() { return m_eType == MEVariantType::EBool ? (bool*)(m_pData) : nullptr; }
	const bool* GetBool() const { return m_eType == MEVariantType::EBool ? (const bool*)(m_pData) : nullptr; }

	M_VAR_GET_FUNC(int, Int);
	M_VAR_GET_FUNC(float, Float);
	M_VAR_GET_FUNC(MString, String);
	M_VAR_GET_FUNC(Vector2, Vector2);
	M_VAR_GET_FUNC(Vector3, Vector3);
	M_VAR_GET_FUNC(Vector4, Vector4);
	M_VAR_GET_FUNC(Quaternion, Quaternion);
	M_VAR_GET_FUNC(Matrix3, Matrix3);
	M_VAR_GET_FUNC(Matrix4, Matrix4);
	M_VAR_GET_FUNC(MStruct, Struct);
	M_VAR_GET_FUNC(MVariantArray, Array);

	float* CastFloatUnsafe() const { return (float*)m_pData; }

	template <typename T>
	T& GetVarUnsafe() const { return *(T*)m_pData; }

	template <typename T>
	T* GetPointerUnsafe() const { return (T*)m_pData; }

	template<typename T>
	T* GetEnum() { return (T*)GetInt(); }

	template<typename T>
	const T* GetEnum() const { return (const T*)GetInt(); }

	MContainer* GetContainer() { return (MContainer*)m_pData; }
	const MContainer* GetContainer() const { return (const MContainer*)m_pData; }

	const MVariant& operator = (const MVariant& var);

	~MVariant();

	void Move(MVariant& var);

	//从另一个var合并过来值
	void MergeFrom(const MVariant& var);

private:

	void Clean();
	MByte* m_pData;
	MEVariantType m_eType;
	uint32_t m_unByteSize;
};

template<typename T>
MVariant::MVariant(const T& var)
{
	m_eType = MEVariantType::EUser;
	m_unByteSize = sizeof(T);
	m_pData = (unsigned char*)new T();
	memcpy(m_pData, &var, sizeof(T));
}

class MORTY_API MContainer
{
public:
	MContainer();
	MContainer(const MContainer& var);
	virtual ~MContainer();

	struct MStructMember
	{
		MString strName;
		MVariant var;
		uint32_t unBeginOffset;
	};

	uint32_t GetSize() const;
	MByte* GetData();

	//For Serialize
	void MemcpyData(MByte* pData);

	MStructMember* GetMember(const uint32_t& unIndex) { return unIndex < m_vMember.size() ? &m_vMember[unIndex] : nullptr; }
	const MStructMember* GetMember(const uint32_t& unIndex) const { return unIndex < m_vMember.size() ? &m_vMember[unIndex] : nullptr; }
	uint32_t GetMemberCount() const { return m_vMember.size(); }

	MVariant* Back();

	template <typename T>
	T* GetMember(const uint32_t& unIndex)
	{
		if (MStructMember* pMember = GetMember(unIndex))
			return pMember->var.GetTypedData<T>();
		return nullptr;
	}

	const MContainer& operator = (const MContainer& var);
	bool operator == (const MContainer& var) const;

	MVariant& operator[](const uint32_t& unIndex);

protected:

	//will move mem.var to owner var of array
	uint32_t AppendStructMember(MStructMember& mem);

protected:

	MVariant::MEVariantType m_ContainerType;
	uint32_t m_unByteSize;
	unsigned char* m_pData;
	std::vector<MStructMember> m_vMember;

	static uint32_t s_unPackSize;
};

class MORTY_API MStruct : public MContainer
{
public:
	MStruct();
	virtual ~MStruct() {}


	uint32_t AppendMVariant(const MString& strName, const MVariant& var);

	template<typename T>
	T* AppendMVariant(const MString& strName)
	{
		MStructMember sm;
		sm.strName = strName;
		sm.var = T();

		uint32_t unIndex = m_tVariantMap[strName] = AppendStructMember(sm);

		return m_vMember[unIndex].var.GetTypedData<T>();
	}

	void SetMember(const MString& strName, const MVariant& var);
	MVariant* FindMember(const MString& strName);
	const MVariant* FindMember(const MString& strName) const;

	template<typename T>
	T* FindMember(const MString& strName)
	{
		if (MVariant* pVar = FindMember(strName))
			return pVar->GetTypedData<T>();
		return nullptr;
	}

	template<typename T>
	bool FindMember(const MString& strName, T& result)
	{
		if (MVariant* pVar = FindMember(strName))
		{
			result = *pVar->GetTypedData<T>();
			return true;
		}

		return false;
	}

	void Move(MStruct& sour);

protected:
	std::unordered_map< MString, uint32_t> m_tVariantMap;
};

class MORTY_API MVariantArray : public MContainer
{
public:
	MVariantArray();
	virtual ~MVariantArray() {}

	void AppendMVariant(const MVariant& var);

//	void Resize(const uint32_t& unSize, const MVariant& var);

	void Move(MVariantArray& sour);

	template<typename T>
	T* AppendMVariant()
	{
		AppendMVariant(T());
		return  m_vMember.back().var.GetTypedData<T>();
	}
};

// void MVariantArray::Resize(const uint32_t& unSize, const MVariant& var)
// {
// 	m_vMember.resize(unSize, MStructMember());
// 	uint32_t unWidth = var.GetSize() / s_unPackSize;
// 	if (var.GetSize() % s_unPackSize) unWidth += 1;
// 	
// 	for (uint32_t i = 0; i < unSize; ++i)
// 	{
// 		m_vMember[i].unBeginOffset = unWidth * s_unPackSize * i;
// 		m_vMember[i].var = var;
// 	}
// 
// 	m_unByteSize = unSize * unWidth * s_unPackSize;
// }

#endif
