#include "imgui.h"

namespace c
{

	inline ImVec4 accent = ImColor(87, 17, 250);
	inline ImVec4 accent_inclined = ImColor(0, 42, 104);

	inline ImVec4 red = ImColor(255, 0, 0);

	namespace bg
	{
		inline ImVec4 background = ImColor(12, 14, 17, 150);
		inline ImVec4 line = ImColor(55, 60, 68, 20);

		inline ImVec2 size = ImVec2(600, 700);
		inline float rounding = 12;
	}

	namespace text
	{
		inline ImVec4 text_active = ImColor(255, 255, 255, 255);
		inline ImVec4 text = ImColor(81, 93, 124, 255);
	}

	namespace widget
	{
		inline ImVec4 background = ImColor(0, 0, 0, 50);
		inline float rounding = 6;
	}
}