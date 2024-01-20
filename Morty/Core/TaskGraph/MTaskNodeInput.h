/**
 * @File         MTaskNodeInput
 * 
 * @Created      2021-07-08 11:50:37
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"
#include "Type/MType.h"

class MTaskNode;
class MTaskNodeOutput;

class MORTY_API MTaskNodeInput : public MTypeClass
{
	MORTY_CLASS(MTaskNodeInput)
public:

	MTaskNodeInput();

	void LinkTo(MTaskNodeOutput* pOutput);

	void UnLink();


	size_t GetIndex() const { return m_unIndex; }
	MString GetStringID() const;

	void SetName(const MString& strName) { m_strName = strName; }
	const MString& GetName() const { return m_strName; }

	MTaskNode* GetTaskNode() const { return pGraphNode; }
	MTaskNode* GetLinkedNode() const;
	MTaskNodeOutput* GetLinkedOutput() const { return pLinkedOutput; }

private:
	friend class MTaskNode;
	friend class MTaskNodeOutput;

private:
	size_t m_unIndex;
	MString m_strName;
	MTaskNode* pGraphNode;
	MTaskNodeOutput* pLinkedOutput;
};
