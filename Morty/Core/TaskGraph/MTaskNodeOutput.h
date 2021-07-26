/**
 * @File         MTaskNodeOutput
 * 
 * @Created      2021-07-08 11:48:17
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTASKNODEOUTPUT_H_
#define _M_MTASKNODEOUTPUT_H_
#include "MGlobal.h"
#include "MType.h"
#include "MVariant.h"

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
	MString GetName() const { return m_strName; }

	template<typename TYPE>
	TYPE* GetValue(const MString& strName);

	template<typename TYPE>
	void SetValue(const MString& strName, const TYPE& value);

public:

	MStruct m_outputValue;

private:

	friend class MTaskNode;

	size_t m_unIndex;
	MString m_strName;

	MTaskNode* pGraphNode;

	std::vector<MTaskNodeInput*> vLinkedInput;
};

template<typename TYPE>
TYPE* MTaskNodeOutput::GetValue(const MString& strName)
{
	return m_outputValue.FindMember<TYPE>(strName);
}

template<typename TYPE>
void MTaskNodeOutput::SetValue(const MString& strName, const TYPE& value)
{
	m_outputValue.AppendMVariant(strName, value);
}

#endif
