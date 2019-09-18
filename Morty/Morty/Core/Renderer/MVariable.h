/**
 * @File         MVariable
 * 
 * @Created      2019-09-01 02:09:49
 *
 * @Author       Morty
 *
 * Only For Shader.
**/

#ifndef _M_MVARIABLE_H_
#define _M_MVARIABLE_H_
#include "MGlobal.h"
#include "MString.h"
#include "Vector.h"
#include "Matrix.h"
#include <vector>
#include <map>

class MStruct;

class MORTY_CLASS MVariable
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
	};

	MVariable();
	MVariable(const float& var);
	MVariable(const Vector3& var);
	MVariable(const Vector4& var);
	MVariable(const Matrix3& var);
	MVariable(const Matrix4& var);
	MVariable(const MStruct& var);
	MVariable(const MVariable& var);

	void* GetData();
	unsigned int GetSize() const;
	MEVariableType GetType() const { return m_eType; }

	template <class T>
	T* GetByType(){ return(T*)m_pData; }
	
	const MVariable& operator = (const MVariable& var);

	~MVariable();

private:

	void Clean();
	unsigned char* m_pData;
	MEVariableType m_eType;
	unsigned int m_unByteSize;
};


class MORTY_CLASS MStruct
{
public:
	MStruct();
	MStruct(const MStruct& var);
	~MStruct();

	struct MStructMember
	{
		MString strName;
		MVariable var;
		unsigned int unBeginOffset;
	};

	void AppendVariable(const MString& strName, const MVariable& var);
	void AppendVariable(const MString& strName, const MString& type);

	void SetMember(const MString& strName, const MVariable& var);
	MVariable* FindMember(const MString& strName);

	unsigned int GetSize() const { return m_unByteSize; }
	void* GetData();

	std::map<MString, MVariable::MEVariableType> GetMemberTypeMap();

	const MStruct& operator = (const MStruct& var);

protected:

	void AppendStructMember(MStructMember& mem);

private:

	unsigned int m_unByteSize;
	unsigned char* m_pData;
	std::vector<MStructMember> m_vMember;


	static unsigned int s_unPackSize;
};

#endif
