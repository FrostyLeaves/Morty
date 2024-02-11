#include "Property/PropertyMModelComponent.h"
#include "Model/MSkeletalAnimation.h"
#include "Resource/MSkeletalAnimationResource.h"

#include "imgui.h"

using namespace morty;

void PropertyMModelComponent::EditAnimation(MModelComponent* pModelComponent)
{
	
	MSkeletalAnimController* pController = pModelComponent->GetSkeletalAnimationController();
	auto pCurrentAnimResource = pController ? pController->GetAnimationResource() : nullptr;

	static auto ModelLoadFunc = [&pModelComponent](const MString& strNewFilePath) {

		MSkeletalAnimController* pOldController = pModelComponent->GetSkeletalAnimationController();
		bool bLoop = false;
		float fPercent = 0.0f;
		MIAnimController::MEAnimControllerState state = MIAnimController::EStop;
		if (pOldController)
		{
			bLoop = pOldController->GetLoop();
			fPercent = pOldController->GetPercent();
			state = pOldController->GetState();
		}
		pModelComponent->PlayAnimation(strNewFilePath);
		MSkeletalAnimController* pController = pModelComponent->GetSkeletalAnimationController();

		if (pOldController)
		{
			pController->SetLoop(bLoop);
			pController->SetPercent(fPercent);
			pController->NextStep(0.0f);
			if (MIAnimController::EPlay == state)
				pController->Play();
		}
	};
	ShowValueBegin("Animation");
	EditMResource("skelanim_file_dlg", MSkeletalAnimationLoader::GetResourceTypeName(), MSkeletalAnimationLoader::GetSuffixList(), pCurrentAnimResource, ModelLoadFunc);

	ShowValueEnd();

	pController = pModelComponent->GetSkeletalAnimationController();
	if (pController)
	{
		ShowValueBegin("Loop");
		bool bLoop = pController->GetLoop();
		if (Editbool(bLoop))
		{
			pController->SetLoop(bLoop);
		}
		ShowValueEnd();


		ShowValueBegin("State");


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
			if (ImGui::Button("Pause", ImVec2(width * 0.25f, 0.0f)))
				pController->Pause();
		}
		else
		{
			if (ImGui::Button("Play", ImVec2(width * 0.25f, 0.0f)))
				pController->Play();
		}

		ShowValueEnd();
	}

}
