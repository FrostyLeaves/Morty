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

	template <class T>
	T* GetByType(){ return(T*)m_pData; }
	
	const MVariant& operator = (const MVariant& var);

	~MVariant();

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
	unsigned int GetMemberCount() { return m_vMember.size(); }

	const MContainer& operator = (const MContainer& var);

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

protected:
	std::unordered_map< MString, unsigned int> m_tVariantMap;
};

class MORTY_CLASS MVariantArray : public MContainer
{
public:
	MVariantArray() :MContainer() {}
	virtual ~MVariantArray() {}

	void AppendMVariant(const MVariant& var);

	MVariant& operator[](const unsigned int& unIndex);
};

#endif
