#include "PropertyMModelInstance.h"
#include "MSkeletalAnimation.h"

#include "imgui.h"

void PropertyMModelInstance::EditAnimation(MModelInstance* pNode)
{
	if (MModelResource* pModelResource = pNode->GetResource())
	{
		MSkeletalAnimController* pController = pNode->GetSkeletalAnimationController();
		const std::vector<MString>& vAnimationsList = *pModelResource->GetAnimationsName();

		unsigned int unCurrentAnimationIndex = 0;
		if (pController)
		{
			if (MSkeletalAnimation* pAnimation = pController->GetAnimation())
			{
				unCurrentAnimationIndex = pAnimation->GetIndex();
			}
		}
		if (!vAnimationsList.empty())
		{
			if (!pController)
			{
				pNode->SetPlayAnimation((*pModelResource->GetAnimationsName())[unCurrentAnimationIndex]);
				pController = pNode->GetSkeletalAnimationController();
			}

			ShowValueBegin("Animations");
			if (EditEnum(vAnimationsList, unCurrentAnimationIndex))
			{
				bool bLoop = pController->GetLoop();
				float fPercent = pController->GetPercent();
				MIAnimController::MEAnimControllerState state = pController->GetState();

				pNode->SetPlayAnimation((*pModelResource->GetAnimationsName())[unCurrentAnimationIndex]);
				pController = pNode->GetSkeletalAnimationController();

				pController->SetLoop(bLoop);
				pController->SetPercent(fPercent);
				pController->Update(0.0f);
				if (MIAnimController::EPlay == state)
					pController->Play();
			}
			ShowValueEnd();

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

	}
}
