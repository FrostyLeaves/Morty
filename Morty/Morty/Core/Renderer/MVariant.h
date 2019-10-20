/**
 * @File         Variant
 * 
 * @Created      2019-09-01 02:09:49
 *
 * @Author       Morty
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

class MStruct;
class MVariantArray;

class MORTY_CLASS Variant
{
public:

	enum MEVariableType
	{
		ENone = 0,
		EFloat = 1,
		EVector3 = 2,
		EVector4 = 3,
		EMatrix3 = 4,
		EMatrix4 = 5,
		EStruct = 6,
		EArray = 7
	};

	Variant();
	Variant(const float& var);
	Variant(const Vector3& var);
	Variant(const Vector4& var);
	Variant(const Matrix3& var);
	Variant(const Matrix4& var);
	Variant(const MStruct& var);
	Variant(const MVariantArray& var);
	Variant(const Variant& var);

	void* GetData();
	unsigned int GetSize() const;
	MEVariableType GetType() const { return m_eType; }

	template <class T>
	T* GetByType(){ return(T*)m_pData; }
	
	const Variant& operator = (const Variant& var);

	~Variant();

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
		Variant var;
		unsigned int unBeginOffset;
	};

	unsigned int GetSize() const { return m_unByteSize; }
	void* GetData();
	unsigned int GetMemberCount() { return m_vMember.size(); }

	const MContainer& operator = (const MContainer& var);

protected:

	void AppendStructMember(MStructMember& mem);

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


	void AppendVariant(const MString& strName, const Variant& var);
//	void AppendVariant(const MString& strName, const MString& type);

	void SetMember(const MString& strName, const Variant& var);
	Variant* FindMember(const MString& strName);
};

class MORTY_CLASS MVariantArray : public MContainer
{
public:
	MVariantArray() :MContainer() {}
	virtual ~MVariantArray() {}

	void AppendVariant(const Variant& var);

	Variant& operator[](const unsigned int& unIndex);
};

#endif
