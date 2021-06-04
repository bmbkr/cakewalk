#include "menu.h"

// used: global variables
#include "../global.h"
// used: config variables
#include "../core/variables.h"
// used: actions with config
#include "../core/config.h"
// used: configurations error logging
#include "../utilities/logging.h"
// used: render setup, etc
#include "../utilities/draw.h"
// used: engine, inputsystem, convar interfaces
#include "../core/interfaces.h"
// used: inputtext() wrappers for c++ standard library (stl) type: std::string
#include "../../dependencies/imgui/cpp/imgui_stdlib.h"
#include "../features/walkbot.h"
#include "../sdk/entity.h"

// spectator list, radar, other stuff here
void W::MainWindow(IDirect3DDevice9* pDevice)
{
	if (G::pCmd != nullptr && G::pLocal != nullptr) {
		ImDrawList* pDrawList = ImGui::GetBackgroundDrawList();
		std::vector<std::string> szLines{
			std::string("pCmd->IN_JUMP: ") + ((G::pCmd->iButtons & IN_JUMP) ? "true" : "false"),
			std::string("pCmd->IN_DUCK: ") + ((G::pCmd->iButtons & IN_DUCK) ? "true" : "false"),
			std::string("pLocal->FL_ONGROUND: ") + ((G::pLocal->GetFlags() & FL_ONGROUND) ? "true" : "false"),
			std::string("pLocal->FL_DUCKING: ") + ((G::pLocal->GetFlags() & FL_DUCKING) ? "true" : "false"),
			std::string("flAvgViewDelta: ") + fmt::format("{}", floorf(CWalkBot::Get().GetAvgViewDelta() / 1000.0f) * 1000.0f)
		};

		float flTopOffset = 2.0f;

		for (const auto& line : szLines) {
			const ImVec2 lineSize = ImGui::CalcTextSize(line.c_str());
			pDrawList->AddText({ ImGui::GetIO().DisplaySize.x - 2.0f - lineSize.x + 1.0f, flTopOffset + 2.0f + 1.0f}, 0xff000000, line.c_str());
			pDrawList->AddText({ ImGui::GetIO().DisplaySize.x - 2.0f - lineSize.x, flTopOffset + 2.0f }, 0xffffffff, line.c_str());
			flTopOffset += lineSize.y + 2.0f;
		}
	}

	if (bMainOpened)
	{
		ImGui::Begin("CakeWalk");
		if (ImGui::BeginTabBar("TabBar")) {
			if (ImGui::BeginTabItem("WalkBot")) {
				T::RenderWalkBot();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("AimBot")) {
				T::RenderAimBot();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Misc")) {
				T::RenderMisc();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Config")) {
				T::RenderConfig();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}
}

void T::RenderWalkBot() {
	if (ImGui::Checkbox("Enabled", &C::Get<bool>(Vars.bWalkbot)))
		CWalkBot::Get().Reset();

	ImGui::Checkbox("Humanize", &C::Get<bool>(Vars.bWalkbotHumanize));
	ImGui::Checkbox("Visualize", &C::Get<bool>(Vars.bWalkbotVisualize));
	ImGui::Checkbox("Auto Optimize", &C::Get<bool>(Vars.bWalkbotAutoOptimize));
	ImGui::Checkbox("Look At Point", &C::Get<bool>(Vars.bWalkbotLookAtPoint));
}

void T::RenderAimBot() {
	ImGui::Checkbox("Enabled", &C::Get<bool>(Vars.bAimbot));
	ImGui::Checkbox("Limit Speed", &C::Get<bool>(Vars.bAimbotStep));
	ImGui::Checkbox("Visible Only", &C::Get<bool>(Vars.bAimbotVisibleOnly));
	ImGui::Checkbox("Silent", &C::Get<bool>(Vars.bAimbotSilent));
	ImGui::SliderInt("Min Damage", &C::Get<int>(Vars.iAimbotMinDamage), 1, 100);
	ImGui::Checkbox("Target Chickens", &C::Get<bool>(Vars.bAimbotChickens));
	ImGui::Checkbox("Ignore While Reloading", &C::Get<bool>(Vars.bAimbotIgnoreWhileReloading));
	ImGui::Checkbox("Auto Shoot", &C::Get<bool>(Vars.bAimbotAutoShoot));
	ImGui::Checkbox("Negate Recoil", &C::Get<bool>(Vars.bAimbotNoRecoil));
	ImGui::Combo("Hitgroup", &C::Get<int>(Vars.iAimbotHitGroup), "Best Damage\0Head\0Chest\0Stomach\0\0");
	ImGui::Combo("Selection", &C::Get<int>(Vars.iAimbotEnemySelection), "Nearest\0Highest Health\0Lowest Health\0\0");
}

void T::RenderMisc() {
	ImGui::Checkbox("Auto Buy", &C::Get<bool>(Vars.bMiscAutoBuy));
	ImGui::InputText("Auto Buy String", &C::Get<std::string>(Vars.szMiscAutoBuyString));
	ImGui::Checkbox("Auto Reload", &C::Get<bool>(Vars.bMiscAutoReload));
	ImGui::SliderFloat("Auto Reload Ratio Threshold", &C::Get<float>(Vars.flMiscAutoReloadAmmoThreshold), 0.01f, 0.99f);
}

void T::RenderConfig() {
	if (ImGui::Button("Save"))
		C::Save("default.qo0");
}