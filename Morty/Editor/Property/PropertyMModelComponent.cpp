#include "Property/PropertyMModelComponent.h"
#include "Model/MSkeletalAnimation.h"
#include "Resource/MSkeletalAnimationResource.h"

#include "imgui.h"

using namespace morty;

void PropertyMModelComponent::EditAnimation(MModelComponent* pModelComponent)
{
    m_editProperty.BindEngine(pModelComponent->GetEngine());

    MSkeletalAnimController* pController          = pModelComponent->GetSkeletalAnimationController();
    auto                     pCurrentAnimResource = pController ? pController->GetAnimationResource() : nullptr;

    static auto              ModelLoadFunc = [&pModelComponent](const MString& strNewFilePath) {
        MSkeletalAnimController*                pOldController = pModelComponent->GetSkeletalAnimationController();
        bool                                    bLoop    = false;
        float                                   fPercent = 0.0f;
        MIAnimController::MEAnimControllerState state    = MIAnimController::EStop;
        if (pOldController)
        {
            bLoop    = pOldController->GetLoop();
            fPercent = pOldController->GetPercent();
            state    = pOldController->GetState();
        }
        pModelComponent->PlayAnimation(strNewFilePath);
        MSkeletalAnimController* pController = pModelComponent->GetSkeletalAnimationController();

        if (pOldController)
        {
            pController->SetLoop(bLoop);
            pController->SetPercent(fPercent);
            pController->NextStep(0.0f);
            if (MIAnimController::EPlay == state) pController->Play();
        }
    };
    m_editProperty.ShowValueBegin("Animation");
    if (m_editProperty.EditMResource(
                "skelanim_file_dlg",
                MSkeletalAnimationLoader::GetResourceTypeName(),
                MSkeletalAnimationLoader::GetSuffixList(),
                pCurrentAnimResource
        ))
    {
        ModelLoadFunc(pCurrentAnimResource->GetResourcePath());
    }

    m_editProperty.ShowValueEnd();

    pController = pModelComponent->GetSkeletalAnimationController();
    if (pController)
    {
        m_editProperty.ShowValueBegin("Loop");
        bool bLoop = pController->GetLoop();
        if (m_editProperty.Editbool(bLoop)) { pController->SetLoop(bLoop); }
        m_editProperty.ShowValueEnd();


        m_editProperty.ShowValueBegin("State");


        float width = ImGui::GetContentRegionAvail().x;

        float fPercent = pController->GetPercent();
        ImGui::SetNextItemWidth(width * 0.75f);
        if (ImGui::SliderFloat("", &fPercent, 0.0f, 100.0f))
        {
            pController->SetPercent(fPercent);
            pController->NextStep(0.0f);
            pController->Pause();
        }

        ImGui::SameLine();

        if (pController->GetState() == MIAnimController::EPlay)
        {
            if (ImGui::Button("Pause", ImVec2(width * 0.25f, 0.0f))) pController->Pause();
        }
        else
        {
            if (ImGui::Button("Play", ImVec2(width * 0.25f, 0.0f))) pController->Play();
        }

        m_editProperty.ShowValueEnd();
    }
}
