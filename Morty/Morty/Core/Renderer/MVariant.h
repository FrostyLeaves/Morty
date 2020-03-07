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

	enum MEVariableType
	{
		ENone = 0,
		EBool = 1,
		EInt = 2,
		EFloat = 3,
		EVector3 = 4,
		EVector4 = 5,
		EMatrix3 = 6,
		EMatrix4 = 7,
		EStruct = 8,
		EArray = 9,
		EString = 10		//Not for shader.
	};

	MVariant();
	MVariant(const bool& var);
	MVariant(const int& var);
	MVariant(const float& var);
	MVariant(const Vector3& var);
	MVariant(const Vector4& var);
	MVariant(const Matrix3& var);
	MVariant(const Matrix4& var);
	MVariant(const MStruct& var);
	MVariant(const MVariantArray& var);
	MVariant(const MVariant& var);
	MVariant(const MString& var);

	void* GetData();
	unsigned int GetSize() const;
	MEVariableType GetType() const { return m_eType; }

	float* GetInt() { return (float*)m_pData; }
	int* GetBool() { return (int*)m_pData; }
	float* GetFloat() { return (float*)m_pData; }

	MStruct* GetStruct() { return (MStruct*)m_pData; }
	const MStruct* GetStruct() const { return (const MStruct*)m_pData; }
	MVariantArray* GetArray() { return (MVariantArray*)m_pData; }
	const MVariantArray* GetArray() const { return (const MVariantArray*)m_pData; }
	MContainer* GetContainer() { return (MContainer*)m_pData; }

	const MVariant& operator = (const MVariant& var);

	~MVariant();

	//从另一个var合并过来值
	void MergeFrom(const MVariant& var);

private:

	void Clean();
	unsigned char* m_pData;
	MEVariableType m_eType;
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

	void SetMember(const MString& strName, const MVariant& var);
	MVariant* FindMember(const MString& strName);
	const MVariant* FindMember(const MString& strName) const;

protected:
	std::unordered_map< MString, unsigned int> m_tVariantMap;
};

class MORTY_CLASS MVariantArray : public MContainer
{
public:
	MVariantArray() :MContainer() {}
	virtual ~MVariantArray() {}

	void AppendMVariant(const MVariant& var);

};

#endif
