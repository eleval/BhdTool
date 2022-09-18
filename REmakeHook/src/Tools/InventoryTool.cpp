#include "pch.h"

#include "Tools/InventoryTool.h"

#include "Game/Items.h"
#include "Hooks/Hooks.h"

#include "imgui/imgui.h"

#include <string>

namespace
{
	constexpr const int MaxInvSlots = 8;
	struct InventorySlot
	{
		int itemId;
		int quantity;
	};

	struct Inventory
	{
		uint8_t padding[0x38];
		InventorySlot slots[MaxInvSlots];
	};

	Inventory* inventory_;
}

void InventoryTool::UpdateUI()
{
	if (ImGui::CollapsingHeader("Inventory"))
	{
		SIZE_T bytesRead;
		ReadProcessMemory(GetCurrentProcess(), (void*)0x00d7c9c0, &inventory_, sizeof(Inventory*), &bytesRead);

		if (inventory_ != nullptr)
		{
			for (int i = 0; i < MaxInvSlots; ++i)
			{
				const std::string label = "Slot " + std::to_string(i);
				const std::string comboLabel = "##SlotCombo" + std::to_string(i);
				const std::string quantityLabel = "##SlotQuantity" + std::to_string(i);
				ImGui::Text(label.c_str());
				ImGui::SameLine();
				ImGui::Combo(comboLabel.c_str(), &inventory_->slots[i].itemId, ItemNames.data(), ItemCount);
				ImGui::SameLine();
				ImGui::InputInt(quantityLabel.c_str(), &inventory_->slots[i].quantity);
			}
		}
	}

	if (ImGui::Button("Try Save"))
	{
		Hooks::TrySave();
	}
}