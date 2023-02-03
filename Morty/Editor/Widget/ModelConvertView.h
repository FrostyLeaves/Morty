#ifndef _MODEL_CONVERT_VIEW_H_
#define _MODEL_CONVERT_VIEW_H_

#include <map>
#include <queue>
#include <functional>

#include "Utility/MString.h"
#include "Model/MModelConverter.h"


#include "Main/IBaseView.h"

class IBaseView;
class ModelConvertView : public IBaseView
{
public:
	ModelConvertView();
	virtual ~ModelConvertView();

public:
	
	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;
	virtual void Input(MInputEvent* pEvent) override;

public:

	void Convert(std::queue<MModelConvertInfo> queue);

private:

	std::queue<MModelConvertInfo> m_convertQueue;

	std::string m_strSourcePath;
	std::string m_strOutputDir;
	std::string m_strOutputName;
	int m_nMaterialTypeEnum;


	MEngine* m_pEngine;
};


#endif