#pragma once
#ifdef BUILD_VULKAN

#include "engine/graphics/interface/shader.hpp"
#include "vk_logical_device.hpp"
#include "vk_window.hpp"



namespace oe::graphics {

	class VKShader : public IShader {
	public:
		vk::PipelineLayout m_pipeline_layout;
		vk::Pipeline m_pipeline;
		
		const VKLogicalDevice* m_logical_device;
		const VKWindow* m_window;

	public:
		VKShader(const VKWindow* window, const ShaderInfo& shader_info);
		~VKShader();

		// Inherited via IShader
		void bind() const override;
		void unbind() const override;

		int getUniformLocation(const std::string& name) override;

		void setUniform1f(const std::string& name, float value) override;
		void setUniform2f(const std::string& name, const glm::fvec2& value) override;
		void setUniform3f(const std::string& name, const glm::fvec3& value) override;
		void setUniform4f(const std::string& name, const glm::fvec4& value) override;

		void setUniform1i(const std::string& name, int value) override;
		void setUniform2i(const std::string& name, const glm::ivec2& value) override;
		void setUniform3i(const std::string& name, const glm::ivec3& value) override;
		void setUniform4i(const std::string& name, const glm::ivec4& value) override;

		void setUniformMat4(const std::string& name, const glm::mat4& value) override;

	};

}
#endif