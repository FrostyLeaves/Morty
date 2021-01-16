/**
 * @File         MProject
 * 
 * @Created      2020-06-02 01:08:02
 *
 * @Author       Pobrecito
**/

#ifndef _M_MPROJECT_H_
#define _M_MPROJECT_H_
#include "MGlobal.h"

#include <vector>

class MORTY_API MProject
{
public:
    MProject();
    virtual ~MProject();

public:

    void SetWorkPath(const MString& strWorkPath) { m_strWorkPath = strWorkPath; }
    MString GetWorkPath() const { return m_strWorkPath; }

private:

    MString m_strWorkPath;
};

#endif
