#pragma once

#include "button.h"
#include "text_panel.h"
#include "sprite_panel.h"



namespace oe::gui {

	struct DecoratedButtonInfo {
		button_callback callback           = nullptr;
		glm::ivec2 size                    = { 50, 50 };
		glm::vec2 offset_position          = { 0, 0 };
		std::string text                   = "";
		glm::vec2 align_parent             = oe::alignments::center_center;
		glm::vec2 align_render             = oe::alignments::center_center;
		glm::vec4 color                    = oe::colors::dark_grey;
		const oe::graphics::Sprite* sprite = nullptr; // must be set
	};

	class DecoratedButton : public Widget {
	public:
		DecoratedButtonInfo button_info;

		oe::gui::SpritePanel* button_background;
		oe::gui::TextPanel* button_text;
		oe::gui::Button* button;

	public:
		DecoratedButton(const DecoratedButtonInfo& button_info);
		~DecoratedButton();

		// Inherited via Widget
		virtual void render(oe::graphics::Renderer& renderer) override;
	};

}