#pragma once

#include <string>
#include <map>

#include "engine/graphics/texture.h"
#include "engine/internal_libs.h"

#define CHAR_COUNT 128



namespace oe::graphics {

	class Font {
	public:
		struct Glyph {
			unsigned char character;
			glm::vec2 size;
			glm::vec2 top_left;
			glm::vec2 advance;
		};

		void* m_ft;
		void* m_face;
		unsigned char* m_data;

	public:
		Texture m_glyph_texture;
		Glyph* m_glyphs[256];
		int m_resolution;

		float bb_max_height;
		float bb_min_height;
		float bb_height;
	
	public:
		Font(int resolution = 64, bool manual_finish = false, std::string font_path = "calibri.ttf");
		~Font();

		// Must be executed on main thread (OpenGL thread)
		void finish();

		inline void bindTexture() const { m_glyph_texture.bind(); }
	};

}
