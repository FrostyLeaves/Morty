#include "PropertyMModelInstance.h"
#include "MSkeletalAnimation.h"

#include "imgui.h"

void PropertyMModelInstance::EditAnimation(MModelInstance* pNode)
{
	
	MSkeletalAnimController* pController = pNode->GetSkeletalAnimationController();
	MSkeletalAnimation* pCurrentAnimResource = pController ? pController->GetAnimation() : nullptr;

	static auto ModelLoadFunc = [&pNode](const MString& strNewFilePath) {

		MSkeletalAnimController* pOldController = pNode->GetSkeletalAnimationController();
		bool bLoop = false;
		float fPercent = 0.0f;
		MIAnimController::MEAnimControllerState state = MIAnimController::EStop;
		if (pOldController)
		{
			bLoop = pOldController->GetLoop();
			fPercent = pOldController->GetPercent();
			state = pOldController->GetState();
		}
		pNode->SetPlayAnimation(strNewFilePath);
		MSkeletalAnimController* pController = pNode->GetSkeletalAnimationController();

		if (pOldController)
		{
			pController->SetLoop(bLoop);
			pController->SetPercent(fPercent);
			pController->Update(0.0f);
			if (MIAnimController::EPlay == state)
				pController->Play();
		}
	};
	ShowValueBegin("Animation");
	EditMResource("skelanim_file_dlg", pCurrentAnimResource, MEResourceType::SkelAnim, ModelLoadFunc);

	ShowValueEnd();

	if (pController = pNode->GetSkeletalAnimationController())
	{
		ShowValueBegin("Loop");
		bool bLoop = pController->GetLoop();
		if (Editbool(bLoop))
		{
			pController->SetLoop(bLoop);
		}
		ShowValueEnd();


		ShowValueBegin("State");


		float width = ImGui::GetContentRegionAvailWidth();

		float fPercent = pController->GetPercent();
		ImGui::SetNextItemWidth(width * 0.75f);
		if (ImGui::SliderFloat("", &fPercent, 0.0f, 100.0f))
		{
			pController->SetPercent(fPercent);
			pController->Update(0.0f);
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
