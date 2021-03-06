#include "button.hpp"
#include "engine/gui/gui_manager.hpp"
#include "engine/graphics/interface/window.hpp"
#include "engine/utility/extra.hpp"
#include "engine/utility/connect_guard_additions.hpp"



namespace oe::gui
{
	Button::Button(Widget* parent, GUI& gui_manager, const info_t& _button_info)
		: Widget(parent, gui_manager, static_cast<const Widget::info_t&>(_button_info))
		, button_info(_button_info)
	{
	}
	
	void Button::virtual_toggle(bool enabled)
	{
		if(enabled)
		{
			// event listeners
			m_cg_cursor.connect<CursorPosEvent, &Button::on_cursor, Button>(m_gui_manager.m_dispatcher, this);
			m_cg_button.connect<MouseButtonEvent, &Button::on_button, Button>(m_gui_manager.m_dispatcher, this);
		}
		else
		{
			// event listeners
			m_cg_cursor.disconnect();
			m_cg_button.disconnect();
		}
	}

	bool Button::test(const glm::vec2& point)
	{
		return oe::utils::bounding_box_test(point, m_render_position, m_render_size);
	}
		
	void Button::on_cursor(const CursorPosEvent& event)
	{
		if(!m_cg_cursor)
			return;

		if (test(event.cursor_windowspace))
		{
			m_dispatcher.trigger(event_hover_latest);
		}
	}

	void Button::on_button(const MouseButtonEvent& event)
	{
		if(!m_cg_button)
			return;

		if (test(event.cursor_pos.cursor_windowspace))
		{
			event_use_latest.action = event.action;
			event_use_latest.button = event.button;
			event_use_latest.modifier = event.mods;
			m_dispatcher.trigger(event_use_latest);
		}
	}

}