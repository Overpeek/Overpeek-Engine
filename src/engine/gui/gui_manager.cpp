#include "gui_manager.hpp"

#include "engine/graphics/interface/window.hpp"
#include "engine/graphics/interface/renderer.hpp"
#include "engine/graphics/font.hpp"

#include "engine/engine.hpp"



namespace oe::gui {

	constexpr int border = 5;

	
	GUI::GUI(oe::graphics::Window* window) 
		: m_window(window)
		, m_offset(0.0f, 0.0f)
	{
		// renderer
		oe::RendererInfo renderer_info = {};
		renderer_info.arrayRenderType = oe::types::dynamicrender;
		renderer_info.indexRenderType = oe::types::staticrender;
		renderer_info.max_quad_count = 10000;
		renderer_info.staticVBOBuffer_data = nullptr;
		m_renderer = m_window->createRenderer(renderer_info);

		// shader
		m_shader = m_window->createShader(ShaderInfo());

		FormInfo form_info = {};
		form_info.size = m_window->getSize() - glm::vec2(2 * border);
		form_info.offset_position = { border, border };
		m_main_frame = new oe::gui::Form(form_info);
		resize();
	}

	GUI::~GUI() {
		m_window->destroyRenderer(m_renderer);
		m_window->destroyShader(m_shader);

		delete m_main_frame;
	}

	void GUI::offset(const glm::vec2& offset) {
		m_offset = offset;

		glm::mat4 ml_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(offset, 0.0f));
		m_shader->bind();
		m_shader->setUniformMat4("ml_matrix", ml_matrix);
	}

	void GUI::render() {
		static int cooldown = 0;
		if ((++cooldown) % 60 == 0) {
			cooldown = 0;
			resize();
		}

		m_renderer->begin();
		m_renderer->clear();

		if (m_main_frame) m_main_frame->__render(*m_renderer);
		
		m_shader->bind();
		m_renderer->end();
		m_renderer->render();
	}

	void GUI::resize() {
		resize(m_window->getSize());
	}

	void GUI::resize(const glm::vec2& window_size) {
		m_main_frame->size = window_size - glm::vec2(2 * border);
		m_main_frame->offset_position = { border, border };
		m_main_frame->__resize();

		/*
		    0                  0
		  0 +------------------+ w
		    |                  |
		    |                  |
		    |                  |
		  0 +------------------+ w
		    h                  h
		*/

		glm::mat4 pr_matrix = glm::ortho(0.0f, (float)window_size.x, (float)window_size.y, 0.0f);
		m_shader->bind();
		m_shader->setUniform1i("usetex", true);
		m_shader->setUniformMat4("pr_matrix", pr_matrix);
	}

	void GUI::addSubWidget(Widget* widget) {
		m_main_frame->addSubWidget(widget);
	}

	void GUI::cursor(oe::mouse_buttons button, oe::actions action, const glm::vec2& cursor_window) {
		glm::vec2 cursor_window_final = cursor_window - m_offset;

		m_main_frame->__cursor(button, action, cursor_window_final);
	}

	void GUI::text(uint32_t codepoint, oe::modifiers mods) {
		m_main_frame->__text(codepoint, mods);
	}

	void GUI::key(oe::keys key, oe::actions action, oe::modifiers mods) {
		m_main_frame->__key(key, action, mods);
	}

}

