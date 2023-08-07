/**
 * @File         MTaskNodeOutput
 * 
 * @Created      2021-07-08 11:48:17
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Type/MType.h"
#include "Variant/MVariant.h"

class MTaskNode;
class MTaskNodeInput;

class MORTY_API MTaskNodeOutput : public MTypeClass
{
	MORTY_CLASS(MTaskNodeOutput)
public:
	MTaskNodeOutput();

	size_t GetIndex() const { return m_unIndex; }
	MString GetStringID() const;

	MTaskNode* GetTaskNode() const { return pGraphNode; }

	const std::vector<MTaskNodeInput*>& GetLinkedInputs() const { return vLinkedInput; }

	void LinkTo(MTaskNodeInput* pInput);
	void UnLink(MTaskNodeInput* pInput);


	void SetName(const MString& strName) { m_strName = strName; }
	const MString& GetName() const { return m_strName; }

private:

	friend class MTaskNode;

	size_t m_unIndex;
	MString m_strName;

	MTaskNode* pGraphNode;

	std::vector<MTaskNodeInput*> vLinkedInput;
};
