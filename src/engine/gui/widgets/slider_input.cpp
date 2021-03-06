#include "slider_input.hpp"

#include "engine/asset/default_shader/default_shader.hpp"
#include "engine/graphics/textLabel.hpp"
#include "engine/graphics/interface/window.hpp"
#include "engine/graphics/interface/texture.hpp"
#include "engine/graphics/renderer.hpp"
#include "engine/utility/fileio.hpp"
#include "engine/utility/connect_guard_additions.hpp"



namespace oe::gui
{
	class SliderLinearRenderer
	{
	private:
		static SliderLinearRenderer* singleton;
		SliderLinearRenderer(const SliderLinearRenderer& copy) = delete;
		SliderLinearRenderer()
			: s_shader()
			, s_renderer(oe::RendererInfo{ 1 })
		{
		}

	public:
		static SliderLinearRenderer& getSingleton() { if (!singleton) singleton = new SliderLinearRenderer(); return *singleton; }

		oe::asset::DefaultShader s_shader;
		oe::graphics::PrimitiveRenderer s_renderer;
	};
	SliderLinearRenderer* SliderLinearRenderer::singleton = nullptr;

	

	BaseBasicSliderInput::BaseBasicSliderInput(Widget* parent, GUI& gui_manager, const BaseBasicSliderInputInfo_t& info)
		: Widget(parent, gui_manager, info.widget_info)
	{}
	
	BaseBasicSliderInput::~BaseBasicSliderInput()
	{}
	
	void BaseBasicSliderInput::virtual_toggle(bool enabled)
	{
		if(enabled)
		{
			label_quad = m_gui_manager.getRenderer()->create();
			value_label = std::make_unique<oe::graphics::TextLabel>();
			quad_knob = m_gui_manager.getRenderer()->create();
			if (!get_info().linear_color)
			{
				quad_lslider = m_gui_manager.getRenderer()->create();
				quad_rslider = m_gui_manager.getRenderer()->create();
			}

			// event listeners
			m_cg_render.connect<GUIRenderEvent, &BaseBasicSliderInput::on_render, BaseBasicSliderInput>(m_gui_manager.m_dispatcher, this);
			m_cg_cursor.connect<CursorPosEvent, &BaseBasicSliderInput::on_cursor, BaseBasicSliderInput>(m_gui_manager.m_dispatcher, this);
			m_cg_button.connect<MouseButtonEvent, &BaseBasicSliderInput::on_button, BaseBasicSliderInput>(m_gui_manager.m_dispatcher, this);
			m_cg_scroll.connect<ScrollEvent, &BaseBasicSliderInput::on_scroll, BaseBasicSliderInput>(m_gui_manager.m_dispatcher, this);
		}
		else
		{
			label_quad.reset();
			value_label.reset();
			quad_knob.reset();
			if (!get_info().linear_color)
			{
				quad_lslider.reset();
				quad_rslider.reset();
			}

			// event listeners
			m_cg_render.disconnect();
			m_cg_cursor.disconnect();
			m_cg_button.disconnect();
			m_cg_scroll.disconnect();
		}
	}
	
