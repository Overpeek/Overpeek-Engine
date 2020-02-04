#pragma once

#include "engine/graphics/shader.h"



namespace oe::graphics {

	class BasicPostprocessShader : public Shader {
	private:


		static constexpr char* vertsource = R"shader(
			#version 400 core
			layout(location = 0) in vec3 vertex_pos;

			out vec2 UV;

			void main() {
				gl_Position = vec4(vertex_pos, 1);
				UV = (vertex_pos.xy + vec2(1.0, 1.0)) / 2.0;
			}
		)shader";

		static constexpr char* fragsource = R"shader(
			#version 330 core
		
			out vec4 color;
		
			in vec2 UV;
		
			uniform sampler2D unif_texture;
			uniform int unif_effect;
			uniform float unif_t;
		
			float gauss_kernel[] = float[](0.035822f, 0.05879f, 0.086425f, 0.113806f, 0.13424f, 0.141836f, 0.13424f, 0.113806f, 0.086425f, 0.05879f, 0.035822f);
		
			float edge_kernel[] = float[](
				-1, -1, -1,
				-1,  8, -1,
				-1, -1, -1
			);
		
			float pixelSizeX;
			float pixelSizeY;
		
			float rand(vec2 co) {
				return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
			}
		
			void main()
			{
				pixelSizeX = 1.0 / textureSize(unif_texture, 0).x;
				pixelSizeY = 1.0 / textureSize(unif_texture, 0).y;
				if (unif_effect == 1) {
					color = vec4(0.0);
			
					//Horitzontal gaussian blur
					for (int i = -5; i < 6; i++) {
						float ioff = pixelSizeX * i;
						color += texture(unif_texture, vec2(UV.x + ioff, UV.y)) * gauss_kernel[i + 5];
					}
				}
				else if (unif_effect == 2) {
					color = vec4(0.0);
			
					//Vertical gaussian blur
					for (int i = -5; i < 6; i++) {
						float ioff = pixelSizeY * i;
						color += texture(unif_texture, vec2(UV.x, UV.y + ioff)) * gauss_kernel[i + 5];
					}
				}
				else if (unif_effect == 3) {
					//Inverted colors
					color = vec4(1.0, 1.0, 1.0, 2.0) - texture(unif_texture, UV);
				}
				else if (unif_effect == 4) {
					//Edge detection
					color += texture(unif_texture, vec2(UV.x - pixelSizeX, UV.y - pixelSizeY));
					color += texture(unif_texture, vec2(UV.x - pixelSizeX, UV.y));
					color += texture(unif_texture, vec2(UV.x - pixelSizeX, UV.y + pixelSizeY));
					color += 0;// texture(unif_texture, vec2(UV.x, UV.y - pixelSizeY));
					color += 0;// texture(unif_texture, vec2(UV.x, UV.y));
					color += 0;// texture(unif_texture, vec2(UV.x, UV.y + pixelSizeY));
					color += -texture(unif_texture, vec2(UV.x + pixelSizeX, UV.y - pixelSizeY));
					color += -texture(unif_texture, vec2(UV.x + pixelSizeX, UV.y));
					color += -texture(unif_texture, vec2(UV.x + pixelSizeX, UV.y + pixelSizeY));
				
					//Black and white
					color.x = color.y = color.z = (color.x + color.y + color.z) / 3;
					// = color.x;
					//color.z = color.x;
				}
				else if (unif_effect == 5) {
					// Trippy
					color = texture(unif_texture, UV) * (sin(50 * (UV.x) + unif_t) + 1.0) / 2.0 * (sin(50 * (UV.y) + unif_t) + 1.0) / 2.0;
					color.a = 1.0;
				}
				else {
					//Normal
					color = texture(unif_texture, UV);
				}
			}
		)shader";

	public:
		static ShaderInfo basicPostprocessShader() {
			ShaderStageInfo vertex = {};
			vertex.source_is_filepath = false;
			vertex.source = vertsource;
			vertex.stage = oe::vertex_shader;

			ShaderStageInfo fragment = {};
			fragment.source_is_filepath = false;
			fragment.source = fragsource;
			fragment.stage = oe::fragment_shader;

			ShaderInfo info = {};
			info.name = "Asset:BasicPostprocessShader";
			info.shader_stages = {
				vertex, fragment
			};

			return info;
		}

		// enum post_process {	normal = 6, invert_colors = 3, gauss_blur_horizontal = 1, gauss_blur_vertical = 2, edge_detection = 4, trippy = 5 };
		// void setPostprocessType(post_process type) {
		// 	setUniform1i("unif_effect", type);
		// }
		// 
		// void time(float _time) {
		// 	setUniform1i("unif_t", _time);
		// }
	};

}