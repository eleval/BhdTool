#pragma once

#include "imgui/imgui.h"

#include <algorithm>
#include <array>
#include <string>
#include <unordered_map>

std::string StrToLower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return std::tolower(c); } // correct
	);
	return s;
}

namespace ImGui
{

template<size_t size>
bool FilteredCombo(const char* label, int* currIndex, const std::array<const char*, size>& values)
{
	static std::unordered_map<std::string, char[128]> filters;
	static std::unordered_map<std::string, bool> ignoreFilters;
	char* filter = filters[label];
	const std::string labelInput = "##Input" + std::string(label);
	const float selectableX = ImGui::GetCursorPosX();
	ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - 19.0f);
	float a = ImGui::GetCursorPosX();
	const bool filterUpdated = ImGui::InputText(labelInput.c_str(), filter, 128);
	if (filter[0] == 0)
	{
		ImGui::SameLine();
		ImGui::SetCursorPosX(a);
		ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - 19.0f);
		ImGui::Text(values[*currIndex]);
	}
	ignoreFilters[label] = filterUpdated ? false : ignoreFilters[label];
	bool selectionChanged = false;
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8.0f);
	if (ImGui::BeginCombo(label, values[*currIndex], ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft))
	{
		for (int i = 0; i < static_cast<int>(values.size()); ++i)
		{
			bool isDisplayed = true;
			if (filter[0] != 0 && !ignoreFilters[label])
			{
				isDisplayed = StrToLower(values[i]).find(StrToLower(filter)) != std::string::npos;
			}
			bool isSelected = i == *currIndex;
			if (isDisplayed)
			{
				if (ImGui::Selectable(values[i], &isSelected))
				{
					*currIndex = i;
					selectionChanged = true;
					strcpy(filter, values[i]);
					ignoreFilters[label] = true;
				}
			}
		}
		ImGui::EndCombo();
	}

	return selectionChanged;
}

}