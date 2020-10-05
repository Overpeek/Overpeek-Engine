#pragma once

#include "engine/internal_libs.hpp"



namespace oe::graphics {

	struct VertexData_internal {
		glm::fvec3 position;
		glm::fvec2 uv;
		glm::fvec4 color;

		constexpr VertexData_internal() noexcept
			: position(0.0f), uv(0.0f), color(0.0f)
		{}

		constexpr VertexData_internal(glm::fvec3 _position, glm::fvec2 _uv, glm::fvec4 _color) noexcept
			: position(_position), uv(_uv), color(_color)
		{}

		constexpr VertexData_internal(glm::fvec2 _position, glm::fvec2 _uv, glm::fvec4 _color) noexcept
			: position(_position, 0.0f), uv(_uv), color(_color)
		{}
	};

	struct VertexData : public VertexData_internal {
		static constexpr size_t component_count = sizeof(VertexData_internal) / sizeof(float);
		static constexpr size_t pos_offset = offsetof(VertexData_internal, position);
		static constexpr size_t uv_offset = offsetof(VertexData_internal, uv);
		static constexpr size_t col_offset = offsetof(VertexData_internal, color);

		constexpr VertexData() noexcept
			: VertexData_internal()
		{}

		constexpr VertexData(glm::fvec3 position, glm::fvec2 uv, glm::fvec4 color) noexcept
			: VertexData_internal(position, uv, color)
		{}

		constexpr VertexData(glm::fvec2 position, glm::fvec2 uv, glm::fvec4 color) noexcept
			: VertexData_internal(position, uv, color)
		{}

		static void config(std::function<void(int, int, size_t)> attrib_fn)
		{
			attrib_fn(0, 3, pos_offset);
			attrib_fn(1, 2, uv_offset);
			attrib_fn(2, 4, col_offset);
		}
	};

}

template <>
struct fmt::formatter<oe::graphics::VertexData> {
	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const oe::graphics::VertexData& d, FormatContext& ctx) {
		return format_to(ctx.out(), "[ {}, {}, {} ]", d.position, d.uv, d.color);
	}
};