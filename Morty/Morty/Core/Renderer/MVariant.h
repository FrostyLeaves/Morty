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

class MContainer;
class MStruct;
class MVariantArray;

class MORTY_CLASS MVariant
{
public:

	enum MEVariantType
	{
		ENone = 0,
		EBool = 1,
		EInt = 2,
		EFloat = 3,
		EVector2 = 4,
		EVector3 = 5,
		EVector4 = 6,
		EMatrix3 = 7,
		EMatrix4 = 8,
		EStruct = 9,
		EArray = 10,
		EString = 11		//Not for shader.
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
	MVariant(const MStruct& var);
	MVariant(const MVariantArray& var);
	MVariant(const MVariant& var);
	MVariant(const MString& var);

	void* GetData() const;
	unsigned int GetSize() const;
	MEVariantType GetType() const { return m_eType; }

	template <typename T>
	T* GetVar() { return(T*)m_pData; }

	bool IsTrue() const { return m_pData && *((float*)m_pData) >= 0.5f; }

	int* GetInt() const { return (int*)m_pData; }
	int* GetBool() const { return (int*)m_pData; }
	float* GetFloat() const { return (float*)m_pData; }
	MString* GetString() const { return (MString*)m_pData; }

	MStruct* GetStruct() { return (MStruct*)m_pData; }
	const MStruct* GetStruct() const { return (const MStruct*)m_pData; }
	MVariantArray* GetArray() { return (MVariantArray*)m_pData; }
	const MVariantArray* GetArray() const { return (const MVariantArray*)m_pData; }
	MContainer* GetContainer() { return (MContainer*)m_pData; }

	const MVariant& operator = (const MVariant& var);

	void Move(MVariant& var);

	~MVariant();

	//从另一个var合并过来值
	void MergeFrom(const MVariant& var);

private:

	void Clean();
	unsigned char* m_pData;
	MEVariantType m_eType;
	unsigned int m_unByteSize;
};

class MContainer
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
	void* GetData();

	MStructMember* GetMember(const unsigned int& unIndex) { return unIndex < m_vMember.size() ? &m_vMember[unIndex] : nullptr; }
	const MStructMember* GetMember(const unsigned int& unIndex) const { return unIndex < m_vMember.size() ? &m_vMember[unIndex] : nullptr; }
	unsigned int GetMemberCount() const { return m_vMember.size(); }

	const MContainer& operator = (const MContainer& var);
	bool operator == (const MContainer& var) const;

	MVariant& operator[](const unsigned int& unIndex);

protected:

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
	MStruct():MContainer() {}
	virtual ~MStruct() {}


	void AppendMVariant(const MString& strName, const MVariant& var);
	MVariant* AppendMVariant(const MString& strName);

	void SetMember(const MString& strName, const MVariant& var);
	MVariant* FindMember(const MString& strName);
	const MVariant* FindMember(const MString& strName) const;


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
