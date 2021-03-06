#include "color_picker.hpp"

#include "engine/interfacegen_renderer.hpp"

#include "engine/gui/gui_manager.hpp"
#include "engine/gui/widgets/slider_input.hpp"

#include "engine/graphics/interface/index_buffer_gen.hpp"
#include "engine/graphics/interface/framebuffer.hpp"
#include "engine/graphics/interface/window.hpp"
#include "engine/graphics/interface/shader.hpp"
#include "engine/graphics/spritePacker.hpp"
#include "engine/utility/connect_guard_additions.hpp"

#include "engine/asset/default_shader/default_shader.hpp"
#include "engine/asset/texture_set/texture_set.hpp"
#include "engine/asset/asset_loader.hpp"



namespace oe::gui
{
	bool in_circle(const glm::vec2& p, float width)
	{
		float l = glm::length(p);
		return l < 1.0f && l > 1.0f - width * 2;
	}

	float sign(const std::array<glm::vec2, 3>& points)
	{
		return (points[0].x - points[2].x) * (points[1].y - points[2].y) - (points[1].x - points[2].x) * (points[0].y - points[2].y);
	}

	bool in_triangle(const glm::vec2& p, const std::array<glm::vec2, 3>& triangle)
	{
		float d1, d2, d3;
		bool has_neg, has_pos;

		d1 = sign({ p, triangle[0], triangle[1] });
		d2 = sign({ p, triangle[1], triangle[2] });
		d3 = sign({ p, triangle[2], triangle[0] });

		has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		return !(has_neg && has_pos);
	}

	glm::vec2 relative_pos(const glm::ivec2& pos_windowspace_a, const glm::ivec2& pos_windowspace_b, const glm::ivec2& size)
	{
		return static_cast<glm::vec2>((pos_windowspace_a - pos_windowspace_b) - size / 2) / static_cast<glm::vec2>(size / 2);
	}

	template<size_t dim = 2>
	glm::vec3 to_barycentric(const glm::vec<dim, float>& p, const std::array<glm::vec<dim, float>, 3>& triangle)
	{
		glm::vec<dim, float> v0 = triangle[1] - triangle[0], v1 = triangle[2] - triangle[0], v2 = p - triangle[0];
		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;

		glm::vec3 barycentric;
		barycentric.z = (d00 * d21 - d01 * d20) / denom;
		barycentric.y = (d11 * d20 - d01 * d21) / denom;
		barycentric.x = 1.0f - barycentric.z - barycentric.y;
		return barycentric;
	}

	glm::vec3 clamp_barycentric(const glm::vec3& barycentric_pos, const std::array<glm::vec2, 3>& triangle)
	{
		glm::vec2 p = barycentric_pos.x * triangle[0] + barycentric_pos.y * triangle[1] + barycentric_pos.z * triangle[2];
		if (barycentric_pos.x < 0)
		{
			float t = glm::dot(p - triangle[1], triangle[2] - triangle[1]) / glm::dot(triangle[2] - triangle[1], triangle[2] - triangle[1]);
			t = glm::clamp(t, 0.0f, 1.0f);
			return glm::vec3(0.0f, 1.0f - t, t);
		}
		else if (barycentric_pos.y < 0)
		{
			float t = glm::dot(p - triangle[2], triangle[0] - triangle[2]) / glm::dot(triangle[0] - triangle[2], triangle[0] - triangle[2]);
			t = glm::clamp(t, 0.0f, 1.0f);
			return glm::vec3(t, 0.0f, 1.0f - t);
		}
		else if (barycentric_pos.z < 0)
		{
			float t = glm::dot(p - triangle[0], triangle[1] - triangle[0]) / glm::dot(triangle[1] - triangle[0], triangle[1] - triangle[0]);
			t = glm::clamp(t, 0.0f, 1.0f);
			return glm::vec3(1.0f - t, t, 0.0f);
		}
		else
		{
			return barycentric_pos;
		}
	}

