#pragma once

#include "database.h"

#define M_COLOR_HEALTH_BAR glm::vec4(1.0, 0.2, 0.2, 1.0)
#define M_COLOR_STAMINA_BAR glm::vec4(0.2, 1.0, 0.2, 1.0)
#define M_COLOR_BLACK glm::vec4(0.0, 0.0, 0.0, 1.0)

#define GUI_FRAME_LOGGER_SIZE 200
#define GUI_FRAME_LOGGER_BAR_WIDTH 0.005

#define MAX_TEXT_LINES	10

namespace oe { class Window; class Shader; class Renderer; }
class Gui {

private:
	int m_selectedButton;
	bool m_selected_logger;

	long int m_currentFrame;
	float m_frame_logger[GUI_FRAME_LOGGER_SIZE];
	float m_slowest_frame;

	long int m_currentUpdate;
	float m_update_logger[GUI_FRAME_LOGGER_SIZE];
	float m_slowest_update;

	int m_chat_opened_timer;
	int m_current_input_history;
	bool m_chat_opened;
	bool m_chat_just_opened;
	std::vector<std::string> m_last_inputs;
	std::vector<std::string> m_text_lines;
	std::string m_current_line;
	std::string m_current_line_reserved;

	void userInput();
	void addTextToLatest(std::string text);

	void selectInputHistory();

public:
	Gui(float maxHealth, float maxStamina, float healthGainRate, float staminaGainRate);

	void renderBlur(oe::Renderer *renderer_blurred);
	void renderNoBlur(oe::Renderer *renderer);
	void update();
	void keyPress(int key, int action);
	void addChatLine(std::string text);

	void typing(unsigned int codepoint, int mods);

	bool chatOpened() { return m_chat_opened; }
};