#ifndef _GUI_NODE_H_
#define _GUI_NODE_H_

#include "MNode.h"
class GUINode : public MNode
{
public:

	GUINode();
	virtual ~GUINode();

protected:

	void DrawGUI(class ImDrawData* pDrawData);


	void InitImGUI();
	bool InitImGUIDX11();
};


#endif