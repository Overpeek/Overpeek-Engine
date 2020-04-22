#pragma once

#include "widget.hpp"
#include "sprite_panel.hpp"



namespace oe::gui {

	typedef std::function<void(glm::vec4)> color_picker_callback;

	struct ColorPickerInfo {
		color_picker_callback callback             = nullptr;
		float min_value                            = -1.0f;
		float max_value                            = 1.0f;
		glm::vec4 initial_color                    = oe::colors::white;
		glm::vec4 background_color                 = oe::colors::dark_grey;
		glm::ivec2 size                            = { 200, 100 };
		const oe::graphics::Sprite* sprite         = nullptr; // must be set
		glm::vec2 offset_position                  = { 0, 0 };
		glm::vec2 align_parent                     = oe::alignments::center_center;
		glm::vec2 align_render                     = oe::alignments::center_center;
	};

	class ColorPicker : public Widget {
	private:
		SpritePanel* preview_panel = nullptr;
	
	public:
		ColorPickerInfo color_picker_info;

	public:
		ColorPicker(const ColorPickerInfo& color_picker_info);

		inline const glm::vec4& get() const { return color_picker_info.initial_color; }
		inline void set(const glm::vec4& color) { color_picker_info.initial_color = color; update(); }
		inline void update() { preview_panel->sprite_panel_info.color = color_picker_info.initial_color; if(color_picker_info.callback) color_picker_info.callback(color_picker_info.initial_color); }
	};

}