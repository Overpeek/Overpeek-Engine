#include <engine/engine.hpp>

#include <string>

#include "texture.png.hpp"



const oe::graphics::Sprite* sprite;
const oe::graphics::Sprite* sprite_white;
oe::graphics::Instance* instance;
oe::graphics::IWindow* window;
oe::graphics::SpritePack* pack;
oe::graphics::Renderer* renderer;
oe::asset::DefaultShader* shader;

double t = 0;
void render(float update_fraction) {
	t += oe::utils::GameLoop::getSingleton().getFrameUpdateScale();

	// clear framebuffer
	window->clear();

	// begin submitting
	renderer->begin();
	renderer->clear();

	// submitting
	renderer->submit(glm::vec2(0.0f, 0.0f),  glm::vec2(1.0f, 1.0f), oe::colors::rainbow(t), oe::alignments::center_center);

	// stop submitting and render
	// pack->bind();
	shader->bind();
	renderer->end();
	renderer->render();

	// swap buffers and poll events
	window->update();

	// check if needs to close
	if (window->shouldClose()) oe::utils::GameLoop::getSingleton().stop();
}

void update() {
	spdlog::info("FPS: " + std::to_string(oe::utils::GameLoop::getSingleton().getFPS()));
}

void resize(const glm::vec2& window_size) {
	float aspect = window->aspect();
	glm::mat4 pr = glm::ortho(-aspect, aspect, 1.0f, -1.0f);
	shader->setProjectionMatrix(pr);
	shader->useTexture(true);
}

void keyboard(oe::keys key, oe::actions action, oe::modifiers mods) {
	if (action == oe::actions::press) {
		if (key == oe::keys::key_escape) {
			oe::utils::GameLoop::getSingleton().stop();
		}
		else if (key == oe::keys::key_enter) {
			window->setFullscreen(!window->getFullscreen());
		}
	}
}

int main(int argc, char** argv) {
	// engine
	oe::EngineInfo engine_info = {};
	engine_info.api = oe::graphics_api::Vulkan;
	oe::Engine::getSingleton().init(engine_info);

	// instance
	oe::InstanceInfo instance_info = {};
	instance_info.debug_messages = true;
	instance_info.favored_gpu_vulkan = oe::gpu::dedicated;
	instance = oe::Engine::getSingleton().createInstance(instance_info);

	// window
	oe::WindowInfo window_info;
	window_info.title = "Test 5 - Vulkan";
	window_info.multisamples = 4;
	window_info.resize_callback = resize;
	window_info.key_callback = keyboard;
	window = instance->createWindow(window_info);

	// instance settings
	instance->culling(oe::culling_modes::back);
	instance->swapInterval(1);
	instance->blending();

	// renderer
	oe::RendererInfo renderer_info = {};
	renderer_info.arrayRenderType = oe::types::dynamicrender;
	renderer_info.indexRenderType = oe::types::staticrender;
	renderer_info.max_quad_count = 6;
	renderer_info.staticVBOBuffer_data = nullptr;
	renderer = window->createRenderer(renderer_info);

	// shader
	shader = new oe::asset::DefaultShader(window);

	// sprites
	auto img = oe::utils::image_data(texture_png, oe::formats::rgba, 5, 5);
	pack = new oe::graphics::SpritePack(window);
	sprite = pack->addSprite(img);
	sprite_white = pack->empty_sprite();
	pack->construct();

	// start
	resize(window->getSize());
	oe::utils::GameLoop::getSingleton().start(render, update, 1);

	// closing
	delete pack;
	delete shader;
	window->destroyRenderer(renderer);
	instance->destroyWindow(window);
	oe::Engine::getSingleton().destroyInstance(instance);

	return 0;
}