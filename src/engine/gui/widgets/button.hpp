#pragma once

#include "widget.hpp"
#include "engine/enum.hpp"



namespace oe::gui
{
	struct ButtonInfo
	{
		WidgetInfo widget_info = { { 50, 50 }, { 0, 0 }, oe::alignments::center_center, oe::alignments::center_center };
	};

	struct ButtonHoverEvent
	{};

	struct ButtonUseEvent
	{
		oe::actions action;
		oe::mouse_buttons button;
		oe::modifiers modifier;
	};

	class Button : public Widget
	{
	public:
		ButtonInfo button_info;
		ButtonHoverEvent event_hover_latest;
		ButtonUseEvent event_use_latest;

	public:
		Button(Widget* parent, GUI& gui_manager, const ButtonInfo& button_info = {});
		~Button();

		bool test(const glm::vec2& point);

		virtual void virtual_toggle(bool enabled) override;
	
	private:
		// events
		void on_cursor(const CursorPosEvent& event);
		void on_button(const MouseButtonEvent& event);
		oe::utils::connect_guard m_cg_cursor;
		oe::utils::connect_guard m_cg_button;
	};

}