#pragma once

#include "Utility/MRenderGlobal.h"
#include "Math/Vector.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMaterialTemplateResource.h"
#include "Scene/MEntity.h"
#include "System/MResourceSystem.h"
#include "Utility/MColor.h"
#include "Utility/MString.h"
#include "Utility/MTransform.h"
#include "Variant/MVariant.h"

#include <any>
#include <cstdint>
#include <functional>
#include <map>

namespace morty
{

class MShaderPropertyBlock;
class MEntity;
class MObject;
class MMaterial;
class MResource;
class MTexture;
class PropertyBase
{
public:
    virtual ~PropertyBase() = default;

    bool ShowNodeBegin(const MString& strNodeName);

    bool ShowNodeBeginWithEx(const MString& strNodeName);

    void ShowNodeExBegin(const MString& strExID);

    void ShowNodeExEnd();

    void ShowNodeEnd();

    void ShowValueBegin(const MString& strValueName);

    void ShowValueEnd();

    //normal
    bool Editbool(bool& value);

    bool Editbool(int& value);

    bool Editfloat(float& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);

    bool EditVector2(Vector2& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);

    bool EditVector3(Vector3& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);

    bool EditVector3(float* pValue, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);

    bool EditVector4(Vector4& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);

    bool EditVector4(float* pValue, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);

    bool EditMTransform(MTransform& trans);

    bool EditEnum(const std::vector<MString>& select, size_t& index);

    bool EditMColor(MColor& value);

    bool EditMString(MString& value);

    bool EditMMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& pMaterial);

    bool EditMMaterial(std::shared_ptr<MMaterial> pMaterial);

    bool EditShaderProperty(const std::shared_ptr<MShaderPropertyBlock>& pProperty);

    bool EditMResource(
            const MString&              strDlgID,
            const MString&              strResourceType,
            const std::vector<MString>& vSuffixList,
            std::shared_ptr<MResource>  pDefaultResource
    );

    bool EditMMaterialTemplateResource(std::shared_ptr<MMaterialTemplateResource>& resource);
    bool EditMMaterialResource(std::shared_ptr<MMaterialResource>& resource);

    void EditSaveMResource(
            const MString&              stringID,
            const MString&              strResourceType,
            const std::vector<MString>& vSuffixList,
            std::shared_ptr<MResource>  pResource
    );

    void                         ShowTexture(MTexturePtr pTexture, const Vector2& v2Size);

    //auto call ShowValue/ShowNode
    bool                         EditMVariant(const MString& strVariantName, MVariant& value);

    unsigned int                 GetID(const MString& strItemName);

    template<typename TYPE> TYPE GetTemporaryValue(const MString& strValueName, const TYPE& defaultValue);

    template<typename TYPE> void SetTemporaryValue(const MString& strValueName, const TYPE& valuealue);

    void                         BindEngine(MEngine* pEngine) { m_engine = pEngine; }

private:
    static unsigned int                    m_unItemIDPool;
    static std::map<MString, unsigned int> m_itemID;

    MEngine*                               m_engine = nullptr;
    std::map<MString, std::any>            m_temporaryValue;
};

template<typename TYPE> TYPE PropertyBase::GetTemporaryValue(const MString& strValueName, const TYPE& defaultValue)
{
    if (m_temporaryValue.find(strValueName) == m_temporaryValue.end())
    {
        m_temporaryValue[strValueName] = defaultValue;
        return defaultValue;
    }

    try
    {
        TYPE result = std::any_cast<TYPE>(m_temporaryValue[strValueName]);
        return result;
    } catch (const std::bad_any_cast& e)
    {
        return defaultValue;
    }

    return defaultValue;
}

template<typename TYPE> void PropertyBase::SetTemporaryValue(const MString& strValueName, const TYPE& valuealue)
{
    m_temporaryValue[strValueName] = valuealue;
}

}// namespace morty