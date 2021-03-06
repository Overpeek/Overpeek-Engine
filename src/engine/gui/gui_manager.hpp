#pragma once

#include "engine/interfacegen.hpp"
#include "engine/utility/connect_guard.hpp"
#include "engine/utility/fileio.hpp"
#include "engine/graphics/font.hpp"
#include "widgets/widget.hpp"

#include <entt/entt.hpp>



/* -- forward declarations -- */
namespace oe::asset { class DefaultShader; }
namespace oe::graphics { class Font; class Renderer; }
namespace oe::gui { class Widget; class Form; class GUI; }
/* -- forward declarations -- */



namespace oe::gui
{
	constexpr int border = 5;

	struct GUIRenderEvent {};
	struct GUIPreRenderEvent {};
	
	class GUI
	{
	public:
		entt::dispatcher m_dispatcher;

	private:
		// graphics
		oe::graphics::Renderer* m_renderer = nullptr;
		oe::asset::DefaultShader* m_shader = nullptr;
		constexpr static oe::RasterizerInfo m_rasterizer = { oe::modes::enable, oe::depth_functions::less_than_or_equal, oe::culling_modes::back, oe::polygon_mode::fill, 1.0f, 1.0f };
		// 
		std::shared_ptr<Widget> m_main_frame{};
		oe::graphics::Window m_window{};
		oe::ResizeEvent latest_resize_event{};
		// fonts
		std::unordered_map<oe::utils::FontFile, oe::graphics::Font> m_fontmap{}; // (fontfile - font pair) pair
		const oe::utils::FontFile m_default_font_file{};

		// viewport stuff
		glm::vec2 m_old_render_size{};
		glm::vec2 m_offset{};
		glm::vec2 m_size_mult{};
		glm::mat4 m_render_ml_matrix{};
		glm::mat4 m_cursor_ml_matrix{};

		void updateModelMatrix();

	public:
		GUI(const oe::graphics::Window& window, const oe::utils::FontFile& font_file = {}, int32_t renderer_primitive_count = 100000);
		~GUI();

		void short_resize();

		// create subwidget
		template<typename T, typename ... Args>
		std::shared_ptr<typename T::widget_t> create(const T& info, Args& ... args)
		{
			return m_main_frame->create(info, args...);
		}

		// remove subwidget
		template<typename T>
		void remove(const std::shared_ptr<T>& widget)
		{
			m_main_frame->remove<T>(widget);
		}

		// remove all subwidgets
		void clear();
		
		// bind SpritePacker that you used to create Font and all Sprites for StaticTextureViews
		void render();
		void render_empty();

		// move the whole gui system
		void offset(const glm::vec2& offset);

		// used for application debugging
		void zoom(const glm::vec2& mult);

		inline oe::graphics::Renderer* getRenderer() const { return m_renderer; }
		inline const oe::graphics::Window& getWindow() const { return m_window; }
		inline const oe::asset::DefaultShader* getShader() const { return m_shader; }

		oe::graphics::Font& getFont(const oe::utils::FontFile& font = {});

	private:
		// events
		void on_resize(const ResizeEvent& event);
		void on_codepoint(const CodepointEvent& event);
		void on_key(const KeyboardEvent& event);
		void on_cursor_pos(const CursorPosEvent& event);
		void on_button(const MouseButtonEvent& event);
		void on_scroll(const ScrollEvent& event);
		oe::utils::connect_guard m_cg_resize;
		oe::utils::connect_guard m_cg_codepoint;
		oe::utils::connect_guard m_cg_key;
		oe::utils::connect_guard m_cg_cursor_pos;
		oe::utils::connect_guard m_cg_button;
		oe::utils::connect_guard m_cg_scroll;
	};

}