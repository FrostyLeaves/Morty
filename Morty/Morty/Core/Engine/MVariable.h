/**
 * @File         MVariable
 * 
 * @Created      2019-09-01 02:09:49
 *
 * @Author       Morty
**/

#ifndef _M_MVARIABLE_H_
#define _M_MVARIABLE_H_
#include "MGlobal.h"
#include "MString.h"
#include "Vector.h"
#include "Matrix.h"
#include <vector>

class MStruct;
// 一个变量
class MORTY_CLASS MVariable
{
public:

	enum MEVAR_TYPE
	{
		ENone = 0,
		EFloat = 1,
		EVector4 = 2,
		EMatrix4 = 3,
		EStruct = 4
	};

	MVariable();
	MVariable(const float& var);
	MVariable(const Vector4& var);
	MVariable(const Matrix4& var);
	MVariable(const MStruct& var);
	MVariable(const MVariable& var);

	void* GetData();
	unsigned int GetSize() const;
	MEVAR_TYPE GetType(){ return m_eType; }

	MStruct* GetStruct() { return (MStruct*)m_pData; }

	const MVariable& operator = (const MVariable& var);

	~MVariable();

private:

	void Clean();
	unsigned char* m_pData;
	MEVAR_TYPE m_eType;
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
	};

	void AppendVariable(const MString& strName, const MVariable& var);
	void AppendVariable(const MString& strName, const MString& type);

	void SetMember(const MString& strName, const MVariable& var);

	unsigned int GetSize() const { return m_unByteSize; }
	void* GetData();


	const MStruct& operator = (const MStruct& var);

private:

	unsigned int m_unByteSize;
	unsigned char* m_pData;
	std::vector<MStructMember> m_vMember;
};

#endif
