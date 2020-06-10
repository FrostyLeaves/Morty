/**
 * @File         MVariant
 * 
 * @Created      2019-09-01 02:09:49
 *
 * @Author       Pobrecito
 *
 * Only For Shader.
**/

#ifndef _M_VARIANT_H_
#define _M_VARIANT_H_
#include "MGlobal.h"
#include "MString.h"
#include "Vector.h"
#include "Matrix.h"
#include <vector>
#include <map>
#include <unordered_map>  


#define M_VAR_GET_FUNC(TYPE, TYPE_NAME)\
 TYPE* Get##TYPE_NAME() { return m_eType == E##TYPE_NAME ? (TYPE*)m_pData : nullptr;} \
\
const TYPE* Get##TYPE_NAME() const { return m_eType == E##TYPE_NAME ? (const TYPE*)m_pData : nullptr;} \
\
template<> TYPE* GetTypedData<TYPE>(){ return Get##TYPE_NAME(); }\


class MContainer;
class MStruct;
class MVariantArray;

class MORTY_CLASS MVariant
{
public:

	enum MEVariantType
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

	unsigned int GetSize() const;
	MEVariantType GetType() const { return m_eType; }

	bool IsTrue() const { return m_pData && *((float*)m_pData) >= 0.5f; }

	template<typename T> T* GetTypedData() { return nullptr; }

	bool* GetBool() { return m_eType == EBool ? (bool*)(m_pData + sizeof(int) - sizeof(bool)) : nullptr; }
	const bool* GetBool() const { return m_eType == EBool ? (const bool*)(m_pData + sizeof(int) - sizeof(bool)) : nullptr; }

	M_VAR_GET_FUNC(int, Int);
	M_VAR_GET_FUNC(float, Float);
	M_VAR_GET_FUNC(MString, String);
	M_VAR_GET_FUNC(Vector2, Vector2);
	M_VAR_GET_FUNC(Vector3, Vector3);
	M_VAR_GET_FUNC(Vector4, Vector4);
	M_VAR_GET_FUNC(Quaternion, Quaternion);
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
	unsigned int m_unByteSize;
};

template<typename T>
MVariant::MVariant(const T& var)
{
	m_eType = EUser;
	m_unByteSize = sizeof(T);
	m_pData = (unsigned char*)new T();
	memcpy(m_pData, &var, sizeof(T));
}

class MORTY_CLASS MContainer
{
public:
	MContainer();
	MContainer(const MContainer& var);
	virtual ~MContainer();

	struct MStructMember
	{
		MString strName;
		MVariant var;
		unsigned int unBeginOffset;
	};

	unsigned int GetSize() const { return m_unByteSize; }
	MByte* GetData();

	//For Serialize
	void MemcpyData(MByte* pData);

	MStructMember* GetMember(const unsigned int& unIndex) { return unIndex < m_vMember.size() ? &m_vMember[unIndex] : nullptr; }
	const MStructMember* GetMember(const unsigned int& unIndex) const { return unIndex < m_vMember.size() ? &m_vMember[unIndex] : nullptr; }
	unsigned int GetMemberCount() const { return m_vMember.size(); }

	template <typename T>
	T* GetMember(const unsigned int& unIndex)
	{
		if (MStructMember* pMember = GetMember(unIndex))
			return pMember->var.GetTypedData<T>();
		return nullptr;
	}

	const MContainer& operator = (const MContainer& var);
	bool operator == (const MContainer& var) const;

	MVariant& operator[](const unsigned int& unIndex);

protected:

	//will move mem.var to owner var of array
	unsigned int AppendStructMember(MStructMember& mem);

protected:

	unsigned int m_unByteSize;
	unsigned char* m_pData;
	std::vector<MStructMember> m_vMember;

	static unsigned int s_unPackSize;
};

class MORTY_CLASS MStruct : public MContainer
{
public:
	MStruct() :MContainer()
		, m_tVariantMap() {}
	virtual ~MStruct() {}


	unsigned int AppendMVariant(const MString& strName, const MVariant& var);

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

	void Move(MStruct& sour);

protected:
	std::unordered_map< MString, unsigned int> m_tVariantMap;
};

class MORTY_CLASS MVariantArray : public MContainer
{
public:
	MVariantArray() :MContainer() {}
	MVariantArray(const unsigned int& unSize);
	virtual ~MVariantArray() {}

	void AppendMVariant(const MVariant& var);
	void Resize(const unsigned int& unSize);

	void Move(MVariantArray& sour);
};

#endif
