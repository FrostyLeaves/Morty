/**
 * @File         MEntityResource
 * 
 * @Created      2021-07-20 10:48:24
 *
 * @Author       DoubleYe
**/

#ifndef _M_MENTITYRESOURCE_H_
#define _M_MENTITYRESOURCE_H_
#include "Utility/MGlobal.h"

#include "Resource/MResource.h"

class MORTY_API MEntityResource : public MResource
{
    MORTY_CLASS(MEntityResource)
public:
    MEntityResource();
    virtual ~MEntityResource();

public:

	static MString GetResourceTypeName() { return "Entity"; }
	static std::vector<MString> GetSuffixList() { return { "entity" }; }

    std::vector<MByte>& GetData() { return m_entityData; }

protected:

    virtual bool Load(const MString& strResourcePath) override;
    virtual bool SaveTo(const MString& strResourcePath) override;

private:

    friend class MEntitySystem;
    std::vector<MByte> m_entityData;
};


#endif
