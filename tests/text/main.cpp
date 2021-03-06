#include <engine/include.hpp>

// NOTE: this test does not follow the best practises
// You should already know by those globals
// Check out the hello-world test for better guide



oe::graphics::Window window;
oe::graphics::Renderer* renderer;
oe::asset::DefaultShader* shader;
oe::graphics::Font* font;

oe::graphics::TextLabel* bkd_label;
oe::asset::FontShader* dyn_label_shader;
oe::graphics::Renderer* dyn_label_renderer;
std::unique_ptr<oe::graphics::Quad> quad;

oe::utils::connect_guard cg_render;
oe::utils::connect_guard cg_update_2;
oe::utils::connect_guard cg_resize;
oe::utils::connect_guard cg_scroll;
oe::utils::connect_guard cg_keyboard;

float zoom = 1.0f;
glm::vec2 offset = { -14.0f, -10.0f };



void render(oe::RenderEvent)
{
	shader->bind();
	renderer->render();

	dyn_label_shader->bind();
	dyn_label_renderer->render();
}

void update_1(oe::UpdateEvent<1>) {
	auto& gameloop = window->getGameloop();
	spdlog::debug("- frametime: {:3.3f} ms ({} fps)\n- updatetime: {:3.3f} ms ({} ups)",
		std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(gameloop.getFrametime()).count(),
		gameloop.getAverageFPS(),
		std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(gameloop.getUpdatetime<30>()).count(),
		gameloop.getAverageUPS<30>());
	
	oe::graphics::Sprite sprite;
	sprite.m_owner = font->getSpritePack()->getTexture();
	quad->setSprite(sprite);
}

void resize(const oe::ResizeEvent& event) {
	const float s_zoom = zoom == 0.0f ? 1.0f : zoom;
	const glm::mat4 pr_matrix = glm::ortho(-event.aspect, event.aspect, 1.0f, -1.0f);
	const glm::mat4 ml_matrix = glm::translate(
									glm::scale(
										glm::translate(
											glm::mat4(1.0f),
											glm::vec3{ -1.0f, -1.0f, 0.0f }),
										glm::vec3{ s_zoom, s_zoom, 1.0f }),
									glm::vec3{ (1.0f / s_zoom + offset.x * 0.1f), (1.0f / s_zoom + offset.y * 0.1f), (0.0f) });
	shader->setProjectionMatrix(pr_matrix);
	shader->setModelMatrix(ml_matrix);
	shader->setTexture(true);
	dyn_label_shader->setProjectionMatrix(pr_matrix);
	dyn_label_shader->setModelMatrix(ml_matrix);
	dyn_label_shader->setTexture(true);
	dyn_label_shader->setSDF(true);
	dyn_label_shader->setEdge(0.1f / s_zoom);
	dyn_label_shader->setOutlineEdge(0.1f / s_zoom);
}

void resize_default()
{
	oe::ResizeEvent r_e;
	r_e.framebuffer_size = window->getSize();
	r_e.aspect = window->getAspect();
	resize(r_e);
}

void on_scroll(const oe::ScrollEvent& e)
{
	zoom += zoom * e.scroll_delta.y * 0.1f;

	resize_default();
}

void on_key(const oe::KeyboardEvent& e)
{
	if(e.action == oe::actions::release)
		return;

	const float s_i_zoom = 1.0f / (zoom == 0.0f ? 1.0f : zoom);
	if(e.key == oe::keys::key_a || e.key == oe::keys::key_left)
		offset.x += s_i_zoom;
	if(e.key == oe::keys::key_d || e.key == oe::keys::key_right)
		offset.x -= s_i_zoom;
	if(e.key == oe::keys::key_s || e.key == oe::keys::key_down)
		offset.y -= s_i_zoom;
	if(e.key == oe::keys::key_w || e.key == oe::keys::key_up)
		offset.y += s_i_zoom;

	resize_default();
}

int main(int argc, char** argv) {
	auto& engine = oe::Engine::getSingleton();
	// engine
	oe::EngineInfo engine_info = {};
	engine_info.api = oe::graphics_api::OpenGL;
	engine.init(engine_info);

	// window
	oe::WindowInfo window_info;
	window_info.title = "Colored Text";
	window_info.multisamples = 4;
	window = oe::graphics::Window(window_info);

	// connect events;
	cg_render.connect<oe::RenderEvent, render>(window);
	cg_update_2.connect<oe::UpdateEvent<1>, update_1>(window);
	cg_resize.connect<oe::ResizeEvent, resize>(window);
	cg_scroll.connect<oe::ScrollEvent, on_scroll>(window);
	cg_keyboard.connect<oe::KeyboardEvent, on_key>(window);

	// renderers
	renderer = new oe::graphics::Renderer(1000);
	dyn_label_renderer = new oe::graphics::Renderer(1000);

	// shaders
	shader = new oe::asset::DefaultShader();
	dyn_label_shader = new oe::asset::FontShader();

	// sprites
	font = new oe::graphics::Font(64, true, oe::asset::Fonts::roboto_regular());
	
	// submitting
	// text options and cache
	const oe::utils::color_string<char32_t> text_input =
	{{
		{ U"|\u2116", { 0.06f, 0.13f, 1.0f, 1.0f } }, { U"The quick brown fox!", oe::colors::white },
		{ U"\u263A\n|", { 1.0f, 0.13f, 0.13f, 1.0f } }, { U"x\u00B2\u221A2AVAVA\U0001242B", oe::colors::orange },
	}};
	oe::graphics::text_render_cache cache{};
	oe::TextOptions options{};
	options.scale = { 0.1f, 0.1f };
	options.advance_padding = 0.0f;
	options.background_color = oe::colors::translucent_black;
	options.resolution = 48;
	cache.create(text_input, *font, options);

	// the upper text, slower but prettier
	cache.submit(*dyn_label_renderer, true, { 0.1f, 0.1f });

	// the bottom text, baked and faster, but limited resolution
	bkd_label = new oe::graphics::TextLabel();
	bkd_label->generate(cache);
	quad = renderer->create();
	quad->setPosition({ 0.1f, 0.4f });
	quad->setSize(bkd_label->size());
	quad->setColor(oe::colors::white);
	quad->setSprite(bkd_label->sprite());
	renderer->forget(std::move(quad));
	
	oe::graphics::Sprite sprite;
	sprite.m_owner = font->getSpritePack()->getTexture();
	quad = renderer->create();
	quad->setPosition({ 0.1f, 0.7f });
	quad->setSize({ 0.5f, 0.5f });
	quad->setColor(oe::colors::white);
	quad->setSprite(sprite);

	// auto close ctest after 2 seconds
	std::thread ctest_close_thread;
	for (size_t i = 0; i < argc; i++)
		if(std::strcmp(argv[i], "--ctest") == 0)
		{
			ctest_close_thread = std::thread([](){
				// test for 2 seconds
				std::this_thread::sleep_for(std::chrono::seconds(2));

				// then stop the gameloop
				window->getGameloop().stop();
			});
			break;
		}
	
	// start
	window->getGameloop().start(); // print the average frametime 30 times in a second

	if(ctest_close_thread.joinable())
		ctest_close_thread.join();
		
	cg_render.disconnect();
	cg_update_2.disconnect();
	cg_resize.disconnect();
	cg_scroll.disconnect();
	cg_keyboard.disconnect();

	// closing
	quad.reset();
	delete bkd_label;
	delete dyn_label_renderer;
	delete renderer;
	delete font;
	delete dyn_label_shader;
	delete shader;

	window.reset();

	return 0;
}
