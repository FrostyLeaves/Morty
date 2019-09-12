/**
 * @File         MModel
 * 
 * @Created      2019-08-06 17:49:01
 *
 * @Author       Morty
**/

#ifndef _M_MMODEL_H_
#define _M_MMODEL_H_
#include "MGlobal.h"

#include <vector>

class MIMesh;
class MORTY_CLASS MModel
{
public:
    MModel();
    virtual ~MModel();

public:

	std::vector<MIMesh*>& GetMeshes() { return m_vMeshes; };
	void Clean();

protected:


private:
  
	std::vector<MIMesh*> m_vMeshes;
    

};


#endif