	void BaseBasicSliderInput::on_render(const GUIRenderEvent& /* event */)
	{
		if(!m_cg_render)
			return;

		if (!get_info().linear_color)
		{
			if (get_info().vertical)
			{
				const glm::ivec2 slider_pos = {
					0.0f,
					get_rendered_value() * static_cast<float>(m_render_size.y - get_info().knob_size.y)
				};
				
				{
					quad_lslider->setPosition(m_render_position);
					quad_lslider->setZ(m_z);
					quad_lslider->setSize({ m_render_size.x, slider_pos.y });
					quad_lslider->setColor(get_info().slider_lcolor);
					quad_lslider->setSprite(get_info().slider_sprite);
				}
				{
					quad_rslider->setPosition({ m_render_position.x, m_render_position.y + slider_pos.y });
					quad_rslider->setZ(m_z + 0.025f);
					quad_rslider->setSize({ m_render_size.x, m_render_size.y - slider_pos.y });
					quad_rslider->setColor(get_info().slider_rcolor);
					quad_rslider->setSprite(get_info().slider_sprite);
				}

				quad_knob->setPosition(static_cast<glm::vec2>(m_render_position + static_cast<glm::ivec2>(slider_pos)));
			}
			else
			{
				const glm::ivec2 slider_pos = {
					get_rendered_value() * static_cast<float>(m_render_size.x - get_info().knob_size.x),
					0.0f
				};

				{
					quad_lslider->setPosition(m_render_position);
					quad_lslider->setZ(m_z);
					quad_lslider->setSize({ slider_pos.x, m_render_size.y });
					quad_lslider->setColor(get_info().slider_lcolor);
					quad_lslider->setSprite(get_info().slider_sprite);
				}
				{
					quad_rslider->setPosition({ m_render_position.x + slider_pos.x, m_render_position.y });
					quad_rslider->setZ(m_z + 0.025f);
					quad_rslider->setSize({ m_render_size.x - slider_pos.x, m_render_size.y });
					quad_rslider->setColor(get_info().slider_rcolor);
					quad_rslider->setSprite(get_info().slider_sprite);
				}

				quad_knob->setPosition(static_cast<glm::vec2>(m_render_position + static_cast<glm::ivec2>(slider_pos)));
			}
			
			quad_knob->setZ(m_z + 0.05f);
			quad_knob->setSize(get_info().knob_size);
			quad_knob->setColor(get_info().knob_color);
			quad_knob->setSprite(get_info().knob_sprite);
		}
		else
		{
			auto& renderer = SliderLinearRenderer::getSingleton();
			renderer.s_renderer->clear();
			renderer.s_renderer->begin();
			if (get_info().vertical)
			{
				std::array<oe::graphics::VertexData, 4> vertices = 
				{{ 
					{ { m_render_position.x + 0,              m_render_position.y + 0,              m_z }, { get_info().slider_sprite->position.x + 0,                                 get_info().slider_sprite->position.y + 0 },                                 get_info().slider_lcolor },
					{ { m_render_position.x + 0,              m_render_position.y + m_render_size.y, m_z }, { get_info().slider_sprite->position.x + 0,                                 get_info().slider_sprite->position.y + get_info().slider_sprite->size.y }, get_info().slider_rcolor },
					{ { m_render_position.x +m_render_size.x, m_render_position.y + m_render_size.y, m_z }, { get_info().slider_sprite->position.x + get_info().slider_sprite->size.x, get_info().slider_sprite->position.y + get_info().slider_sprite->size.y }, get_info().slider_rcolor },
					{ { m_render_position.x +m_render_size.x, m_render_position.y + 0,              m_z }, { get_info().slider_sprite->position.x + get_info().slider_sprite->size.x, get_info().slider_sprite->position.y + 0 },                                 get_info().slider_lcolor },
				}};

				renderer.s_renderer->submitVertex(vertices);

				const glm::ivec2 slider_pos = {
					0.0f,
					get_rendered_value() * static_cast<float>(m_render_size.y - get_info().knob_size.y)
				};
				quad_knob->setPosition(static_cast<glm::vec2>(m_render_position + static_cast<glm::ivec2>(slider_pos)));
			}
			else
			{
				std::array<oe::graphics::VertexData, 4> vertices =
				{{
					{ { m_render_position.x + 0,               m_render_position.y + 0,               m_z }, { get_info().slider_sprite->position.x + 0,                                 get_info().slider_sprite->position.y + 0 },                                 get_info().slider_lcolor },
					{ { m_render_position.x + 0,               m_render_position.y + m_render_size.y, m_z }, { get_info().slider_sprite->position.x + 0,                                 get_info().slider_sprite->position.y + get_info().slider_sprite->size.y }, get_info().slider_lcolor },
					{ { m_render_position.x + m_render_size.x, m_render_position.y + m_render_size.y, m_z }, { get_info().slider_sprite->position.x + get_info().slider_sprite->size.x, get_info().slider_sprite->position.y + get_info().slider_sprite->size.y }, get_info().slider_rcolor },
					{ { m_render_position.x + m_render_size.x, m_render_position.y + 0,              m_z }, { get_info().slider_sprite->position.x + get_info().slider_sprite->size.x, get_info().slider_sprite->position.y + 0 },                                 get_info().slider_rcolor },
				}};

				renderer.s_renderer->submitVertex(vertices);

				const glm::ivec2 slider_pos = {
					get_rendered_value() * static_cast<float>(m_render_size.x - get_info().knob_size.x),
					0.0f
				};
				quad_knob->setPosition(static_cast<glm::vec2>(m_render_position + static_cast<glm::ivec2>(slider_pos)));
			}

			quad_knob->setZ(m_z + 0.05f);
			quad_knob->setSize(get_info().knob_size);
			quad_knob->setColor(get_info().knob_color);
			quad_knob->setSprite(get_info().knob_sprite);

			get_info().slider_sprite->m_owner->bind();
			m_gui_manager.getShader()->bind();

			renderer.s_renderer->end();
			renderer.s_renderer->render();
		}

		// value
		const auto& text_options = get_info().text_options;
		label_quad->toggle(text_options.enabled);
		if (text_options.enabled)
		{
			const std::u32string s = get_rendered_label();
			oe::graphics::text_render_cache cache{};
			cache.create<char32_t>({ s, text_options.initial_text_color }, m_gui_manager.getFont(text_options.font), text_options);
			value_label->generate(cache);
			
			const glm::ivec2 size = value_label->size();
			const glm::ivec2 position =
				+ m_render_position
				+ oe::alignmentOffset(m_render_size, text_options.align)
				- oe::alignmentOffset(size, text_options.align);

			label_quad->setPosition(static_cast<glm::vec2>(position));
			label_quad->setZ(m_z + 0.075f);
			label_quad->setSize(static_cast<glm::vec2>(size));
			label_quad->setSprite(value_label->sprite());
			label_quad->setColor(oe::colors::white);
		}
	}

