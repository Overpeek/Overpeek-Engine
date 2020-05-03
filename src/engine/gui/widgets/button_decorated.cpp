#include "button_decorated.hpp"

#if _DEBUG && 0
#define _DEFAULT_COLOR glm::vec4(0.0f, 0.0f, 0.0f, 0.2f)
#else
#define _DEFAULT_COLOR glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)
#endif // _DEBUG


namespace oe::gui {

	DecoratedButton::DecoratedButton(GUI* gui_manager, const DecoratedButtonInfo& _button_info)
		: Widget(gui_manager, _button_info.size, _button_info.align_parent, _button_info.align_render, _button_info.offset_position)
		, button_info(_button_info)
	{
		ButtonInfo b_info = {};
		b_info.size = _button_info.size;
		b_info.align_parent = oe::alignments::center_center;
		b_info.align_render = oe::alignments::center_center;
		b_info.callback = _button_info.callback;
		button = new Button(gui_manager, b_info);
		addSubWidget(button);

		SpritePanelInfo sp_info = {};
		sp_info.size = _button_info.size;
		sp_info.align_parent = oe::alignments::center_center;
		sp_info.align_render = oe::alignments::center_center;
		sp_info.sprite = _button_info.sprite;
		sp_info.color = _button_info.color;
		button_background = new oe::gui::SpritePanel(gui_manager, sp_info);
		button->addSubWidget(button_background);

		TextPanelInfo tp_info = {};
		tp_info.font_size = _button_info.size.y * 0.8f;
		tp_info.text = _button_info.text;
		tp_info.align_parent = oe::alignments::center_center;
		tp_info.align_render = oe::alignments::center_center;
		button_text = new oe::gui::TextPanel(gui_manager, tp_info);
		button_background->addSubWidget(button_text);
	}

	DecoratedButton::~DecoratedButton() {
		// delete button_background;
		// delete button_text;
		// delete button;
	}

}