	class ColorPickerRenderer
	{
	private:
		static ColorPickerRenderer* singleton;
		ColorPickerRenderer(const ColorPickerRenderer& copy) = delete;
		ColorPickerRenderer()
			: c_shader({ "asset:color_picker_shader", {
					{
						oe::shader_stages::vertex_shader,
						{
							oe::asset::AssetLoader::resource_string("shader/default_shader/shader.vert.glsl"),
							{}
						}
					},
					{
						oe::shader_stages::fragment_shader,
						{
							oe::asset::AssetLoader::resource_string("shader/gui/color_picker.frag.glsl"),
							{}
						}
					}
				}})
			, c_renderer_circle(oe::RendererInfo{ 1 })
			, c_renderer_triangle(oe::RendererInfo{ 1 })
			, c_pack()
		{
			auto checker_img = oe::asset::TextureSet::generate_checkerboard();
			TextureInfo ti;
			ti.data = checker_img.data;
			ti.data_format = checker_img.format;
			ti.size_offset = { { checker_img.width, 0 }, { checker_img.height, 0 } };
			ti.wrap = oe::texture_wrap::repeat;

			c_checkerboard = {
				{ ti }, { 0.0f, 0.0f }, { 1.0f, 1.0f }
			};

			auto circle_img = oe::asset::TextureSet::generate_circle(32);
			c_circle_sprite = c_pack.create(circle_img);
			c_pack.construct();

			c_renderer_circle->begin();
			c_renderer_circle->submitVertex(std::array<oe::graphics::VertexData, 4> {{
				{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }, oe::colors::white },
				{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }, oe::colors::white },
				{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, oe::colors::white },
				{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }, oe::colors::white },
			}}, 0);
			c_renderer_circle->end();
		}

	public:
		static ColorPickerRenderer& getSingleton() { if (!singleton) singleton = new ColorPickerRenderer(); return *singleton; }

		oe::graphics::Shader c_shader;
		oe::asset::DefaultShader c_defaultshader;
		oe::graphics::PrimitiveRenderer c_renderer_circle;
		oe::graphics::TrianglePrimitiveRenderer c_renderer_triangle;
		oe::graphics::Sprite const* c_circle_sprite;
		oe::graphics::SpritePack c_pack;
		oe::graphics::Sprite c_checkerboard;
	};
	ColorPickerRenderer* ColorPickerRenderer::singleton = nullptr;



	ColorPicker::ColorPicker(Widget* parent, GUI& gui_manager, const info_t& color_picker_info, value_t& value_ref)
		: SpritePanel(parent, gui_manager, { color_picker_info.color_input_info.background_color, color_picker_info.color_input_info.sprite, 0.0f, oe::alignments::top_left, color_picker_info.color_input_info.widget_info })
		, m_color_picker_info(color_picker_info)
		, m_value(value_ref)
	{
		m_triangle_vertices[0].data.color = oe::colors::red;
		m_triangle_vertices[1].data.color = oe::colors::white;
		m_triangle_vertices[2].data.color = oe::colors::black;

		m_triangle_vertices[0].data.uv = { 0.0f, 0.0f };
		m_triangle_vertices[1].data.uv = { 0.0f, 0.0f };
		m_triangle_vertices[2].data.uv = { 0.0f, 0.0f };

		auto& renderer = ColorPickerRenderer::getSingleton();

		SpritePanel::info_t lsp_info;
		lsp_info.widget_info.pixel_size = glm::ivec2{ std::min(m_render_size.x, m_render_size.y - 25) };
		lsp_info.widget_info.fract_origon_offset = oe::alignments::top_center;
		lsp_info.widget_info.fract_render_offset = oe::alignments::top_center;
		lsp_info.widget_info.toggled = color_picker_info.color_input_info.widget_info.toggled;
		lsp_info.sprite = &renderer.c_checkerboard;
		lsp_info.color_tint = oe::colors::white;
		m_framebuffer_panel = create(lsp_info);

		if (color_picker_info.preview)
		{
			SpritePanel::info_t sp_info;
			sp_info.widget_info.pixel_size = { 25, 25 };
			sp_info.widget_info.pixel_origon_offset = { 5, 5 };
			sp_info.widget_info.fract_origon_offset = oe::alignments::top_left;
			sp_info.widget_info.fract_render_offset = oe::alignments::top_left;
			sp_info.widget_info.toggled = color_picker_info.color_input_info.widget_info.toggled;
			sp_info.sprite = renderer.c_circle_sprite;
			sp_info.color_tint = oe::colors::white;
			m_preview = create(sp_info);
		}

		if (color_picker_info.alpha)
		{
			BasicSliderInput<float>::info_t s_info;
			s_info.knob_size = { 16, 16 };
			s_info.widget_info.pixel_size = { -10, 15 };
			s_info.widget_info.fract_size = { 1.0f, 0.0f };
			s_info.widget_info.pixel_origon_offset = { 0, -5 };
			s_info.widget_info.fract_origon_offset = oe::alignments::bottom_center;
			s_info.widget_info.fract_render_offset = oe::alignments::bottom_center;
			s_info.widget_info.toggled = color_picker_info.color_input_info.widget_info.toggled;
			s_info.slider_sprite = &renderer.c_checkerboard;
			s_info.knob_sprite = &renderer.c_checkerboard;
			s_info.value_bounds = { 0.0f, 1.0f };
			s_info.linear_color = true;
			s_info.slider_lcolor = oe::colors::white;
			s_info.slider_rcolor = oe::colors::black;
			s_info.knob_sprite = color_picker_info.color_input_info.sprite;
			m_alpha_slider = create(s_info, m_value.a);
		}

		m_value_last = m_value + 1.0f;
		sprite_panel_info.sprite = m_color_picker_info.color_input_info.sprite;

		m_alpha_slider->connect_listener<BasicSliderInputUseEvent<float>, &ColorPicker::on_slider_use>(this);
	}

	ColorPicker::~ColorPicker()
	{
		m_alpha_slider->disconnect_listener<BasicSliderInputUseEvent<float>, &ColorPicker::on_slider_use>(this);
	}

	void ColorPicker::on_slider_use(const BasicSliderInputUseEvent<float>& event)
	{
		m_value.a = event.value;
	}
	
	void ColorPicker::virtual_toggle(bool enabled)
	{
		SpritePanel::virtual_toggle(enabled);
		if(enabled)
		{
			m_wheel_fb = { { m_framebuffer_panel->m_render_size } };
			m_framebuffer_panel->sprite_panel_info.sprite = &m_wheel_fb->getSprite();

			// event listeners
			m_cg_render.connect<GUIRenderEvent, &ColorPicker::on_render, ColorPicker>(m_gui_manager.m_dispatcher, this);
			m_cg_cursor.connect<CursorPosEvent, &ColorPicker::on_cursor, ColorPicker>(m_gui_manager.m_dispatcher, this);
			m_cg_button.connect<MouseButtonEvent, &ColorPicker::on_button, ColorPicker>(m_gui_manager.m_dispatcher, this);
		}
		else
		{
			m_wheel_fb.reset();

			// event listeners
			m_cg_render.disconnect();
			m_cg_cursor.disconnect();
			m_cg_button.disconnect();
		}
	}
	
	void ColorPicker::update_from_value(bool update_dir)
	{
		m_value_last = m_value;

		// set color
		if(update_dir)
		{
			glm::vec3 hsv = oe::utils::rgbToHSV(m_value);
			m_triangle_vertices[0].data.color = { oe::utils::hsvToRGB({ hsv.x, 1.0f, 1.0f }), 1.0f };
			m_direction = -hsv.x * glm::two_pi<float>();

			glm::vec3 left = hsv.z * (hsv.y) * glm::vec3{ 1.0f, 0.0f, 0.0f };
			glm::vec3 right = hsv.z * (1.0f - hsv.y) * glm::vec3{ 0.0f, 1.0f, 0.0f };
			glm::vec3 origin = (1.0f - hsv.z * (hsv.y) - hsv.z * (1.0f - hsv.y)) * glm::vec3{ 0.0f, 0.0f, 1.0f };

			origin = origin + left + right;
			m_barycentric_pos_triangle = origin;
		}
		
		// triangle vertices
		m_triangle_vertices[0].data.position = { std::cos(m_direction - 0 * m_equilateral_triangle_angles) * m_triangle_width, std::sin(m_direction - 0 * m_equilateral_triangle_angles) * m_triangle_width, 0.0f };
		m_triangle_vertices[1].data.position = { std::cos(m_direction - 1 * m_equilateral_triangle_angles) * m_triangle_width, std::sin(m_direction - 1 * m_equilateral_triangle_angles) * m_triangle_width, 0.0f };
		m_triangle_vertices[2].data.position = { std::cos(m_direction - 2 * m_equilateral_triangle_angles) * m_triangle_width, std::sin(m_direction - 2 * m_equilateral_triangle_angles) * m_triangle_width, 0.0f };

		// preview color
		if (m_color_picker_info.preview)
			m_preview->sprite_panel_info.color_tint = m_value;

		// circle in wheel
		glm::vec2 selector_wheel_f = { std::cos(m_direction), std::sin(m_direction) };
		selector_wheel_f *= -m_framebuffer_panel->m_render_size / 2;
		selector_wheel_f.y *= -1.0f;
		m_selector_wheel = glm::vec2(selector_wheel_f) * (1.0f - m_wheel_width);

		// circle in triangle
		glm::vec2 selector_triangle_f =
			+ m_barycentric_pos_triangle.x * m_triangle_vertices[0].data.position
			+ m_barycentric_pos_triangle.y * m_triangle_vertices[1].data.position
			+ m_barycentric_pos_triangle.z * m_triangle_vertices[2].data.position;
		selector_triangle_f *= -m_framebuffer_panel->m_render_size / 2;
		selector_triangle_f.y *= -1.0f;
		m_selector_triangle = selector_triangle_f;
	}
	
	void ColorPicker::update_to_value()
	{
		update_from_value(false);

		// set color according to triangle and wheel
		float alpha = m_alpha_slider ? m_alpha_slider->m_value : m_value.a;
		m_value =
			+ m_barycentric_pos_triangle.x * m_triangle_vertices[0].data.color
			+ m_barycentric_pos_triangle.y * m_triangle_vertices[1].data.color
			+ m_barycentric_pos_triangle.z * m_triangle_vertices[2].data.color;
		m_value = glm::max(m_value, 0.0f);
		m_value.a = alpha;
		m_value_last = m_value;

		// preview color
		if (m_color_picker_info.preview)
			m_preview->sprite_panel_info.color_tint = m_value;

		m_event_use_latest.value = m_value;
		m_dispatcher.trigger(m_event_use_latest);
	}

	void ColorPicker::on_render(const GUIRenderEvent& /* event */)
	{
		if(!m_cg_render)
			return;

		if(m_value_last != m_value)
			update_from_value(true);

		m_wheel_fb->bind();
		m_wheel_fb->clear(oe::colors::transparent);

		const float y = m_render_size.y == 0 ? 1.0f : static_cast<float>(m_framebuffer_panel->m_render_size.y);
		const float aspect = static_cast<float>(m_framebuffer_panel->m_render_size.x) / y;
		glm::mat4 pr_matrix = glm::ortho(-aspect, aspect, 1.0f, -1.0f);

		auto& renderer = ColorPickerRenderer::getSingleton();
		renderer.c_shader->bind();
		renderer.c_shader->setUniform("u_viewport", m_framebuffer_panel->m_render_size);
		renderer.c_shader->setUniform("u_offset", -m_framebuffer_panel->m_render_size / 2);
		renderer.c_shader->setUniform("u_wheel_width", m_wheel_width);
		renderer.c_shader->setUniform("mvp_matrix", pr_matrix);
		renderer.c_shader->setUniform("u_hsv", true);
		renderer.c_renderer_circle->render();

		renderer.c_defaultshader.bind();
		renderer.c_defaultshader.setProjectionMatrix(pr_matrix);
		renderer.c_defaultshader.setColor(oe::colors::white);
		renderer.c_defaultshader.setTexture(false);
		renderer.c_renderer_triangle->begin();
		renderer.c_renderer_triangle->submitVertex(m_triangle_vertices, 0);
		renderer.c_renderer_triangle->end();
		renderer.c_renderer_triangle->render(0, 1);

		renderer.c_shader->bind();
		renderer.c_shader->setUniform("u_viewport", m_framebuffer_panel->m_render_size / 16);
		renderer.c_shader->setUniform("u_offset", -m_framebuffer_panel->m_render_size / 2 + m_selector_triangle);
		renderer.c_shader->setUniform("u_wheel_width", m_wheel_width);
		renderer.c_shader->setUniform("mvp_matrix", pr_matrix);
		renderer.c_shader->setUniform("u_hsv", false);
		renderer.c_renderer_circle->render();

		renderer.c_shader->bind();
		renderer.c_shader->setUniform("u_viewport", m_framebuffer_panel->m_render_size / 16);
		renderer.c_shader->setUniform("u_offset", -m_framebuffer_panel->m_render_size / 2 + m_selector_wheel);
		renderer.c_shader->setUniform("u_wheel_width", m_wheel_width);
		renderer.c_shader->setUniform("mvp_matrix", pr_matrix);
		renderer.c_shader->setUniform("u_hsv", false);
		renderer.c_renderer_circle->render();

		renderer.c_checkerboard.size = m_alpha_slider->m_render_size / 8;

		m_gui_manager.getWindow()->bind();
		m_framebuffer_panel->sprite_panel_info.sprite = &m_wheel_fb->getSprite();
	}

	void ColorPicker::on_cursor(const CursorPosEvent& event)
	{
		if(!m_cg_cursor)
			return;
		// relative_pos
		const glm::vec2 r_pos = relative_pos(event.cursor_windowspace, m_framebuffer_panel->m_render_position, m_framebuffer_panel->m_render_size);

		if (m_dragging_wheel)
		{
			/*if (!in_circle(r_pos, m_wheel_width))
				return;*/

			m_direction = atan2f(r_pos.y, r_pos.x);
			const float hue = -m_direction / glm::two_pi<float>();
			m_triangle_vertices[0].data.color = { oe::utils::hsvToRGB({ hue, 1.0f, 1.0f }), 1.0f };

			update_to_value();
		}
		if (m_dragging_triangle)
		{
			/*if (!in_triangle(r_pos, { m_triangle_vertices[0].position, m_triangle_vertices[1].position, m_triangle_vertices[2].position }))
				return;*/

			m_barycentric_pos_triangle = to_barycentric<2>(r_pos, { m_triangle_vertices[0].data.position, m_triangle_vertices[1].data.position, m_triangle_vertices[2].data.position });
			m_barycentric_pos_triangle = clamp_barycentric(m_barycentric_pos_triangle, { m_triangle_vertices[0].data.position, m_triangle_vertices[1].data.position, m_triangle_vertices[2].data.position });
			
			update_to_value();
		}

		if (in_circle(r_pos, m_wheel_width) || in_triangle(r_pos, { m_triangle_vertices[0].data.position, m_triangle_vertices[1].data.position, m_triangle_vertices[2].data.position }))
			m_dispatcher.trigger(m_event_hover_latest);
	}

	void ColorPicker::on_button(const MouseButtonEvent& event)
	{
		if(!m_cg_button)
			return;

		m_event_use_latest.action = event.action;
		m_event_use_latest.button = event.button;
		m_event_use_latest.modifier = event.mods;

		glm::vec2 r_pos = relative_pos(event.cursor_pos.cursor_windowspace, m_framebuffer_panel->m_render_position, m_framebuffer_panel->m_render_size);
		if (event.button == oe::mouse_buttons::button_left)
		{
			m_dragging_wheel = event.action != oe::actions::release && in_circle(r_pos, m_wheel_width);
			m_dragging_triangle = event.action != oe::actions::release && in_triangle(r_pos, { m_triangle_vertices[0].data.position, m_triangle_vertices[1].data.position, m_triangle_vertices[2].data.position });
		}

		if (m_dragging_wheel || m_dragging_triangle)
			on_cursor(event.cursor_pos);
	}
}