	void BaseBasicSliderInput::on_cursor(const CursorPosEvent& event)
	{
		if(!static_cast<bool>(get_info().interact_flags & interact_type_flags::cursor))
			return;

		if (oe::utils::bounding_box_test(event.cursor_windowspace, m_render_position, m_render_size))
			send_hover_event();

		if (m_dragging) {
			if (get_info().vertical)
			{
				const float value = static_cast<float>(event.cursor_windowspace.y - m_render_position.y - (get_info().knob_size.y / 2))
				                    / static_cast<float>(m_render_size.y - get_info().knob_size.y);
				set_rendered_value(value);
			}
			else
			{
				const float value = static_cast<float>(event.cursor_windowspace.x - m_render_position.x - (get_info().knob_size.x / 2))
				                    / static_cast<float>(m_render_size.x - get_info().knob_size.x);
				set_rendered_value(value);
			}
			clamp();
			send_use_event();
		}
	}

	void BaseBasicSliderInput::on_button(const MouseButtonEvent& event)
	{
		if(!static_cast<bool>(get_info().interact_flags & interact_type_flags::cursor))
			return;

		auto set_dragging_send_event = [this, &event](bool newstate){
			m_dragging = newstate;

			auto& use_e = get_use_event();
			use_e.action = event.action;
			use_e.button = event.button;
			use_e.modifier = event.mods;
			send_use_event();

			on_cursor(event.cursor_pos);
		};

		if (oe::utils::bounding_box_test(event.cursor_pos.cursor_windowspace, m_render_position, m_render_size))
		{
			// hold
			if (event.button == oe::mouse_buttons::button_left && event.action != oe::actions::release) // press or repeat
				set_dragging_send_event(true);
		}

		// release
		if (event.button == oe::mouse_buttons::button_left && event.action == oe::actions::release)
			set_dragging_send_event(false);
	}

	void BaseBasicSliderInput::on_scroll(const ScrollEvent& event)
	{
		if (!static_cast<bool>(get_info().interact_flags & interact_type_flags::scroll))
			return;

		const glm::vec2& cursor_window = m_gui_manager.getWindow()->getCursorWindow();
		if (oe::utils::bounding_box_test(cursor_window, m_render_position, m_render_size))
		{
			step(event.scroll_delta.y);
			clamp();

			auto& use_e = get_use_event();
			use_e.action = oe::actions::none;
			use_e.button = oe::mouse_buttons::none;
			use_e.modifier = oe::modifiers::none;
			send_use_event();
		}
	}

}