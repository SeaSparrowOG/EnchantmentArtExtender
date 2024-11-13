#pragma once

#include "RE/N/NiSmartPointer.h"

namespace RE
{
	class Actor;
	class ActorMagicCaster;
	class TESObjectWEAP;

	class WeaponEnchantmentController : public ReferenceEffectController
	{
	public:
		virtual ~WeaponEnchantmentController() = default;  // 00

		// override
		RE::TESObjectREFR*   GetTargetReference() override;          // 0B
		RE::BGSArtObject*    GetHitEffectArt() override;             // 0C
		RE::TESEffectShader* GetHitEffectShader() override;          // 0D
		bool                 GetManagerHandlesSaveLoad() override;   // 0E
		RE::NiAVObject*      GetAttachRoot() override;               // 0F
		float                GetParticleAttachExtent() override;     // 10
		bool                 GetUseParticleAttachExtent() override;  // 11
		bool                 GetDoParticles() override;              // 12
		bool                 GetParticlesUseLocalSpace() override;   // 13
		bool                 GetUseRootWorldRotate() override;       // 14
		bool                 GetIsRootActor() override;              // 15
		bool                 GetShaderUseParentCell() override;      // 19
		bool                 GetAllowTargetRoot() override;          // 2C
		bool                 IsReadyForAttach() override;            // 1D

		// members
		ActorMagicCaster*     magicCaster;   // 08
		Actor*                target;        // 10
		TESEffectShader*      effectShader;  // 18
		BGSArtObject*         artObject;     // 20
		NiPointer<NiAVObject> attachRoot;    // 28
		TESObjectWEAP*        weapon;        // 30
		bool                  firstPerson;   // 38
		std::byte             pad39[7];      // 39
	};
	static_assert(sizeof(WeaponEnchantmentController) == 0x40);
}