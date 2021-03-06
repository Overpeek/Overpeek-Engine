#pragma once

// Graphics
#include "graphics/interface/command_buffer.hpp"
#include "graphics/interface/framebuffer.hpp"
#include "graphics/interface/instance.hpp"
#include "graphics/interface/poly_renderer.hpp"
#include "graphics/interface/shader.hpp"
#include "graphics/interface/texture.hpp"
#include "graphics/interface/window.hpp"
#include "graphics/font.hpp"
#include "graphics/renderer.hpp"
#include "graphics/textLabel.hpp"

// Audio
#include "audio/audio.hpp"

// ecs
#include "ecs/components/all.hpp"
#include "ecs/world.hpp"
#include "ecs/entity.hpp"

// Logic
#include "utility/color_string.hpp"
#include "utility/random.hpp"
#include "utility/gameloop.hpp"
#include "utility/fileio.hpp"
#include "utility/font_file.hpp"
#include "utility/formatted_error.hpp"
#include "utility/extra.hpp"
#include "utility/connect_guard.hpp"
#include "utility/connect_guard_additions.hpp"
#include "utility/ts_queue.hpp"

// Gui
#include "gui/gui_manager.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/text_panel.hpp"
#include "gui/widgets/text_input.hpp"
#include "gui/widgets/number_input.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/button_decorated.hpp"
#include "gui/widgets/slider_input.hpp"
#include "gui/widgets/vec_slider.hpp"
#include "gui/widgets/sprite_panel.hpp"
#include "gui/widgets/checkbox.hpp"
#include "gui/widgets/color_input.hpp"
#include "gui/widgets/color_picker.hpp"
#include "gui/widgets/list.hpp"
#include "gui/widgets/graph.hpp"

// Other
#include "engine/enum.hpp"
#include "engine/engine.hpp"
#include "internal_libs.hpp"
#include "graphics/sprite.hpp"
#include "graphics/spritePacker.hpp"
#include "networking/client.hpp"
#include "networking/server.hpp"

// Asset
#include "asset/default_shader/default_shader.hpp"
#include "asset/font_shader/font_shader.hpp"
#include "asset/texture_set/texture_set.hpp"
#include "asset/fonts.hpp"

// POSSIBLE DEFINES:
// - OE_DEBUG_API_CALLS
//   - print opengl / vulkan calls with their arguments
//   - compiled
// - OE_USING_NAMESPACES
//   - shorter namespaces
//   - headers (obviously)
// - BUILD_VULKAN
//   - enable experimental vulkan
//   - compiled
// - OE_BUILD_MODE_SHADERC
//   - enable shaderc glsl optimizer
//   - compiled
// - OE_TERMINATE_IS_THROW
//   - fatal errors throw instead of assert
//   - compiled

#if defined(OE_USING_NAMESPACES)
namespace oe
{
	using namespace audio;
	using namespace ecs;
	using namespace graphics;
	using namespace gui;
	using namespace networking;
	using namespace utils;
}
#endif